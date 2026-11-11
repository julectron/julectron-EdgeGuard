# Real-Time Operating System (RTOS) Architecture

In Julectron EdgeGuard, we use a Real-Time Operating System (RTOS) for deterministic time management and task scheduling. The overall system architecture is divided into several independent task modules.

## Task Architecture
The system pipeline is divided into 5 independent tasks. The number and definition of tasks depend on the system architecture and the designer's decision. However, from my perspective, fewer tasks are always better to reduce context-switching overhead and system complexity.

The 5 core tasks are:
1. **Receiving Task:** Reads data from the sensor.
2. **Preprocessing Task:** Prepares the data (filtering, windowing).
3. **Processing Task:** Performs DSP (FFT, feature extraction).
4. **Modeling Task:** Runs the TinyML model inference.
5. **Decision Task:** Compares the result with the threshold.

## Task Priorities
In an RTOS, each task must have a priority. The Receiving and Preprocessing tasks must have higher priorities than the others. Otherwise, incoming sensor data will be lost (data overrun). In this architecture, I have assigned the highest priority (4) to the Receiving Task and the lowest priority (1) to the Decision Task.

## System State Machine (Motor Health)
Instead of using simple if/else statements, we implemented a **State Machine** with 4 distinct states: **Normal**, **Warning**, **Critical**, and **Error**.

To define this structure, we currently use **Placeholder Thresholds** (dummy values) which will be replaced with real-world calculated values later.

### State Behavior:
- The system always remains in exactly one state at any given time.
- If no valid data is received (or the data is invalid/NaN), the system transitions to the **Error** state.
- When valid data is present, the system dynamically rotates between the other three states (**Normal**, **Warning**, **Critical**) based on the threshold values.

## Inter-Task Communication & Synchronization
To accelerate data transfer and ensure proper synchronization between tasks, we defined and handled **Queues** inside the tasks. By passing the **address (pointer)** of the data arrays instead of copying the entire data, the system achieves high-speed data transfer and seamless synchronization.

**Modular Interfaces:**
We designed the tasks to be modular, communicating through specific **Interfaces**. We ensured that each task is modularized based on the nature of its data output. This is because the output data structure of the Receiving task is entirely different from the output data structure of the Processing task.

**Additional Note (For more information):**
**Queue** in the RTOS architecture is a subset of **Synchronization**, responsible for synchronizing and communicating between tasks. Other methods include Semaphore, Mutex, Event Groups, Task Notifications, Stream Buffers, and Message Buffers. In this architecture, we chose to use Queue. Additionally, task definition and coding are subsets of **Task Management**.

Additionally, a **Binary Semaphore** is used to synchronize the hardware DMA interrupt with the Receiving task, ensuring the CPU wakes up only when the buffer is full.

## Code Implementation

*Note: The code below is the initial architecture with implemented Queue, Semaphore, State Machine logic, and Modular Interfaces. The internal structures and DSP logic will be implemented step-by-step.*

