# Embedded-Systems
This repository contains my university embedded systems lab work and mini-projects built around Arduino/ATmega328P and low-level embedded programming. The main goal of this repo is to document both practical implementation and the core concepts behind embedded systems, including GPIO control, ADC reading, timers, interrupts, serial communication, and peripheral interfacing.

## Overview

Throughout these labs, I worked on small but important embedded applications that show how software directly interacts with hardware. The projects include LED control, joystick direction detection, analog signal acquisition, LCD output, sound monitoring, and serial communication with PC-side visualization.

This repository reflects hands-on practice with:

- Embedded C/C++
- AVR register-level programming
- Arduino-based prototyping
- Sensor and peripheral interfacing
- Real-time / non-blocking design
- Debugging and testing hardware-software systems

## Topics Covered

- GPIO input/output
- LED driving and timing
- Analog-to-digital conversion (ADC)
- Joystick input processing with thresholds and dead-zones
- Timers and sampling-rate considerations
- Serial communication (UART)
- 16x2 LCD interfacing
- Microphone/sound sensing
- Event indication with LEDs
- Low-level AVR concepts using the ATmega328P datasheet and AVR instruction set

## Lab Highlights

### 1. LED Blinking and Basic GPIO
Implemented basic LED control to understand digital outputs, delays, and pin configuration. This also included comparing higher-level Arduino programming with lower-level AVR-style control.

### 2. Joystick Direction Detection
Read X/Y analog values from a joystick module, converted them into discrete directions (**up, down, left, right**), and displayed the result using LEDs and Serial Monitor output.

### 3. Register-Level AVR Practice
Explored how embedded code maps to microcontroller registers and instructions, helping build a better understanding of how the ATmega328P works internally.

### 4. Sensor Interfacing and Display
Worked with external modules such as microphones and LCDs to build interactive systems that measure real-world inputs and provide immediate feedback.

### 5. Serial Visualization
Extended embedded applications with a Python-based serial interface / GUI to monitor system behavior from the PC side in real time.

## Hardware Used

- Arduino Uno / ATmega328P
- LEDs
- Resistors
- Joystick module
- Microphone module
- 16x2 LCD
- Potentiometer
- Breadboard and jumper wires

## Software / Tools

- Arduino IDE
- Embedded C/C++
- AVR register-level programming
- Serial Monitor
- Python (for GUI / serial visualization)
- ATmega328P datasheet
- AVR instruction set documentation
