**Course:** ENCE 3608 – Introduction to Embedded Systems  
**Instructor:** Dr. Alexz Farrall  
**Lab Number & Title:** Lab 1 - Blinking LEDs
**Student Name:** Farid Ibadov  
**Student ID:** 17954  
**Program / Section:** BSCE26  
**Date:** March, 2026

---

## 1. Objectives

In the first lab of Embedded Systems, the objective was to connect three LEDs to three **Arduino Uno**'s GPIO pins with each LED connected in series with an appropriate current-limiting resistor (to avoid damaging LED) with the pin configured as digital output.

After the circuit was built, starting with a clearly readable pattern and using deliberate delays, the update rate was progressively increased by *reducing/removing delays* and *simplifying the control logic* as well as *simplifying the loop structure*.

To record and report the *waveform frequency*, *period*, and *duty cycle* of one of the LED pins while the pattern was running for multiple update-rate versions of the code, an **oscilloscope** was used.

As speed and complexity increased, *timing variation* (jitter) in the *edge-to-edge timing* were observed. Finally, the fastest reliable pattern update rate achieved in software was identified as well as explanation to that was obvious.

In this lab report, appropriate theoretical background, system architecture and evaluation method are described to help understand the system behavior and underlying logic. 

---
## 2. Theoretical Background

### 2.1 Embedded Systems and Microcontrollers

As throughout this course all labs will be conducted using Arduino Uno, it is important to get familiar with it, as well as understand the advantages of using it.

Since Embedded Systems by being computational systems requiring low-cost, energy-efficient, yet powerful and autonomous hardware with corresponding software and firmware to perform specific tasks, Arduino, as a platform, meets most of those requirements. 

In this specific lab, Arduino Uno is an Embedded System with ATmega328P microcontroller chip which executes firmware to control LED outputs.

> A microcontroller unit (MCU) is essentially a small computer on a single chip. It is designed to manage specific tasks within an embedded system without requiring a complex operating system. [1]

### 2.2 GPIO Ports and Digital Logic Levels

Arduino Uno exposes 20 GPIO pins for general use.

>**GPIO (General-Purpose Input/Output)** is a universal digital interface used in microcontrollers, processors, and integrated circuits that allows for configurable input and output functions through software. Each GPIO pin can be individually programmed as either an *input* or an *output*, enabling flexible control of peripheral devices, reading of logic states, and digital signal exchange with other system components. [2]

Those 20 pins are grouped into 3 I/O ports:
- `PORTD`: D0 - D7
- `PORTB`: D8 - D13
- `PORTC`: A0 - A5

![image](Pasted%20image%2020260204115127.png)

Each port contains **8 bits** where each bit corresponds to one actual physical pin. In the image above, those can be traced. Those are controlled using internal register, which can be accessed through so-called "AVR Programming" where work on the registers can be done directly, giving higher efficiency and speed. Those are showed in Microprocessors Lab Report.

Digital signals operate at two logic levers:
- HIGH (logic 1) = 5V
- LOW (logic 0) = 0V

However, since readings are not always perfect, and internal/external disturbances my affect the signal, there are preset thresholds for those values to avoid misinterpreting the signal. Manufacturers define those in their datasheets, the most common of which is **TTL standard** (Transistor-Transistor Logic).

> TTL is a hardware voltage standard that tells digital circuits how to interpret electrical signals as binary 0 and 1.

![image](Pasted%20image%2020260228232841.png)

### 2.3 LEDs and Current-limiting Resistors

Unlike traditional metal-coil light bulbs, LED represent a semiconductor devices that quite sensitive to the current level. With an excessive current applied, they frequently tend to "burnout". Therefore, to reduce current going through LEDs, resistors with a particular resistance rating must be applied. These ratings can be calculated easily using **Ohm's Law**.
$$V = I \cdot R$$
Depending on the specification (usually color) simple LEDs have a specified **forward voltage** values. On average, as a rule of thumb, **2 Volts** are taken.

