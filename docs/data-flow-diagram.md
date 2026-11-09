# Julectron EdgeGuard - Data Flow Architecture

<img width="512" height="768" alt="data-flow-diagram png" src="https://github.com/user-attachments/assets/32a1f5f5-247d-4eb0-ad0a-258934bc6b29" />


### Design Philosophy
System design starts with defining the problem: What do we want to achieve, and what must we do? We create a logical data flow diagram and follow it step by step.

### Step 1: Configuration
The first step is enabling the peripherals involved in the project (RCC). Then, the GPIO pins must be configured as Alternate Function (AF) and assigned to the SPI peripheral. 
*Design Note: It is highly recommended to encapsulate the configuration of each peripheral within an independent function for modularity.*

### Step 2: Buffer Definition
After enabling the peripherals, we must design the data buffering strategy. The ADXL345 sensor provides data across three axes (X, Y, Z). Therefore, a separate buffer is defined for each axis. The buffer size is determined based on the data rate, data size, and processing capability.

### Step 3: Synchronization
When a buffer is full, the architecture can handle it in different ways. The preferred approach is a combination of precise timing (hardware timer) and interrupts. This ensures the CPU is not busy waiting; instead, it is triggered to read the data exactly when a buffer is ready.

### Step 4: Preprocessing
Before the CPU processes the data, we must prepare it. This includes:
- Filtering
- Normalization
- Quantization and Standardization (if needed)
After this, the data is ready for DSP processing and feature extraction.

> ⚠️ **CRITICAL WARNING: Preprocessing Consistency**
> Preprocessing plays a vital role in data quality. The preprocessing steps applied to the data during the **TinyML model training phase** MUST be identical to the steps applied during **real-time execution**. If the pipeline differs, the system's predictions will fail. 

### Step 5: Final Decision
After merging the extracted model weights into the pipeline, the system reaches the final decision stage. A final decision is made by combining the TinyML model output and the manually defined threshold. 
*(Design Note: The threshold is defined manually by the designer because it represents the client's risk tolerance and safety limits, requiring precise calculation and fine-tuning.)*

### Final Output
The final decision leads to the system output (e.g., LED alarm or UART data).

---
*End of Day 2. I do not claim to know everything, but I strive to be better than I was yesterday.*
