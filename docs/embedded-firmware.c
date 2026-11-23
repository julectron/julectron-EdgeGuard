#include "stm32f4xx_hal.h"
#include <math.h>
#include "arm_math.h"

TIM_HandleTypeDef htim2;
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi1_rx;

volatile uint8_t spi_rx_buffer[128 * 6];

int16_t accel_x[128];
int16_t accel_y[128];
int16_t accel_z[128];

float accel_x_f[128];
float accel_y_f[128];
float accel_z_f[128];
float fft_output_x[128];
float fft_output_y[128];
float fft_output_z[128];

float magnitude_x[37];
float magnitude_y[37];
float magnitude_z[37];

arm_rfft_fast_instance_f32 fft_instance_x;
arm_rfft_fast_instance_f32 fft_instance_y;
arm_rfft_fast_instance_f32 fft_instance_z;
float feat_rms[3];
float feat_peak[3];
float feat_crest[3];
float feat_kurtosis[3];
float feat_skewness[3];
float feat_variance[3];
float feat_mean[3];

float feat_dom_freq[3];
float feat_spec_cent[3];
float feat_band_energy[3];
float feat_spec_kurt[3];

void SystemClock_Config(void);
void Timer_config(void);
void GPIO_config(void);
void DMA_config(void);
void SPI_config(void);
void ADXL345_Init(void);
void ADXL345_Read_Data(void);
void ADXL345_Preprocess(void);
void ADXL345_FFT(float *input, uint16_t fft_length, float magnitude[37], float *output, arm_rfft_fast_instance_f32 *instance);
void ADXL345_Processing(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    Timer_config();
    GPIO_config();
    DMA_config();
    SPI_config();
    ADXL345_Init();
    ADXL345_Read_Data();
    HAL_TIM_Base_Start(&htim2);
}

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

    htim2.Instance = TIM2;

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

void DMA_config(void)
{
    __HAL_RCC_DMA2_CLK_ENABLE();

    hdma_spi1_rx.Instance                 = DMA2_Stream2;
    hdma_spi1_rx.Init.Channel             = DMA_CHANNEL_3;
    hdma_spi1_rx.Init.MemAddress          = (uint32_t)spi_rx_buffer;
    hdma_spi1_rx.Init.PeriphAddress       = (uint32_t)&hspi1.Instance->DR;
    hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_spi1_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_spi1_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_spi1_rx.Init.Mode                = DMA_NORMAL;
    hdma_spi1_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_spi1_rx.Init.BufferSize          = 128 * 6;

    HAL_DMA_Init(&hdma_spi1_rx);
    __HAL_LINKDMA(&hspi1, hdmarx, hdma_spi1_rx);

    __HAL_DMA_ENABLE_IT(&hdma_spi1_rx, DMA_IT_TC);
    __HAL_DMA_ENABLE_IT(&hdma_spi1_rx, DMA_IT_TE);

    HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

    __HAL_DMA_ENABLE(&hdma_spi1_rx);
}

void SPI_config(void)
{
    __HAL_RCC_SPI1_CLK_ENABLE();

    hspi1.Instance = SPI1;

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

void ADXL345_Init(void)
{
    uint8_t odr_data[2] = {0x2C, 0x0E};
    uint8_t format_data[2] = {0x31, 0x0B};
    uint8_t power_data[2] = {0x2D, 0x08};

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, odr_data, 2, 100);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, format_data, 2, 100);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, power_data, 2, 100);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

void ADXL345_Read_Data(void)
{
    uint8_t cmd = 0xF2;

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)spi_rx_buffer, 128 * 6);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        
        ADXL345_Preprocess();
        ADXL345_FFT(accel_x_f, 128, magnitude_x, fft_output_x, &fft_instance_x);
        ADXL345_FFT(accel_y_f, 128, magnitude_y, fft_output_y, &fft_instance_y);
        ADXL345_FFT(accel_z_f, 128, magnitude_z, fft_output_z, &fft_instance_z);
        ADXL345_Processing();

        uint8_t cmd = 0xF2;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
        HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)spi_rx_buffer, 128 * 6);
    }
}

