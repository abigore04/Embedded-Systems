# Embedded-Systems

This repository contains my university Embedded Systems lab work and mini-projects built around Arduino Uno / ATmega328P, peripheral interfacing, actuation, contactless communication, and PC-side visualization.

The main goal of this repository is to document both practical implementation and the core concepts behind embedded systems, including GPIO control, ADC reading, timers, interrupts, serial communication, actuator control, RFID/IR communication, GUI integration, and hardware-software testing.

## Overview

Throughout these labs, I worked on small but important embedded applications that show how software directly interacts with hardware. The projects include LED control, joystick direction detection, analog signal acquisition, LCD output, sound monitoring, serial communication with PC-side visualization, actuator-based reaction game logic, and a contactless security system using IR and RFID.

This repository reflects hands-on practice with:

- Embedded C/C++
- Arduino-based prototyping
- AVR / ATmega328P concepts
- Sensor and peripheral interfacing
- GPIO, ADC, timers, and serial communication
- Real-time / non-blocking design
- State-machine based logic
- Servo and stepper motor control
- RFID and infrared communication
- Python GUI development with PyQt6
- CSV and SQLite-based data logging
- Hardware debugging and oscilloscope-based validation

## Topics Covered

- GPIO input/output
- LED driving and timing
- Analog-to-digital conversion (ADC)
- Joystick input processing with thresholds and dead-zones
- Timers and sampling-rate considerations
- Serial communication through UART / USB serial
- 16x2 LCD interfacing
- Microphone / sound sensing
- Event indication with LEDs and buzzers
- Servo motor position control
- Stepper motor control using ULN2003 driver
- Random timing and reaction-time measurement
- False-start detection logic
- PyQt6 GUI communication with Arduino
- CSV logging and simple statistics
- Infrared remote decoding using NEC protocol
- RFID-RC522 tag reading over SPI
- Keypad-based code entry
- SQLite database logging for RFID scans
- Low-level AVR concepts using the ATmega328P datasheet and AVR instruction set

## Lab Highlights

### 1. LED Blinking and Basic GPIO

Implemented basic LED control to understand digital outputs, delays, and pin configuration. This also included comparing higher-level Arduino programming with lower-level AVR-style control.

### 2. Joystick Direction Detection

Read X/Y analog values from a joystick module, converted them into discrete directions such as **up, down, left, and right**, and displayed the result using LEDs and Serial Monitor output.

### 3. Register-Level AVR Practice

Explored how embedded code maps to microcontroller registers and instructions, helping build a better understanding of how the ATmega328P works internally.

### 4. Sensor Interfacing and Display

Worked with external modules such as microphones and LCDs to build interactive systems that measure real-world inputs and provide immediate feedback.

### 5. Serial Visualization

Extended embedded applications with a Python-based serial interface / GUI to monitor system behavior from the PC side in real time.

### 6. Reaction Game with Actuators

Designed a two-player reaction game using two push buttons, a buzzer, a servo motor, and a 28BYJ-48 stepper motor driven through a ULN2003 module.

The Arduino generates a random waiting time, triggers a buzzer as the start signal, measures each player’s reaction time, detects false starts, and updates the score. The servo points toward the round winner, while the stepper motor works as a physical tug-of-war score indicator.

A PyQt6 GUI was also used to enter player names, start/reset the match, receive serial results from Arduino, save round and match data into CSV files, and display basic statistics.

### 7. Contactless Communication and Security System

Built a small security system using a 4x4 membrane keypad, IR remote, IR receiver, RFID-RC522 module, passive RFID tags, two LEDs, and a PyQt6 GUI.

The system works in three main states:

- `WAITING_FOR_CODE`
- `LOCKED`
- `UNLOCKED`

The user first sets a 4-digit code using the keypad. After that, the system enters locked mode and disables RFID scanning. To unlock it, the same code must be entered using the IR remote. Once unlocked, RFID scanning becomes active.

When an RFID tag is scanned, Arduino sends the UID to the PC-side GUI using serial communication. The GUI stores the tag in an SQLite database, assigns an ID to new tags, increments the scan count for repeated tags, and updates timestamps.

This lab also included oscilloscope evidence for both IR receiver output and RC522 antenna behavior.

## Hardware Used

- Arduino Uno / ATmega328P
- LEDs
- Resistors
- Push buttons
- Joystick module
- Microphone module
- 16x2 LCD
- Potentiometer
- Buzzer
- Servo motor
- 28BYJ-48 stepper motor
- ULN2003 stepper motor driver
- 4x4 membrane keypad
- IR receiver module
- IR remote controller
- RFID-RC522 reader
- Passive RFID tags/cards
- Breadboard and jumper wires
- Oscilloscope for signal evidence

## Software / Tools

- Arduino IDE
- PlatformIO / VS Code
- Embedded C/C++
- AVR register-level programming
- Serial Monitor
- Python
- PyQt6
- PySerial
- SQLite
- CSV logging
- ATmega328P datasheet
- AVR instruction set documentation

## Main Skills Practiced

- Reading and interpreting datasheets
- Mapping Arduino pins to ATmega328P functions
- Connecting sensors and actuators correctly
- Designing state-machine based embedded logic
- Using serial communication between Arduino and PC
- Building simple PyQt6 GUIs for embedded systems
- Logging experimental results into CSV and SQLite files
- Testing hardware behavior with real measurements
- Explaining not only what the code does, but why the system works

## Notes

This repository is mainly educational. The projects were completed as part of ENCE 3608 – Introduction to Embedded Systems and focus on understanding embedded behavior from both software and hardware perspectives.
