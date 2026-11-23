# Engineering Log

## Project Architecture Overview & Recent Progress

I have mentally divided the EdgeGuard project into four main pillars:
1. **Embedded System Pipeline (Data Acquisition to Preprocessing)**
2. **Real-Time Operating System (RTOS) Architecture**
3. **TinyML Model Training (Python) & C++ Weight Extraction**
4. **Real-World Data Collection & Dataset Engineering** (The last two are complementary).

### Current Status:
- **RTOS Architecture:** The overall architecture is designed, though some refinements are still pending.
- **Embedded System Design:** Peripherals are configured. The buffer size is strictly calculated to synchronize with the SPI sampling rate, avoid CPU interference, and maintain a valid window size for FFT.
- **Code Implementation:** The sensor read function, DMA callbacks, and prototypes are defined.

### Architectural Principles: Preprocessing Order
I gained valuable insights regarding the **Preprocessing Pipeline**. I had to carefully analyze and determine the correct sequence for:
- DC Offset removal
- Windowing (Hann)
- FFT implementation

Establishing these core principles is critical because the wrong order directly impacts the quality of time-domain and frequency-domain features, ultimately affecting the accuracy of the decision engine. This phase required deep architectural analysis to ensure system reliability. I adopted a precision-oriented and perfectionist approach here, which is necessary for this specific stage. As a result, I fell slightly behind on GitHub updates for a few days.
### Update on Embedded Firmware
The DSP Pipeline (Data Acquisition, Preprocessing, and Feature Extraction) is now fully implemented. You can review the complete source code here:
- [Embedded Firmware Code](embedded-firmware.c)

### Why I am documenting this:
I am sharing this to ensure my documentation reflects **genuine experience and architectural reasoning**, rather than copied tutorials. I hope this helps others navigating the same path. The embedded system design phase is nearing completion and will be published soon.