```c
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stm32f4xx_hal.h"
#include <math.h>

#define PRIORITY_RECEIVING    4
#define PRIORITY_PREPROCESS   3
#define PRIORITY_PROCESSING   3
#define PRIORITY_MODELING     2
#define PRIORITY_DECISION     1

#define THRESHOLD_WARNING     30.0f
#define THRESHOLD_CRITICAL    60.0f

typedef enum
{
    Normal,
    Warning,
    Critical,
    Error
} SystemState;

SystemState currentState = Normal;

QueueHandle_t receivingQueue;
QueueHandle_t preprocessingQueue;
QueueHandle_t processingQueue;
QueueHandle_t modelingQueue;

SemaphoreHandle_t dmaSemaphore;


BaseType_t ReceivingModule_Read(SPI_HandleTypeDef *spi, ReceivingData *output);

BaseType_t PreprocessingModule_Process(ReceivingData *input, PreprocessingData *output);

BaseType_t ProcessingModule_Process(PreprocessingData *input, ProcessingData *output);

BaseType_t ModelingModule_Process(ProcessingData *input, ModelData *output);

BaseType_t DecisionModule_IsValid(float ModelOutput);


void receivingTask(void *pvParameters)
{
    SPI_HandleTypeDef *spi = (SPI_HandleTypeDef *)pvParameters;
    ReceivingData data;

    while(1)
    {
        if(xSemaphoreTake(dmaSemaphore, portMAX_DELAY) == pdTRUE)
        {
            if(ReceivingModule_Read(spi, &data) == pdTRUE)
            {
                xQueueSend(receivingQueue, &data, portMAX_DELAY);
            }
        }
    }
}

void preprocessingTask(void *pvParameters)
{
    ReceivingData data;

    while(1)
    {
        if(xQueueReceive(receivingQueue, &data, portMAX_DELAY) == pdPASS)
        {
            PreprocessingData output;

            if(PreprocessingModule_Process(&data, &output) == pdTRUE)
            {
                xQueueSend(preprocessingQueue, &output, portMAX_DELAY);
            }
        }
    }
}

void processingTask(void *pvParameters)
{
    PreprocessingData data;

    while(1)
    {
        if(xQueueReceive(preprocessingQueue, &data, portMAX_DELAY) == pdPASS)
        {
            ProcessingData output;

            if(ProcessingModule_Process(&data, &output) == pdTRUE)
            {
                xQueueSend(processingQueue, &output, portMAX_DELAY);
            }
        }
    }
}

void modelingTask(void *pvParameters)
{
    ProcessingData data;

    while(1)
    {
        if(xQueueReceive(processingQueue, &data, portMAX_DELAY) == pdPASS)
        {
            ModelData output;

            if(ModelingModule_Process(&data, &output) == pdTRUE)
            {
                xQueueSend(modelingQueue, &output, portMAX_DELAY);
            }
        }
    }
}

void decisionTask(void *pvParameters)
{
    ModelData data;
    float ModelOutput;

    while(1)
    {
        if(xQueueReceive(modelingQueue, &data, pdMS_TO_TICKS(1)) == pdPASS)
        {
            ModelOutput = data.modelOutput;

            if(DecisionModule_IsValid(ModelOutput) != pdTRUE)
            {
                currentState = Error;
            }
            else
            {
                switch(currentState)
                {
                    case Normal:
                        if(ModelOutput >= THRESHOLD_WARNING)
                            currentState = Warning;
                        break;

                    case Warning:
                        if(ModelOutput >= THRESHOLD_CRITICAL)
                            currentState = Critical;
                        else if(ModelOutput < THRESHOLD_WARNING)
                            currentState = Normal;
                        break;

                    case Critical:
                        if(ModelOutput < THRESHOLD_CRITICAL)
                            currentState = Warning;
                        break;

                    case Error:
                        break;

                    default:
                        currentState = Error;
                        break;
                }
            }
        }
        else
        {
            currentState = Error;
        }
    }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(
        dmaSemaphore,
        &xHigherPriorityTaskWoken
    );

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void Create_Tasks(void)
{
    dmaSemaphore = xSemaphoreCreateBinary();

    if(dmaSemaphore == NULL)
    {
        Error_Handler();
    }

    receivingQueue = xQueueCreate(5, sizeof(ReceivingData));
    preprocessingQueue = xQueueCreate(5, sizeof(PreprocessingData));
    processingQueue = xQueueCreate(5, sizeof(ProcessingData));
    modelingQueue = xQueueCreate(5, sizeof(ModelData));

    if(receivingQueue == NULL ||
       preprocessingQueue == NULL ||
       processingQueue == NULL ||
       modelingQueue == NULL)
    {
        Error_Handler();
    }

    xTaskCreate(receivingTask, "Receiving", 1024, &hspi, PRIORITY_RECEIVING, NULL);
    xTaskCreate(preprocessingTask, "Preprocessing", 1024, NULL, PRIORITY_PREPROCESS, NULL);
    xTaskCreate(processingTask, "Processing", 1024, NULL, PRIORITY_PROCESSING, NULL);
    xTaskCreate(modelingTask, "Modeling", 1024, NULL, PRIORITY_MODELING, NULL);
    xTaskCreate(decisionTask, "Decision", 1024, NULL, PRIORITY_DECISION, NULL);
}
