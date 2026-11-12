# RTOS Architecture (In Development)

*Note on Architecture Refactoring:*
In the previous architecture, there were structural issues and a lack of organization. I decided to redesign the architecture in a more structured and standard way. This caused a slight delay in the daily routine of the EdgeGuard project, but resulted in a much more robust and defensible codebase.

Since I need to add new code daily, I currently have to replace the entire file rather than editing it, because piecemeal edits disrupt the overall order. Therefore, the **final detailed documentation will be written upon the completion of the RTOS architecture phase.**

## Current Code Implementation

```c
#define RECEIVE_PRIORITY          5
#define PREPROCESSING_PRIORITY    4
#define PROCESSING_PRIORITY       3
#define OUTPUT_MODEL_PRIORITY     2
#define DECISION_PRIORITY         1

#define RECEIVE_STACK_SIZE          256
#define PREPROCESSING_STACK_SIZE   1024
#define PROCESSING_STACK_SIZE      2048
#define OUTPUT_MODEL_STACK_SIZE    1024
#define DECISION_STACK_SIZE         256

#define QUEUE_LENGTH               10
#define QUEUE_ITEM_SIZE            sizeof(VibrationData *)

#define VIBRATION_BUFFER_SIZE      800


typedef struct
{
    float X;
    float Y;
    float Z;

} VibrationData;


VibrationData Vibration_Buffer[VIBRATION_BUFFER_SIZE];

VibrationData *Vibration_Data_Ptr = Vibration_Buffer;


QueueHandle_t Receive_Queue;
QueueHandle_t Preprocessing_Queue;
QueueHandle_t Processing_Queue;
QueueHandle_t Output_Model_Queue;

SemaphoreHandle_t DMA_Semaphore;


void Receive_Task(void *pvParameters)
{
    VibrationData *Data_Ptr = Vibration_Buffer;

    while (1)
    {
        xSemaphoreTake(DMA_Semaphore, portMAX_DELAY);

        xQueueSend(Receive_Queue, &Data_Ptr, portMAX_DELAY);
    }
}


void Preprocessing_Task(void *pvParameters)
{
    VibrationData *Data_Ptr;

    while (1)
    {
        xQueueReceive(Receive_Queue, &Data_Ptr, portMAX_DELAY);

        xQueueSend(Preprocessing_Queue, &Data_Ptr, portMAX_DELAY);
    }
}


void Processing_Task(void *pvParameters)
{
    VibrationData *Data_Ptr;

    while (1)
    {
        xQueueReceive(Preprocessing_Queue, &Data_Ptr, portMAX_DELAY);

        xQueueSend(Processing_Queue, &Data_Ptr, portMAX_DELAY);
    }
}


void Output_Model_Task(void *pvParameters)
{
    VibrationData *Data_Ptr;

    while (1)
    {
        xQueueReceive(Processing_Queue, &Data_Ptr, portMAX_DELAY);

        xQueueSend(Output_Model_Queue, &Data_Ptr, portMAX_DELAY);
    }
}


void Decision_Task(void *pvParameters)
{
    VibrationData *Data_Ptr;

    while (1)
    {
        xQueueReceive(Output_Model_Queue, &Data_Ptr, portMAX_DELAY);
    }
}


void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(DMA_Semaphore, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void Queue_Management(void)
{
    Receive_Queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);

    Preprocessing_Queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);

    Processing_Queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);

    Output_Model_Queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);

    DMA_Semaphore = xSemaphoreCreateBinary();
}


void Task_Management(void)
{
    xTaskCreate(Receive_Task, "Receive", RECEIVE_STACK_SIZE, NULL, RECEIVE_PRIORITY, NULL);

    xTaskCreate(Preprocessing_Task, "Preprocessing", PREPROCESSING_STACK_SIZE, NULL, PREPROCESSING_PRIORITY, NULL);

    xTaskCreate(Processing_Task, "Processing", PROCESSING_STACK_SIZE, NULL, PROCESSING_PRIORITY, NULL);

    xTaskCreate(Output_Model_Task, "Output_Model", OUTPUT_MODEL_STACK_SIZE, NULL, OUTPUT_MODEL_PRIORITY, NULL);

    xTaskCreate(Decision_Task, "Decision", DECISION_STACK_SIZE, NULL, DECISION_PRIORITY, NULL);
}
