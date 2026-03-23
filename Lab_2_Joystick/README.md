**Course:** ENCE 3608 – Introduction to Embedded Systems  
**Instructor:** Dr. Alexz Farrall  
**Lab Number & Title:** Lab 2 - Joystick
**Student Name:** Farid Ibadov  
**Student ID:** 17954  
**Program / Section:** BSCE26  
**Date:** March, 2026

---

## 1. Objectives

In this lab, the objective was to read a joystick’s X/Y axes and convert them into four discrete directions (up, down, left, right) using **thresholds** and a **dead-zone**. By illuminating the corresponding **LED** for the detected direction and printing the **direction** text to the serial monitor in real time, one can visually observe the work of this electronical peripheral. 

---
## 2. Theoretical Background

### 2.1 Digital vs Analog Signals

Joystick module consists two potentiometers placed perpendicularly, motion of each represent one of two axes: X and Y (pins **VRX** and **VRY**). Additionally, it has one push button (**SW**) which behaves like a switch; however, in the scope of this lab, it was not used.
![image](images/Pasted%20image%2020260302193950.png)
Potentiometers are electrical components that help to regulate the voltage going through them. At rest, those potentiometers placed at the middle position, meaning , they pass only half of the voltage coming.

But voltage levels are not discrete and signals produced by those two potentiometers represent analog signal. In order for the Arduino to comprehend and understand the position of joystick, those analog signals need to be converted onto digital. This is done using ADC converter.

### 2.2 Analog to Digital Converter

> The Arduino Uno has 6 onboard ADC (**Analog to Digital Converter**) channels which can be used to read analog signals in the 0–5 V range. It has a **10 bit ADC**, which implies that it’ll give a digital value in the range of 0–1023 (2¹⁰). [1]

![image](images/Pasted%20image%2020260302195505.png)

To calculate the Digital Output, the following formula can be used:
$$ADC\;Value = \frac{V_{in}}{VREF}\cdot(2^n − 1)$$
Where,
- $ADC\;Value$ = 0 ... 1023
- $V_{in}$ = voltage value from potentiometer
- $VREF$ = reference voltages

There are 3 types of reference voltages that can be used:
- `analogReference(INTERNAL)` - Use the internal 1.1V reference
- `analogReference(DEFAULT)` - Use the default 5V or 3.3V reference
- `analogReference(EXTERNAL)` - Use external voltage applied to AREF pin

Smaller VREF -> smaller step size -> finer measurement resolution.

On Arduino Uno operating volage is used as reference by default: **5V**

So, when joystick is at rest, each potentiometer has the following ADC Value (rounded):
$$ADC\;Value = \frac{2.5V}{5V}\cdot(2^{10} − 1) = 512 $$

### 2.3 Prescaler, Conversion Time, Sampling rate

**Prescaler** is basically divider which divides clock values to use suitable frequency. Greater prescaler -> lower frequency.

On Arduino Uno, ADC uses system clock, running at 16 MHz and default prescaler as 128. So ADS, by default, operates at a frequency of
$$ADC\,Clock = \frac{16\,MHz}{128} = 125\,kHz$$
Default value was set as a balance between speed and accuracy.

Each conversion takes about **13 ADC clock cycles**, so conversion time is approximately (first conversion takes 25 cycles due to initialization processes). So, the **Conversion Time** (time needed to covert one analog signal into appropriate digital signals) is calculated like this:
$$Conversion\;Time = 13 \cdot \frac{1}{125\,kHz} = 104\,\mu s$$
Knowing Conversion Time, Sampling rate can be calculated, which indicates how many samples (conversions) can be done in 1 second:
$$Sampling\;Rate = \frac{1}{Conversion\;Time} = \frac{1}{104\,\mu s} = 9,600\,SPS$$

If the prescaler value is decreased, the ADC clock frequency increases, conversion time becomes shorter, and the sampling rate increases. However, if the ADC clock becomes too high, conversion accuracy may worsen and the readings may become less stable. Therefore, it is recommended to keep the ADC clock frequency below $200\,kHz$.

### 2.4 Joystick Module - Closer Look

To understand how each potentiometer in a joystick is placed, joystick should be looked at from top-down orientation with its pins looking towards West and then one can refer to the official datasheet of the manufacturers, since in some modules directions may be flipped. [2]

![image](images/Pasted%20image%2020260302204458.png)

Important to keep in mind that, as any physical component, potentiometers in joystick module tend to gain bias after a long time of exploitation, meaning, they may drift. Therefore, it is crucial to specify some dead-zones by introducing **thresholds**, after which signal can be sampled. 

---
## 3. Hardware & Configuration

![image](images/Pasted%20image%2020260302211023.png)

### 3.1 Pin Mapping

