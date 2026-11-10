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

## Code Implementation

```c
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_hal.h"

#define PRIORITY_RECEIVING    4
#define PRIORITY_PREPROCESS   3
#define PRIORITY_PROCESSING   3
#define PRIORITY_MODELING     2
#define PRIORITY_DECISION     1

void receivingTask(void *pvParameters) {
    SPI_HandleTypeDef *spi = (SPI_HandleTypeDef *)pvParameters;
    while(1) { }
}

void preprocessingTask(void *pvParameters) {
    ReceivingData *data = (ReceivingData *)pvParameters;
    while(1) { }
}

void processingTask(void *pvParameters) {
    PreprocessingData *data = (PreprocessingData *)pvParameters;
    while(1) { }
}

void modelingTask(void *pvParameters) {
    ProcessingData *data = (ProcessingData *)pvParameters;
    while(1) { }
}

void decisionTask(void *pvParameters) {
    ModelData *data = (ModelData *)pvParameters;
    while(1) { }
}

void Create_Tasks(void) {
    xTaskCreate(receivingTask, "Receiving", 1024, &hspi, PRIORITY_RECEIVING, NULL);
    xTaskCreate(preprocessingTask, "Preprocessing", 1024, &receivingData, PRIORITY_PREPROCESS, NULL);
    xTaskCreate(processingTask, "Processing", 1024, &preprocessingData, PRIORITY_PROCESSING, NULL);
    xTaskCreate(modelingTask, "Modeling", 1024, &processingData, PRIORITY_MODELING, NULL);
    xTaskCreate(decisionTask, "Decision", 1024, &modelData, PRIORITY_DECISION, NULL);
}


---
* Day 3: I do not claim to know everything, but I am trying my best to become the best.*