> Pins configured as `OUTPUT` with `pinMode()` are said to be in a **low-impedance state**. This means that they can provide a substantial amount of current to other circuits. ATmega pins can source (provide positive current) or sink (provide negative current) up to 40 mA (milliamps) of current to other devices/circuits. [3]

However, it is recommendable to keep it at **20mA** or less. Knowing current, running through the circuit, voltage, provided through the pins, and forward voltages of LEDs, appropriate resistor values are calculated through this formula:
$$R = \frac {V_{OH} - V_F}{I}$$
Where,
- $V_{OH}$ = Output High Voltage.
- $V_F$ = LED's Forward Voltage.
- $I$ = Current
- $R$ = Resistor Value

> *Note:* long leg of LED correspond to **+** while short to **-** . This tendency can be noted in many electronics components, but to double check, it is important to check the manufacturer's datasheet.

![image](Pasted%20image%2020260301012301.png)

### 2.4 Digital Timing and System Clock

Arduino Uno has **crystal oscillator** running at a **frequency** of *16 MHz*; accordingly, **clock period** is
$$T_{clk} = \frac{1}{16 \, MHz} = 62.5 \, ns$$
Since each instruction takes several clock cycles to be executed (depending on the instruction, as direct-register programming takes much time), as they do not occur instantaneously, execution time of a particular instruction or the whole program itself can be calculated by multiplying the number of clock cycles they require by **clock period** of the board. This concept is highly important to understand, since it explicitly demonstrates the timing limitations of the system. 

It is noteworthy that, although ATmega328P has its own internal 8Mhz RC oscillator, from factory it is set to "External Crystal Mode" and uses board's crystal instead since this option is more stable and fast.
### 2.5 Deterministic and Non-Deterministic Behavior

There are two types of behavior in which system works:
- **Deterministic** - for a given input, system will always produce exact output at exact time.
- **Non-Detereministic** - timing of the output may vary due to interrupts or conditional logic. As a result, **jitter** may occur, which is the variation in edge timing.

Since in the following lab behavior is linear and has not any condition, this type of behavior is considered as deterministic.

### 2.6 Blocking and Non-Blocking Behavior

While writing a program for a specific task, two types of behavior can be observed depending on the logic the program will have:
- **Blocking Delay behavior**:
	- Is when execution of program is deliberately paused for a fixed duration. The CPU doesn't perform anything else during the delay. No sampling of inputs. No updates on the state. No handling of communications.
- **Non-Blocking Delay behavior**:
	- Is when the CPU is never intentionally paused. Time is not waited for, but measured. The program never stops looping. Every pass verifies: Has enough time gone by? Is there an active input? Is a task prepared for execution?

### 2.7 Duty Cycle

> **Duty cycle** is the ratio of time a load or circuit is ON compared to the time the load or circuit is OFF. Duty cycle, sometimes called "duty factor," is expressed as a percentage of ON time. A 60% duty cycle is a signal that is ON 60% of the time and OFF the other 40%. [4]

Depending on the delay we set in our program for each LED, Duty cycle of individual LED, as well as whole system containing 3 LEDs can be determined and later proved using oscilloscope. 

### 2.8 Jitter and Edge Stability

> **Jitter** is defined as *short-term fluctuations* in the timing of the clock pulse, where the rising and falling edges deviate from their ideal positions. [5]

Fluctuations caused by jitter may be small, however, they introduce a risk of anomalies to occur, such as:
- Timing violations,
- Data corruption,
- Performance degradation, and etc.

There are several factors causing jitter:
- Clock instability,
- Noise and EMI/RFI inference,
- Clock frequency increase,
- Regular interrupts, and etc.

Jitter can be calculated using oscilloscope and the following formulas:
$$Jitter = t_{actual} - t{ideal}$$
$$J_{peak-to-peak} = t_{max} - t_{min}$$

In this lab, as time delays between LED's ON/OFF modes shrink (when they blink faster), **jitter** and **code complexity** increase. 

### 2.9 Software Overhead and Speed Limitations 