| Component    |            | Arduino Connection | Type       |
| ------------ | ---------- | ------------------ | ---------- |
| **Joystick** | VCC (+5V)  | **5V** pin         |            |
|              | GND        | **GND** pin        |            |
|              | VERT (VRX) | **A0** pin         |            |
|              | HORZ (VRY) | **A1** pin         |            |
|              | SEL (SW)   | none               |            |
| LED1         |            | pin **D8**         | **Blue**   |
| LED2         |            | pin **D9**         | **Orange** |
| LED3         |            | pin **D10**        | **White**  |
| LED4         |            | pin **D11**        | **Red**    |
| R1           |            | LED1               | $220\ohm$  |
| R2           |            | LED2               | $220\ohm$  |
| R3           |            | LED3               | $220\ohm$  |
| R4           |            | LED4               | $220\ohm$  |

### 3.2 Configuration Choices

As was mentioned before, axes might be flipped depending on the model of the module. The easiest way to overcome the code incompatibility is to flip the wires between terminals, adjusting desired direction of work. 

---

## 4. Implementation (Code Walkthrough)

### 4.0 Code and System Diagram

```c title:"Lab2.io"
// Joystick (A0/A1) -> 4 directions -> 4 LEDs + Serial
// LEDs: D8=UP, D9=DOWN, D10=LEFT, D11=RIGHT

const int JOY_X = A0;
const int JOY_Y = A1;

// UP, DOWN, LEFT, RIGHT
const int ledPins[4] = {8, 9, 10, 11};                 
const char* names[5] = {"NEUTRAL", "UP", "DOWN", "LEFT", "RIGHT"};

const int CENTER = 512;
const int DEADZONE = 60;

int lastDir = 0; // 0=NEUTRAL, 1=UP, 2=DOWN, 3=LEFT, 4=RIGHT

void setLeds(int dir) {
  // turn all off
  for (int i = 0; i < 4; i++) digitalWrite(ledPins[i], LOW);

  // turn one on if not neutral
  if (dir != 0) digitalWrite(ledPins[dir - 1], HIGH);
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 4; i++) pinMode(ledPins[i], OUTPUT);
  setLeds(0);
}

void loop() {
  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);

  int low  = CENTER - DEADZONE;
  int high = CENTER + DEADZONE;

  int dir = 0; // neutral by default

  // same priority: LEFT/RIGHT first, else UP/DOWN
  if      (x < low)  dir = 3;   // LEFT
  else if (x > high) dir = 4;   // RIGHT
  else if (y < low)  dir = 2;   // DOWN
  else if (y > high) dir = 1;   // UP

  if (dir != lastDir) {
    setLeds(dir);
    Serial.println(names[dir]);
    lastDir = dir;
  }

  delay(20);
}
```

![image](images/Joystick%20Direction%20Control-2026-03-09-171130.png)
### 4.1 Pin Configuration

```c
const int JOY_X = A0;
const int JOY_Y = A1;
const int ledPins[4] = {8, 9, 10, 11}; 
```

- Initialize variables `JOY_X` and `JOY_Y` and assign to them pins **A0** and **A1**. Those will be responsible for readings from two potentiometers (Vertical and Horizontal)
- Through this array we define 4 LED by individual index, so that configuration more obvious through the table:

| Index | Indication | Pin |
| ----- | ---------- | --- |
| 0     | UP         | **D8**  |
| 1     | DOWN       | **D9**  |
| 2     | LEFT       | **D10** |
| 3     | RIGHT      | **D11** |

### 4.2 Serial Output

```c
const char* names[5] = {"NEUTRAL", "UP", "DOWN", "LEFT", "RIGHT"};
```

- those are the messages that will be printed in a serial output depending on the direction the user points to using joystick's knob.

- On the line 47 of code, `Serial.println(names[dir]);` can be observed, which is responsible for displaying those messages, depending on the particular direction

| `dir` | `names[dir]` |
| ----- | ------------ |
| 0     | NEUTRAL      |
| 1     | UP           |
| 2     | DOWN         |
| 3     | LEFT         |
| 4     | RIGHT        |

### 4.3 Thresholding 

```c
const int CENTER = 512;
const int DEADZONE = 60;
```

- As was specified before, when joystick is idle, each potentiometer passes just half of the voltage, in ADC value it is half of 1023, which is 512. So `CENTER` is specified as 512.  
- Threshold value is specified as 60. This value can be selected at random, through testing. Once again, this is important in order to avoid false readings and create "safe" neutral interval:

```c
int low  = CENTER - DEADZONE;
int high = CENTER + DEADZONE;
```

### 4.4 State Memory

```c
int lastDir = 0;
```

This is done to prevent repeated printing of the same message with each iteration of loop, by storing previously detected direction, so in serial monitor, output appears only when the direction is changed.

### 4.5 LED control