void ADXL345_Preprocess(void)
{
    int32_t sum_x = 0;
    int32_t sum_y = 0;
    int32_t sum_z = 0;
    int16_t offset_x = 0;
    int16_t offset_y = 0;
    int16_t offset_z = 0;

    for (int i = 0; i < 128; i++)
    {
        sum_x += (int16_t)((spi_rx_buffer[i * 6 + 1] << 8) | spi_rx_buffer[i * 6 + 0]);
        sum_y += (int16_t)((spi_rx_buffer[i * 6 + 3] << 8) | spi_rx_buffer[i * 6 + 2]);
        sum_z += (int16_t)((spi_rx_buffer[i * 6 + 5] << 8) | spi_rx_buffer[i * 6 + 4]);
    }

    offset_x = sum_x / 128;
    offset_y = sum_y / 128;
    offset_z = sum_z / 128;

    for (int i = 0; i < 128; i++)
    {
        int16_t raw_x = (int16_t)((spi_rx_buffer[i * 6 + 1] << 8) | spi_rx_buffer[i * 6 + 0]);
        int16_t raw_y = (int16_t)((spi_rx_buffer[i * 6 + 3] << 8) | spi_rx_buffer[i * 6 + 2]);
        int16_t raw_z = (int16_t)((spi_rx_buffer[i * 6 + 5] << 8) | spi_rx_buffer[i * 6 + 4]);

        accel_x[i] = raw_x - offset_x;
        accel_y[i] = raw_y - offset_y;
        accel_z[i] = raw_z - offset_z;
    }

    float alpha = 0.7385f;

    for (int i = 1; i < 128; i++)
    {
        accel_x[i] = (int16_t)(accel_x[i - 1] + alpha * (accel_x[i] - accel_x[i - 1]));
        accel_y[i] = (int16_t)(accel_y[i - 1] + alpha * (accel_y[i] - accel_y[i - 1]));
        accel_z[i] = (int16_t)(accel_z[i - 1] + alpha * (accel_z[i] - accel_z[i - 1]));
    }
    for (int i = 0; i < 128; i++)
    {
        float window = 0.5f - 0.5f * cosf(2.0f * 3.14159265f * i / 128.0f);

        accel_x_f[i] = (float)accel_x[i] * window;
        accel_y_f[i] = (float)accel_y[i] * window;
        accel_z_f[i] = (float)accel_z[i] * window;
    }

    float mean_x = 0, mean_y = 0, mean_z = 0;
    float var_x = 0, var_y = 0, var_z = 0;

    for (int i = 0; i < 128; i++)
    {
        mean_x += accel_x_f[i];
        mean_y += accel_y_f[i];
        mean_z += accel_z_f[i];
    }
    mean_x /= 128.0f;
    mean_y /= 128.0f;
    mean_z /= 128.0f;

    for (int i = 0; i < 128; i++)
    {
        var_x += (accel_x_f[i] - mean_x) * (accel_x_f[i] - mean_x);
        var_y += (accel_y_f[i] - mean_y) * (accel_y_f[i] - mean_y);
        var_z += (accel_z_f[i] - mean_z) * (accel_z_f[i] - mean_z);
    }
    var_x /= 128.0f;
    var_y /= 128.0f;
    var_z /= 128.0f;

    float std_x = sqrtf(var_x);
    float std_y = sqrtf(var_y);
    float std_z = sqrtf(var_z);

    if (std_x > 0.0f && std_y > 0.0f && std_z > 0.0f)
    {
        for (int i = 0; i < 128; i++)
        {
            accel_x_f[i] = (accel_x_f[i] - mean_x) / std_x;
            accel_y_f[i] = (accel_y_f[i] - mean_y) / std_y;
            accel_z_f[i] = (accel_z_f[i] - mean_z) / std_z;
        }
    }
}
void ADXL345_FFT(float *input, uint16_t fft_length, float magnitude[37], float *output, arm_rfft_fast_instance_f32 *instance)
{
    arm_status status;

    status = arm_rfft_fast_init_f32(instance, fft_length);

    if (status != ARM_MATH_SUCCESS)
    {
        return;
    }

    arm_rfft_fast_f32(instance, input, output, 0);

    magnitude[0] = fabsf(output[0]);

    arm_cmplx_mag_f32(&output[2], &magnitude[1], 36);
}
void ADXL345_Processing(void)
{
    float mean_x = 0.0f, mean_y = 0.0f, mean_z = 0.0f;
    float var_x = 0.0f, var_y = 0.0f, var_z = 0.0f;
    float rms_x = 0.0f, rms_y = 0.0f, rms_z = 0.0f;
    float peak_x = 0.0f, peak_y = 0.0f, peak_z = 0.0f;
    float crest_x = 0.0f, crest_y = 0.0f, crest_z = 0.0f;
    float kurt_x = 0.0f, kurt_y = 0.0f, kurt_z = 0.0f;
    float skew_x = 0.0f, skew_y = 0.0f, skew_z = 0.0f;
    
    float dom_freq_x = 0.0f, dom_freq_y = 0.0f, dom_freq_z = 0.0f;
    float spec_cent_x = 0.0f, spec_cent_y = 0.0f, spec_cent_z = 0.0f;
    float band_energy_x = 0.0f, band_energy_y = 0.0f, band_energy_z = 0.0f;
    float spec_kurt_x = 0.0f, spec_kurt_y = 0.0f, spec_kurt_z = 0.0f;
    
    float max_val_x = 0.0f, max_val_y = 0.0f, max_val_z = 0.0f;
    float sum_sq_x = 0.0f, sum_sq_y = 0.0f, sum_sq_z = 0.0f;
    float sum_cube_x = 0.0f, sum_cube_y = 0.0f, sum_cube_z = 0.0f;
    float sum_quad_x = 0.0f, sum_quad_y = 0.0f, sum_quad_z = 0.0f;
    float sum_mag_x = 0.0f, sum_mag_y = 0.0f, sum_mag_z = 0.0f;
    float sum_weighted_x = 0.0f, sum_weighted_y = 0.0f, sum_weighted_z = 0.0f;
    float sum_spec_kurt_x = 0.0f, sum_spec_kurt_y = 0.0f, sum_spec_kurt_z = 0.0f;
    float mean_mag_x = 0.0f, mean_mag_y = 0.0f, mean_mag_z = 0.0f;

    for (int i = 0; i < 128; i++)
    {
        mean_x += accel_x_f[i];
        mean_y += accel_y_f[i];
        mean_z += accel_z_f[i];
    }
    mean_x /= 128.0f;
    mean_y /= 128.0f;
    mean_z /= 128.0f;

    for (int i = 0; i < 128; i++)
    {
        float dx = accel_x_f[i] - mean_x;
        float dy = accel_y_f[i] - mean_y;
        float dz = accel_z_f[i] - mean_z;

        float abs_x = (dx < 0.0f) ? -dx : dx;
        float abs_y = (dy < 0.0f) ? -dy : dy;
        float abs_z = (dz < 0.0f) ? -dz : dz;

        if (abs_x > max_val_x) max_val_x = abs_x;
        if (abs_y > max_val_y) max_val_y = abs_y;
        if (abs_z > max_val_z) max_val_z = abs_z;

        sum_sq_x += dx * dx;
        sum_sq_y += dy * dy;
        sum_sq_z += dz * dz;

        sum_cube_x += dx * dx * dx;
        sum_cube_y += dy * dy * dy;
        sum_cube_z += dz * dz * dz;

        sum_quad_x += dx * dx * dx * dx;
        sum_quad_y += dy * dy * dy * dy;
        sum_quad_z += dz * dz * dz * dz;
    }

    var_x = sum_sq_x / 128.0f;
    var_y = sum_sq_y / 128.0f;
    var_z = sum_sq_z / 128.0f;

    rms_x = sqrtf(sum_sq_x / 128.0f);
    rms_y = sqrtf(sum_sq_y / 128.0f);
    rms_z = sqrtf(sum_sq_z / 128.0f);

    peak_x = max_val_x;
    peak_y = max_val_y;
    peak_z = max_val_z;

    crest_x = (rms_x > 0.0f) ? (peak_x / rms_x) : 0.0f;
    crest_y = (rms_y > 0.0f) ? (peak_y / rms_y) : 0.0f;
    crest_z = (rms_z > 0.0f) ? (peak_z / rms_z) : 0.0f;

    kurt_x = (var_x > 0.0f) ? ((sum_quad_x / 128.0f) / (var_x * var_x)) : 0.0f;
    kurt_y = (var_y > 0.0f) ? ((sum_quad_y / 128.0f) / (var_y * var_y)) : 0.0f;
    kurt_z = (var_z > 0.0f) ? ((sum_quad_z / 128.0f) / (var_z * var_z)) : 0.0f;

    skew_x = (var_x > 0.0f) ? ((sum_cube_x / 128.0f) / (var_x * rms_x)) : 0.0f;
    skew_y = (var_y > 0.0f) ? ((sum_cube_y / 128.0f) / (var_y * rms_y)) : 0.0f;
    skew_z = (var_z > 0.0f) ? ((sum_cube_z / 128.0f) / (var_z * rms_z)) : 0.0f;

    for (int i = 0; i < 36; i++)
    {
        float m_x = magnitude_x[i + 1];
        float m_y = magnitude_y[i + 1];
        float m_z = magnitude_z[i + 1];
        
        sum_mag_x += m_x;
        sum_mag_y += m_y;
        sum_mag_z += m_z;
        
        sum_weighted_x += m_x * (float)i;
        sum_weighted_y += m_y * (float)i;
        sum_weighted_z += m_z * (float)i;

        band_energy_x += m_x * m_x;
        band_energy_y += m_y * m_y;
        band_energy_z += m_z * m_z;
    }

    mean_mag_x = sum_mag_x / 36.0f;
    mean_mag_y = sum_mag_y / 36.0f;
    mean_mag_z = sum_mag_z / 36.0f;
    for (int i = 0; i < 36; i++)
    {
        float m_x = magnitude_x[i + 1];
        float m_y = magnitude_y[i + 1];
        float m_z = magnitude_z[i + 1];
        
        sum_spec_kurt_x += (m_x - mean_mag_x) * (m_x - mean_mag_x) * (m_x - mean_mag_x) * (m_x - mean_mag_x);
        sum_spec_kurt_y += (m_y - mean_mag_y) * (m_y - mean_mag_y) * (m_y - mean_mag_y) * (m_y - mean_mag_y);
        sum_spec_kurt_z += (m_z - mean_mag_z) * (m_z - mean_mag_z) * (m_z - mean_mag_z) * (m_z - mean_mag_z);
    }

    spec_cent_x = (sum_mag_x > 0.0f) ? (sum_weighted_x / sum_mag_x) : 0.0f;
    spec_cent_y = (sum_mag_y > 0.0f) ? (sum_weighted_y / sum_mag_y) : 0.0f;
    spec_cent_z = (sum_mag_z > 0.0f) ? (sum_weighted_z / sum_mag_z) : 0.0f;

    spec_kurt_x = (mean_mag_x > 0.0f) ? ((sum_spec_kurt_x / 36.0f) / (mean_mag_x * mean_mag_x)) : 0.0f;
    spec_kurt_y = (mean_mag_y > 0.0f) ? ((sum_spec_kurt_y / 36.0f) / (mean_mag_y * mean_mag_y)) : 0.0f;
    spec_kurt_z = (mean_mag_z > 0.0f) ? ((sum_spec_kurt_z / 36.0f) / (mean_mag_z * mean_mag_z)) : 0.0f;
}