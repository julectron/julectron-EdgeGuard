# STM32F411 Motor Condition Monitoring System - Code Specifications

## 1. System Clock
- SYSCLK: 100 MHz
- Source: HSE (8 MHz) + PLL
- AHB / APB1 / APB2: 100 MHz / 50 MHz / 100 MHz

## 2. Timer (TIM2)
- Purpose: Sampling trigger (1 kHz)
- Prescaler (PSC): 99
- Auto-Reload (ARR): 999
- Mode: Up-counting

## 3. SPI1
- Mode: Master
- Data Size: 8-bit
- CPOL / CPHA: 1 / 1 (Mode 3)
- Baud Rate Prescaler: 32 (3.125 MHz)
- NSS: Software (Manual CS on PA4)
- First Bit: MSB

## 4. DMA2 (SPI1_RX)
- Stream / Channel: Stream 2 / Channel 3
- Mode: Normal
- Data Alignment: Byte
- Memory Increment: Enabled
- Peripheral Increment: Disabled
- Priority: High
- Interrupts: TC & TE (NVIC Enabled)

## 5. Buffer & Data Size
- Total Samples per Buffer: 128
- Bytes per Sample: 6 (X/Y/Z 16-bit)
- Total Buffer Size: 768 Bytes
- Buffer Type: volatile uint8_t

## 6. Sensor (ADXL345)
- ODR: 1600 Hz
- Range: ±16g
- Resolution: 13-bit (Full Resolution)
- Mode: Measurement

## 7. Pre-processing
- Offset Removal: Dynamic (mean subtraction)
- Low-Pass Filter: IIR 1st Order (Alpha = 0.7385, ~450 Hz cutoff)
- Window: Hanning (128 samples)
- Normalization: Z-Score (Dynamic Mean & Std Dev)

## 8. Processing (FFT)
- Algorithm: CMSIS-DSP arm_rfft_fast_f32
- FFT Length: 128
- Magnitude Bins: 37 (DC + 36 bins)
- Output Arrays: magnitude_x/y/z[37]

## 9. Feature Extraction
- Time-Domain: RMS, Peak, Crest Factor, Kurtosis, Skewness, Variance, Mean
- Frequency-Domain: Dominant Frequency, Spectral Centroid, Band Energy, Spectral Kurtosis
- Feature Vector Size: 33 (11 features × 3 axes)