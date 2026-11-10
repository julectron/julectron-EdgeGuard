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

## Inter-Task Communication & Synchronization
To accelerate data transfer and ensure proper synchronization between tasks, we defined and handled **Queues** inside the tasks. By passing the **address (pointer)** of the data arrays instead of copying the entire data, the system achieves high-speed data transfer and seamless synchronization.

**Additional Note (For more information):**
**Queue** in the RTOS architecture is a subset of **Synchronization**, responsible for synchronizing and communicating between tasks. Other methods include Semaphore, Mutex, Event Groups, Task Notifications, Stream Buffers, and Message Buffers. In this architecture, we chose to use Queue. Additionally, task definition and coding are subsets of **Task Management**.

## Code Implementation

*Note: The code below is the initial skeleton with implemented Queue communication. The internal DSP and AI logic will be implemented step-by-step.*

```c
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f4xx_hal.h"

#define PRIORITY_RECEIVING    4
#define PRIORITY_PREPROCESS   3
#define PRIORITY_PROCESSING   3
#define PRIORITY_MODELING     2
#define PRIORITY_DECISION     1

QueueHandle_t receivingQueue;
QueueHandle_t preprocessingQueue;
QueueHandle_t processingQueue;
QueueHandle_t modelingQueue;

void receivingTask(void *pvParameters)
{
    SPI_HandleTypeDef *spi = (SPI_HandleTypeDef *)pvParameters;
    ReceivingData data;

    while(1)
    {
        if(xQueueSend(receivingQueue, &data, portMAX_DELAY) != pdPASS)
        {
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

            xQueueSend(preprocessingQueue, &output, portMAX_DELAY);
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

            xQueueSend(processingQueue, &output, portMAX_DELAY);
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

            xQueueSend(modelingQueue, &output, portMAX_DELAY);
        }
    }
}

void decisionTask(void *pvParameters)
{
    ModelData data;

    while(1)
    {
        if(xQueueReceive(modelingQueue, &data, portMAX_DELAY) == pdPASS)
        {
        }
    }
}

void Create_Tasks(void)
{
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

    xTaskCreate(receivingTask, "Receiving", 1024,
                &hspi, PRIORITY_RECEIVING, NULL);

    xTaskCreate(preprocessingTask, "Preprocessing", 1024,
                NULL, PRIORITY_PREPROCESS, NULL);

    xTaskCreate(processingTask, "Processing", 1024,
                NULL, PRIORITY_PROCESSING, NULL);

    xTaskCreate(modelingTask, "Modeling", 1024,
                NULL, PRIORITY_MODELING, NULL);

    xTaskCreate(decisionTask, "Decision", 1024,
                NULL, PRIORITY_DECISION, NULL);
}