```c
void setLeds(int dir) {
  for (int i = 0; i < 4; i++) 
	  digitalWrite(ledPins[i], LOW);

  if (dir != 0)
	  digitalWrite(ledPins[dir - 1], HIGH);
}
```

This function does the following:
- Turns all LEDs off each time to clear the previous direction
- If the direction is not neutral - turns on exactly one LED

`[dir - 1]` since direction values are 1, 2, 3, 4 and the LED array indexes are from 0 to 3, so that.

### 4.6 Initial Setup

```c
void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 4; i++) 
	  pinMode(ledPins[i], OUTPUT);
  setLeds(0);
}
```

- `Serial.begin(9600);` to open serial communication with an appropriate baud rate.
- configuring all the LEDs as an output.
- setting all the LEDs off, ensuring that the system starts with a NEUTRAL state.

### 4.7 Main Loop

This section is explained using comments on the top of each line of the code.

```c
void loop() {
  // read ADC-converted analog values from joystic 
  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);

  // defining thresholds
  int low  = CENTER - DEADZONE;
  int high = CENTER + DEADZONE;

  // neutral by default
  int dir = 0;

  // if else statements for identifying the directions
  // those are mutually exclusive - one LED at a time
  // since LEFT and RIGHT are checked first, if knob is
  // pushed diagonally, those directions are displated first
  if      (x < low)  dir = 3;   // LEFT
  else if (x > high) dir = 4;   // RIGHT
  else if (y < low)  dir = 2;   // DOWN
  else if (y > high) dir = 1;   // UP

  // when direction changes: updates LED, prints direction,
  // updates memory
  if (dir != lastDir) {
    setLeds(dir);
    Serial.println(names[dir]);
    lastDir = dir;
  }
  // small delay to limit the update rate
  delay(20);
}
```


---

## 5. Results & Evidence

### 5.1 Expected LED Behavior and Serial Output

When the joystick is moved in a particular direction the corresponding LED turns on and the direction is being written in the serial monitor as follows:

- **UP** -> **LED1** (Blue) turns on and in Serial monitor `UP` message is displayed:
![image](images/Pasted%20image%2020260305140400.png)

- **DOWN** -> **LED2** (Orange) turns on and in Serial monitor `DOWN` message is displayed:
![image](images/Pasted%20image%2020260305140409.png)

- **RIGHT** -> **LED3** (Red) turns on and in Serial monitor `RIGHT` message is displayed:
![image](images/Pasted%20image%2020260305140418.png)

- **LEFT** -> **LED4** (White) turns on and in Serial monitor `LEFT` message is displayed:
![image](images/Pasted%20image%2020260305140426.png)


Note that after each release, joystick goes to the NEUTRAL position and the following message is being displayed in Serial monitor. 
### 5.2 Stability at Neutral

Can be observed in Lab 4 Report, where small mechanical drift is observed; however, the stability remains.  

### 5.3 Direction Priority

As was explained in code overview, the if-else statement responsible for direction set considers LEFT and RIGHT directions first, meaning those are in priority when compared with UP and DOWN. Therefore, the UP and DOWN directions are set, when the know is perfectly aligned with the axis they sit on. In the future, more functionality may be introduced when considering transition-directions, like North-West and etc., for now, it is just a peculiarity of the program.

![image](images/Pasted%20image%2020260308230941.png)

- here the joystick was moved in 4 diagonal directions, demonstrating that the program priorities horizontal directions first. 
### 5.4 System Responsiveness

The last important thing the program includes is `delay(20)`. This means the loop waits 20 ms after each iteration, so the system updates at about 50 times per second. This helps to make the system more stable and easy to observe, while without it, system will iterate extremely fast which is way too excessive. In one sentence, there is no sense of pushing the limits of hardware while executing simple tasks.

---
## 6. Conclusion

Overall, this lab focuses on the development of a trivial system to read joystick analog signals and parse them into discrete directional commands. X and Y voltages of the joystick's potentiometers were measured using the Arduino's ADC, and the direction of the knob's movement was determined using threshold comparisons through dead-zone specification. LEDs were used as indicators of the direction detected, and the serial monitor to display orientation literally.  
  
By preventing false triggers due to slight signal fluctuations near the joystick's center position, which can be caused by physical imperfections of the peripherals, implementation of a dead-zone has highly increased the system reliability. Additionally, considering previous direction in decision making ensured that the output updates only when the direction is changed - this helped to avoid flooding the Serial Monitor with repetitive information.  
  
This lab highlights basic concepts such as:  
- real-time input processing,  
- threshold-based decision making 
- analog to digital conversion.  
  
Conducting multiple practice runs demonstrated how deterministic input manipulation and informative feedback means were effectively implemented in the scope of such a simple, yet practical and educational lab work.

---

## 7. References

1. https://medium.com/vicara-hardware-university/arduino-adc-2b1df1541ca9
2. https://components101.com/modules/joystick-module
