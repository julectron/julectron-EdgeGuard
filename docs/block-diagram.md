# Julectron EdgeGuard - System Architecture




### System Overview
The primary goal of this project is to monitor the health and detect faults in an electric motor. This is achieved by using an **ADXL345 vibration sensor** to capture vibration data across three axes (X, Y, Z). We apply **FFT (Fast Fourier Transform)** to each axis to analyze the motor's condition in the frequency domain.

### Data Acquisition & Architecture
Delivering the sensor data to the CPU at the exact right time is critical. Therefore, the system requires a highly synchronized architecture. Currently, the planned architecture consists of:
- **SPI** protocol for high-speed sensor communication.
- **DMA (Direct Memory Access)** to offload the CPU.
- **Three separate buffers (X, Y, Z)** for continuous data streaming.

*(Note on Precise Timing: The system relies on a calculated timing mechanism. By defining the exact sample rate and buffer size, the time required to fill a buffer is mathematically determined. A hardware timer will manage this precise timing, triggering the CPU exactly when the buffer is full to ensure zero data loss and maximum reliability.)*

The ultimate goal of this architecture is to ensure that data reaches the processing unit at the correct time, leading to an accurate decision and a reliable final output.

### TinyML & Decision Engine
On the software side (Python), a machine learning model will be trained. Currently, during the data collection and architectural design phase, the specific model and features have not been finalized. The choice of the model may evolve as the system architecture and project conditions are further refined.

---
*Day 2 of the project. I do not claim to know everything, but I am trying my best to become the best.*