Traditional Arduino C++ programming, like `digitalWrite()` function, introduces huge overhead compared to direct register manipulation. This is due to the fact that, pin lookups should be carried before, including abstraction layers and multiple instruction contained within the function. 

For instance, On a standard 16MHz Arduino (AVR-based like the Uno)`digitalWrite()` takes approximately *50 to 60+* clock cycles to execute, lasting about $3.5 - 4 \, \mu 𝑠$, while typical **direct write** only 2 cycles, lasting $0.125 \, \mu s$.

---

## 3. Hardware & Configuration

Having enough background knowledge, simple system design can be sketched. One handy resource for that is [wokwi.com](https://wokwi.com/) which is an online simulation tool with a rich library, allowing to deploy desired scheme on various boards, like Arduino, ESP32, STM32, Pi, before working with the real hardware. This is quite helpful, especially for beginners, since it helps to avoid potential mistakes using real hardware that can be damaged during improper work. 

### 3.1 Pin Mapping / Wiring

![image](Pasted%20image%2020260301011725.png)

| Component | Connection                             | Type          |
| --------- | -------------------------------------- | ------------- |
| LED1      | `+` to Digital **pin 13**<br>`-` to R1 | any color     |
| LED2      | `+` to Digital **pin 12**<br>`-` to R2 | any color     |
| LED3      | `+` to Digital **pin 11**<br>`-` to R3 | any color     |
| R1        | LED1 and **GND**                       | $330 \, \ohm$ |
| R2        | LED2 and **GND**                       | $330 \, \ohm$ |
| R3        | LED3 and **GND**                       | $330 \, \ohm$ |

The values of resisters were chosen as $330 \ohm$. Using formula 
$$R = \frac {5V - 2V}{20\,mA} = 150\ohm$$

$150\ohm$ defined to be minimum resistance value for the current limiting resistor to connect in series with LED without damaging it, meaning, it is possible to connect any resistor with its value higher that $150\ohm$ - the higher the resistance, the dimmer LED will blink.

---

## 4. Implementation (Code Walkthrough)

```c title:"Lab1.io"
int LED1 = 13;
int LED2 = 12;
int LED3 = 11;

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);  
}

void loop() {
  digitalWrite(LED1, HIGH);
  delay(100);
  digitalWrite(LED1, LOW);
  delay(100);

  digitalWrite(LED2, HIGH);
  delay(200);
  digitalWrite(LED2, LOW);
  delay(200);

  digitalWrite(LED3, HIGH);
  delay(50);  
  digitalWrite(LED3, LOW);
  delay(50); 
}
```

### 4.1 Pin Configuration

```c
int LED1 = 13;
int LED2 = 12;
int LED3 = 11;
```

3 variables are initialized with appropriate names corresponding to each LED connected to the particular Arduino digital pin, as was described in Hardware Configuration. This helps to make code more readable, as manipulating the pins using only their numbers could be confusing. 

Example:
```c
pinMode(LED1, OUTPUT);
digitalWrite(LED1, HIGH);
```
is the same as 
```c
pinMode(13, OUTPUT);
digitalWrite(13, HIGH);
```
however, second option gives better visual control over code.

### 4.2 GPIO Configuration

```c
void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);  
}
```

- `void` means function `setup()` does not return anything - its purpose is initial configuration which is **run only once** with each reset/power-up. 
- `pinMode(pin, OUTPUT)` - configures pin as digital output ($5V - 0V$). By default pins are in input mode, therefore it is important to specify it.

### 4.3 Infinite Loop

```c
void loop() {
  digitalWrite(LED1, HIGH);
  delay(100);
  digitalWrite(LED1, LOW);
  delay(100);

  digitalWrite(LED2, HIGH);
  delay(200);
  digitalWrite(LED2, LOW);
  delay(200);

  digitalWrite(LED3, HIGH);
  delay(50);  
  digitalWrite(LED3, LOW);
  delay(50); 
}
```

- `loop()` continuously forever once the firmware is written onto the board and there's a power supply to it. It will constantly repeat the same sequence.

#### 4.3.1 LED1 Blinking Pattern

```c
  digitalWrite(LED1, HIGH);
  delay(100);
  digitalWrite(LED1, LOW);
  delay(100);
```

- Using `delay()` we can pause the program for a specific amount of time. Number inside the parenthesis specify time in **milliseconds**.
- This code snippet literally means: "**turn on** the LED and wait for 100 milliseconds, then **turn off** the LED and wait for another 100 milliseconds"

- **Period**:
$$T = 100\,ms\,+100\,ms\,+200\,ms\,+200\,ms\,+50\,ms\,+50\,ms\,=\,200\,ms$$
- **Frequency**:
$$f = \frac {1}{T} = \frac{1}{700\,ms}= 1.43\,Hz$$
- **Duty Cycle**:
$$D = \frac {100}{700} = 14.3\%$$


#### 4.3.2 LED2 Blinking Pattern

```c
  digitalWrite(LED2, HIGH);
  delay(200);
  digitalWrite(LED2, LOW);
  delay(200);
```

- **Period** for LED2 to blink:
$$T = 700\,ms$$
- **Frequency**:
$$f = \frac {1}{T} = \frac{1}{700\,ms}= 1.43\,Hz$$
- **Duty Cycle**:
$$D = \frac {200}{700} = 28.6\%$$

#### 4.3.3 LED3 Blinking Pattern

```c
  digitalWrite(LED3, HIGH);
  delay(50);  
  digitalWrite(LED3, LOW);
  delay(50); 
```

- **Period** for LED3 to blink:
$$T = 700\,ms$$
- **Frequency**:
$$f = \frac {1}{T} = \frac{1}{700\,ms}= 1.43\,Hz$$
- **Duty Cycle**:
$$D = \frac {50}{700} = 7.1\%$$

### 4.4 Blocking delay

Due to the `delay()` in each LED blink, The next LED does not change its state until the state of the previous LED changes. This is why each LED blinks one after another, sequentially, not simultaneously.

Total time for one full iteration of `loop()` is:
$$T = T_1+T_2+T_3 \, =200+400+100=700\,ms$$
So the program's update frequency is:
$$f = \frac{1}{0.7} = 1.43\,Hz$$

---

## 5. Results & Evidence

### 5.1 LED 1

```cpp
  digitalWrite(LED1, HIGH);
  delay(100);
```

![image](Pasted%20image%2020260320115919.png)

```cpp
  digitalWrite(LED1, LOW);
  delay(100);
```

![image](Pasted%20image%2020260320120323.png)

### 5.2 LED 2

```cpp
  digitalWrite(LED2, HIGH);
  delay(200);
```

![image](Pasted%20image%2020260320115813.png)

```cpp
  digitalWrite(LED2, LOW);
  delay(200);
```

![image](Pasted%20image%2020260320120304.png)

### 5.3 LED 3

```cpp
  digitalWrite(LED3, HIGH);
  delay(50); 
```

![image](Pasted%20image%2020260320115628.png)

```cpp
  digitalWrite(LED3, LOW);
  delay(50); 
```

![image](Pasted%20image%2020260320120340.png)


---

## 6. Conclusion

Overall, in this lab simple LED blinking system was implemented using Arduino Uno digital output pins and current-limiting resistors. 
Although the task itself was basic, it helped to understand several core ideas that will be used in later labs, such as GPIO configuration, digital logic levels, timing with `delay()`, and the relation between software instructions and physical output behavior. 
The oscilloscope observations also showed that even a very small program can be analyzed in terms of period, frequency, duty cycle, and timing stability. Overall, this lab served as a practical starting point for working with embedded systems both in hardware and in code.

---

## 7. References

1. https://www.ibm.com/think/topics/microcontroller
2. https://www.tme.eu/en/news/library-articles/glossary/page/61888/gpio-general-purpose-input-output-definition/
3. https://docs.arduino.cc/learn/microcontrollers/digital-pins/
4. https://www.fluke.com/en/learn/blog/electrical/what-is-duty-cycle
5. https://community.element14.com/learn/learning-center/the-tech-connection/b/blog/posts/what-is-jitter
