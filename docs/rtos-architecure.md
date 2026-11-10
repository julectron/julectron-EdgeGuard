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
