# Motor and Bearing Basics (For EdgeGuard Project)

## 1. Motivation for Dataset Selection
As the first part of the project (Embedded System to Processing) and the RTOS architecture are completed, the next step is the TinyML phase. However, the most critical component of TinyML is **high-quality, real-world data**. 

Since this is my first project in electric motor condition monitoring, I decided to acquire a validated dataset instead of collecting raw data from an improperly mounted sensor, which could lead to unrealistic values. Therefore, I obtained the **CWRU (Case Western Reserve University) Bearing Dataset**.

## 2. Electric Motor Fundamentals
A typical electric motor consists of:
- **Rotor:** The rotating part.
- **Stator:** The stationary part.
- **Shaft:** The connection point for the load.
- **Bearings:** To reduce friction and support the shaft.

## 3. Bearing Anatomy and Faults
Bearing failure is one of the most common motor faults. A bearing consists of:
- **Inner Race (IR):** Attached to the rotating shaft.
- **Outer Race (OR):** Attached to the stationary motor housing.
- **Rolling Elements (Balls):** Located between the races.
- **Cage:** Maintains the spacing of the rolling elements.

Common causes of bearing failure include misalignment, improper installation, and inadequate lubrication. 

## 4. Bearing Fault Frequencies
Bearing defects generate specific vibration frequencies, which are crucial for the Decision Engine:
- **BPFI (Ball Pass Frequency Inner):** Inner race fault.
- **BPFO (Ball Pass Frequency Outer):** Outer race fault.
- **BSF (Ball Spin Frequency):** Rolling element (ball) fault.
- **FTF (Fundamental Train Frequency):** Cage fault.

## 5. Sensor Role
The ADXL345 sensor does not inherently understand these terminologies. It only measures **vibration and acceleration** and converts them into raw **X, Y, and Z axis** data. The algorithm (FFT and Feature Extraction) is responsible for translating these raw values into fault signatures.