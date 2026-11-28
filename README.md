# Julectron EdgeGuard 
<img width="400" alt="mypic" src="https://github.com/user-attachments/assets/ba7c9747-32a3-4847-a806-ab6905b1f50a" />

## Overview
A Real-Time Intelligent Motor Condition Monitoring System with Embedded Anomaly Detection. 
Designed for predictive maintenance of electric motors using STM32, FreeRTOS, and Edge AI.

## Project Status
- **Phase 0 (completed):** Learning motor vibration signatures & setting up data analysis pipeline.
- **Upcoming:** Deploying DSP pipeline to hardware.

## System Architecture (in progress)
1. **Sensing:** ADXL345 (Digital 3-axis Accelerometer via SPI)
2. **Data Transfer:** SPI + DMA
3. **Processing:** CMSIS-DSP (FFT, Feature Extraction)
4. **Decision Engine:** Rule-Based (V1.0) -> TinyML (V2.0)

## Roadmap
- **Phase 0:** Motor Basics & Data Analysis (completed)
- **V1.0 (Days 1-40):** Embedded System (DMA, DSP, Rule-based detection)
- **V2.0 (Days 41-56):** Edge AI (TFLite Micro, INT8 Quantization)

## Author
**Morteza Ranjbar** (Founder of Julectron)
[GitHub Profile](https://github.com/julectron)
## Documentation
- [System Architecture & Block Diagram](docs/block-diagram.md)
- [Data Flow Diagram](docs/data-flow-diagram.md)
-  [RTOS Architecture & Synchronization](docs/rtos-architecture.md)
- [Embedded Firmware Code](docs/embedded-firmware.c)
- [Engineering Log](docs/engineering-log.md)
- [Firmware Specifications](docs/firmware-specifications.md)
- [Motor and Bearing Basics](docs/motor-and-bearing-basics.md)
- [My Journey (Reflections)](docs/my-journey.md)
- [Python Phase Report (Day 1)](docs/python-day1-report.md)
