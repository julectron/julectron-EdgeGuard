# Engineering Log

## Today's Focus
- **Architecture Review & Refactoring:** Reviewed the dual-pipeline RTOS architecture. Analyzed data flow, queue synchronization, and state machine transitions to ensure zero data loss during mode switching.
- **HAL Layer Analysis:** Deep dived into STM32 HAL source code to better understand SPI/DMA callbacks and interrupt handling for the upcoming sensor integration. 
- **C/C++ Optimization:** Reviewed pointer arithmetic and memory management strategies to optimize the RTOS queues for high-frequency vibration data.

## Next Steps:
- Begin implementing the actual SPI driver for the ADXL345 sensor.
# Engineering Log

## Today's Focus
Today was dedicated to the precise calculation of buffer sizes and the configuration of the DMA (Direct Memory Access) tailored specifically for the **Julectron EdgeGuard** project. 

## Engineering Insight
This phase of the project involves challenges that cannot be solved by simply following textbooks or online references. Beyond a certain point, engineering requires deep practical skills, critical thinking, and creativity to adapt hardware configurations to real-world project constraints.

## Next Steps
- Finalize the DMA configuration and link it with the SPI and Timer peripherals.
- Begin the actual sensor data acquisition logic.