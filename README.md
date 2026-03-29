# STM32-Mini-DSO
A mini Digital Storage Oscilloscope (DSO) built using STM32F103C8T6 (Blue Pill) and ST7789 TFT display. Captures analog signals using ADC + DMA and visualizes waveform in real-time with scaling and measurement features.
# STM32 Mini Digital Storage Oscilloscope (DSO)

## Project Overview

This project is a simple implementation of a **Digital Storage Oscilloscope (DSO)** using the STM32 Blue Pill and an ST7789 TFT display.

The goal of this project was to understand how real oscilloscopes work — from signal sampling to waveform visualization.

Even though the waveform is not perfectly stable yet, this project helped me deeply understand ADC, DMA, SPI, and real-time embedded systems.

---

##  What I Learned

* How to capture high-speed analog data using **ADC + DMA**
* How to process and display real-time signals
* Basics of **oscilloscope working (triggering, scaling, waveform plotting)**
* Efficient screen updates using SPI
* Debugging timing and sampling issues in embedded systems

---

##  Hardware Used

* STM32F103C8T6 (Blue Pill)
* ST7789 TFT Display (240x320)
* Function Generator (Input Signal)
* Push Buttons (for scaling control)

---
## Pin Configuration (Mini DSO)
## ST7789 Connections

| TFT Pin    | STM32 Pin | Description  |
| ---------- | --------- | ------------ |
| VCC        | 3.3V      | Power supply |
| GND        | GND       | Ground       |
| SCL (SCK)  | PA5       | SPI Clock    |
| SDA (MOSI) | PA7       | SPI Data     |
| RES (RST)  | PA0       | Reset pin    |
| DC (A0)    | PA1       | Data/Command |
| CS         | PA2       | Chip Select  |
| BLK        | 3.3V      | Backlight    |

---
## Input Signal (Function Generator)
| Signal Source | STM32 Pin            | Description    |
| ------------- | -------------------- | -------------- |
| Analog Input  | PA4 (ADC1 Channel 4) | Waveform input |

---
## Peripheral Configuration
## ADC Configuration
ADC: ADC1
Channel: Channel 4 (PA4)
Resolution: 12-bit
Mode: Continuous Conversion
Trigger: Software
Data Alignment: Right
Sampling Time: 1.5 cycles

## Used for 

high-speed signal sampling

## DMA Configuration (ADC)
DMA1 Channel1 → ADC
Mode: Circular / Continuous
Buffer Size: 320 samples

## Used for:

Fast data acquisition
CPU-free sampling

## SPI Configuration
SPI: SPI1
Mode: Master
Data Size: 8-bit
Clock Polarity: Low
Clock Phase: 1st Edge
Baud Rate Prescaler: 16

## Used for

TFT display communication

## DMA Configuration (SPI)
DMA1 Channel3 → SPI TX

## Used for:

Faster screen updates
Smooth waveform rendering

## GPIO Configuration
Output Pins:
PA0 → TFT Reset
PA1 → TFT DC
PA2 → TFT CS

##  Features Implemented

*  Real-time waveform display
*  ADC sampling using DMA
*  Basic trigger detection for stable waveform
*  Voltage scaling (Zoom in/out)
*  Time scaling
*  Measurement display:

  * Maximum Voltage
  * Peak-to-Peak Voltage
*  Grid display like an oscilloscope
*  Button-based control

---

##  Current Limitations

* Waveform is slightly **distorted / unstable**
* Sampling rate not perfectly optimized
* No proper anti-aliasing or filtering
* Trigger logic is basic

---

##  Future Improvements

* Improve ADC sampling timing accuracy
* Add better trigger modes (rising/falling edge)
* Implement filtering (low-pass)
* Improve UI and add waveform freeze
* Add frequency measurement

---

##  Output

(Currently shows waveform but needs improvement in stability)

---

##  Note

This project is still a **work in progress**, but it represents a strong learning step toward building a complete oscilloscope.

---

##  Author

Swapnali Rathod.
