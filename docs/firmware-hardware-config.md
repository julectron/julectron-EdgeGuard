# Firmware Hardware Configuration (In Development)

## Today's Achievement
Today, I focused on configuring the embedded system peripherals based on the specific requirements of the **Julectron EdgeGuard** project. I successfully configured the following peripherals:

1. **System Clock:** Configured to 100 MHz using HSE and PLL.
2. **Timer (TIM2):** Configured to generate a precise 1 kHz (1 ms) trigger for the DMA/SPI acquisition.
3. **GPIO:** Configured PA5, PA6, PA7 (SPI1), PA4 (CS), PA8 (Input), and PA9 (Output).
4. **SPI1:** Configured in Master mode, Mode 3 (CPOL=High, CPHA=2Edge), 8-bit data size, with DMA enabled for TX and RX.

*Note: The System Clock configuration was implemented with the help of standard architecture references. The rest of the peripheral configurations are fully tailored to the EdgeGuard project requirements.*

**Current Status:** This code is currently under active development and is incomplete. The DMA configuration and the final main logic will be added in the next steps.

## Hardware Configuration Code

```c
#include "stm32f4xx_hal.h"

TIM_HandleTypeDef htim2;
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi1_rx;

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 8;
    RCC_OscInitStruct.PLL.PLLN       = 200;
    RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ       = 4;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);

    HAL_PWREx_EnableOverDrive();
}

void Timer_config(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};

    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Init.Prescaler   = 99;
    htim2.Init.Period      = 999;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;

    HAL_TIM_Base_Init(&htim2);
    HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

    __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE);
}

void GPIO_config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin        = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode       = GPIO_MODE_AF;
    GPIO_InitStruct.Alternate  = GPIO_AF5_SPI1;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.OutputType = GPIO_OUTPUT_PUSH_PULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct = (GPIO_InitTypeDef){0};

    GPIO_InitStruct.Pin        = GPIO_PIN_4;
    GPIO_InitStruct.Mode       = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull       = GPIO_NOPULL;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct = (GPIO_InitTypeDef){0};

    GPIO_InitStruct.Pin        = GPIO_PIN_8;
    GPIO_InitStruct.Mode       = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct = (GPIO_InitTypeDef){0};

    GPIO_InitStruct.Pin        = GPIO_PIN_9;
    GPIO_InitStruct.Mode       = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void SPI_config(void)
{
    __HAL_RCC_SPI1_CLK_ENABLE();

    hspi1.Init.Mode              = SPI_MODE_MASTER;
    hspi1.Init.CLKPolarity       = SPI_POLARITY_HIGH;
    hspi1.Init.CLKPhase          = SPI_PHASE_2EDGE;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    hspi1.Init.NSS               = SPI_NSS_SOFT;

    HAL_SPI_Init(&hspi1);

    __HAL_SPI_ENABLE_DMA(&hspi1, SPI_DMA_TX);
    __HAL_SPI_ENABLE_DMA(&hspi1, SPI_DMA_RX);
}