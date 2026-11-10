#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
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

SemaphoreHandle_t dmaSemaphore;


void receivingTask(void *pvParameters)
{
    SPI_HandleTypeDef *spi = (SPI_HandleTypeDef *)pvParameters;
    ReceivingData data;

    while(1)
    {
        if(xSemaphoreTake(dmaSemaphore, portMAX_DELAY) == pdTRUE)
        {
            if(xQueueSend(receivingQueue, &data, portMAX_DELAY) != pdPASS)
            {
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
