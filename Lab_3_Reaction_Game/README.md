**Course:** ENCE 3608 – Introduction to Embedded Systems  
**Instructor:** Dr. Alexz Farrall  
**Lab Number & Title:** Lab 3 - Reaction Game
**Student Name:** Farid Ibadov  
**Student ID:** 17954  
**Program / Section:** BSCE26  
**Date:** March, 2026

---
## 1. Objectives

In this lab, a reaction-timing counter game was designed using the **DS1307 RTC** (real-time clock) and an external display - **3461BS 4-digit 7-egment display**.

The logic of the game is as follows:
- When the system is **idle**, the display shows **0**.
- When the user **starts the game**, by pressing on the **push-button**, counter starts to increment from **0 to 10** at fixed rate which is derived from external RTC module.
- There is some predefined number which should be caught as quickly as possible though the **second button press**. As far as user does not touch the button, the counter will loop going from 0 to 10 and back to 0: `0 → 1 → 2 → ... → 10 → 0 → 1 ...`
- If the button is pressed within **100 milliseconds** since the target value appeared, the player wins and an external **LED** starts to blink as an immediate feedback, indicating the win.
- Otherwise, when the press happens after 100 ms or during another number shown, the user loses, LED stays turned off and the game resets, saying idle at 0 and waiting for the next start through button press. 

The main objective of this lab was to understand communication buses used in Arduino through the practical work with **I2C serial communication** which is represented though pairing Arduino UNO board with an external RTC module. 

Another goal was to understand *practical advantages* of using an external RTC module for timing, rather than the board's main crystal oscillator.

---
## 2. Theoretical Background

### 2.1 Types of Communication Buses

Very frequently in embedded systems, communication between microcontroller and various peripherals is required. This can be achieved using **serial** or **parallel** communication buses. 

> **Serial interfaces** stream their data, *one single bit at a time*. These interfaces can operate on as little as one wire, usually never more than four. [1]

![image](images/Pasted%20image%2020260313220456.png)

> **Parallel interfaces** transfer *multiple bits at the same time*. They usually require buses of data - transmitting across eight, sixteen, or more wires. [1]

![image](images/Pasted%20image%2020260313220603.png)

There are some advantages and disadvantages of using each of them: [2]

| **Parallel Communication**                | **Serial Communication**                |
| ----------------------------------------- | --------------------------------------- |
| One wire per signal/bit                   | Few shared wires                        |
| Data appears simultaneously               | Data is time-encoded                    |
| Minimal protocol overhead                 | Requires clocks, framing, both          |
| Strong determinism at short distances     | Depends on protocol                     |
| Pin count scales linearly with data width | Pin count scales slowly, often constant |

However, one great disadvantage of parallel communication that makes serial communication more attractive is the **number of buses required**, which in case of parallel ends up in occupying too many ports, requiring lots of wiring and relatively more energy. As the number of ports is limited, the main objective of a neatly designed system, where enormous speed is not a main factor, is to use as less wiring as possible, along with reducing energy consumption, since parallel communication requires more energy than serial.

### 2.2 Types of Serial Communication Buses

There are lots of Serial Communication protocols. There are divided into 2 groups:
- **Synchronous** - data is being transmitted at a specific rate and rules, defined by shared clock. (**SPI**, **I2C**)
- **Asynchronous** - no shared clock, the data is being transmitted as frames, each having particular start and stop bits, highlighting them in a data stream. (**UART**)

By looking at Arduino Uno pinout, particular pins corresponding to each Serial Communication protocol can be traced:

![image](images/Pasted%20image%2020260313223440.png)


### 2.3 I2C Communication Protocol

Since in this lab, DS1307 RTC module is used, which works through **I2C**, the main focus of this lab report will be on this type of Serial Communication protocol. 

> **I2C** (**Inter-Integrated Circuit**) is a *two-wire* serial communication protocol using a **serial data line** (SDA) and a **serial clock line** (SCL). The protocol supports multiple target devices on a communication bus and can also support multiple controllers that send and receive commands and data. Communication is sent in byte packets with a unique address for each target device. [3]

I2C works using **master-slave architecture** where one device acts as a master, controlling the timing of the communication, generating the clock signal, and deciding when the data is sent or requested, while other device(s) respond(s) to those commands when it(they) is(are) addressed using it(their) specific address. 

Typical I2C **message** looks like this:
![image](images/Pasted%20image%2020260313235250.png)

- **Start Condition**
		- start, which is initiated by the master, happens when the data line (SDA) goes *from high to low* before the clock line (SCK), alerting all slaves that an address transmission will happen soon.
- **Address Frame**
		- typically consists out of 7 or 10 bits. Each slave has its address. For instance, for DS1307 RTC module default address is set to **0x68**. Master will refer to it using this address, which also can be changed, if address conflict appears.
- **Read/Write**
		- `1` refers to requesting data (Read)
		- `0` refer to sending data (Write)
- **ACK/NACK**
		- when `0` the address is acknowledged, meaning slave with that address exists and ready to respond.
		- when `1` means peripheral does not respond or unable to process the request.
- **Data Frame**
		- can be 1 or more, carries actual data.
- **Stop Condition**
		- happens when SDA goes high after SCL.

![image](images/Pasted%20image%2020260313235046.png)


As it can be seen in pinout diagram, Uno board has 2 pairs of pins supporting I2C communication. In this lab, **A4** (for **SDA**) and **A5** (for **SCL**) pins were used.

### 2.4 DS1307 Real-Time Clock (RTC) Module

Instead of using software-based time tracking though microcontroller's clock, which calculates only **elapsed time** and which tends to drift over time due to prescaler errors and interrupt scheduling (0.36 seconds error per hour [2]), a dedicated time-tracking device using **absolute time** is much better option when seeking precise long-term timing referencing. 

RTC modules are very handy for that purpose fore their ability to track absolute time. In this particular lab DS1307 RTC module was used. As it can be observed, it has its own power source and $32-kHz$ Crystal which allows it to keep track of the *seconds*, *minutes*, *hours*, *days*, *months*, and *year* information totally autonomously. The only thing is needed is to program those values when it is used for the first time by `rtc.set()` function. After that, this function can be commented or removed.

![image](images/Pasted%20image%2020260314005131.png)

Since RTC provides only absolute time, **elapsed time** can be easily calculated as the difference of two timestamps:
$$\Delta t = t_{stop} \; - \; t_{start}$$

### 2.5 7-Segment Display and Multiplexing

In this lab **3461BS** 4-digit 7-segment display was used. Each of 4 digits, visually representing numerical digits from 0 to 9, consists of 7 LED segments with an additional decimal point segment.  

![image](images/Pasted%20image%2020260314003922.png)

![image](images/Pasted%20image%2020260314001307.png)

Each segment is labeled with a corresponding letter from **a** to **g** in alphabetical order with additional **dp** segment for decimal point. Lighting some of them simultaneously will form desired digit:

| Digit | Active Segments     |
| ----- | ------------------- |
| 0     | a, b, c, d, e, f    |
| 1     | b, c                |
| 2     | a, b, d, e, g       |
| 3     | a, b, c, d, g       |
| 4     | b, c, f, g          |
| 5     | a, c, d, f, g       |
| 6     | a, c, d, e, f, g    |
| 7     | a, b, c             |
| 8     | a, b, c, d, e, f, g |
| 9     | a, b, c, d, f, g    |

One important thing to check before working with 7-segment displays is whether it is operating in **Common Anode** or **Common Cathode** mode. For instance, **3461BS** display works in Common Anode mode; however, some displays, like **5641AS**, work in Common Cathode mode. Therefore, it is important to refer to the manufacturer's datasheet.

- **Common Cathode**
		- all cathodes of a digit are connected together and a digit is activated only when common cathode is connected to the **GND**. Segments turn on when their corresponding pins get **high voltage**. 
- **Common Anode**
		- all anodes of a digit are connected together, and a digit is activated when the common anode is connected to **VCC**. Segments turn on when their corresponding pins get **low voltage**.


In 4-digit display all digits share same segment control lines; therefore, it is impossible to control all digits simultaneously and that is why **multiplexing** is used, by activating one digit at a time and quickly switching to the next digit. This is done so fast that the human's eyes do not see those switches. Without multiplexing, each digit would have its own control lines, ending up in 32 pins instead of 12 pins.

Multiplexing is performed using **D1-D4** pins.

---

## 3. Hardware & Configuration

![image](images/Pasted%20image%2020260314010024.png)

### 3.1 Pin Mapping

| Component                    | Pin / Signal        |                   Arduino Connection | Purpose                      |
| ---------------------------- | ------------------- | -----------------------------------: | ---------------------------- |
| **3461BS 4-digit 7-segment** | segment **a**       |                               **D2** | segment control              |
|                              | segment **b**       |                               **D3** | segment control              |
|                              | segment **c**       |                               **D4** | segment control              |
|                              | segment **d**       |                               **D5** | segment control              |
|                              | segment **e**       |                               **D6** | segment control              |
|                              | segment **f**       |                               **D7** | segment control              |
|                              | segment **g**       |                               **D8** | segment control              |
|                              | **D3**              |                              **D11** | second digit select          |
|                              | **D4**              |                              **D12** | digit select                 |
| **Push Button**              | one terminal        |                               **D9** | configured as `INPUT_PULLUP` |
|                              | other terminal      |                              **GND** | active LOW                   |
| **LED**                      | anode / control pin |                              **D10** | output indicator             |
|                              | cathode             | **GND** through $220\;\ohm$ resistor | recommended series resistor  |
| **DS1307 RTC module**        | **SDA**             |                               **A4** | I2C data line                |
|                              | **SCL**             |                               **A5** | I2C clock line               |
|                              | **VCC**             |                               **5V** | power                        |
|                              | **GND**             |                              **GND** | ground                       |

---

## 4. Implementation (Code Walkthrough)

### 4.0 Code and System Diagram


![image](images/RTC%20Game%20Reaction%20Time%20Flow-2026-03-13-221001.png)


``` cpp title:"reactionGame.ino"
#include "Arduino.h"
#include "Wire.h"
#include "uRTCLib.h"

const int segmentPins[] = {2, 3, 4, 5, 6, 7, 8}; 
const int digitSelectPins[] = {11, 12}; 

const int BUTTON_PIN = 9;
const int LED_PIN = 10;

uRTCLib rtc(0x68); 

// Common Anode (3461BS) - 0 light the segment
const byte digitPatterns[11] = {
  0b11000000, // 0
  0b11111001, // 1
  0b10100100, // 2
  0b10110000, // 3
  0b10011001, // 4
  0b10010010, // 5
  0b10000010, // 6
  0b11111000, // 7
  0b10000000, // 8
  0b10010000, // 9
  0b11111111  // empty
};

int currentCounter = 0;
const int targetValue = 5; 
bool isPlaying = false;
int previousRTCSecond = -1;
unsigned long lastTickTime = 0;

void setup() {
  Serial.begin(9600);
  
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
  for (int i = 0; i < 2; i++) {
    pinMode(digitSelectPins[i], OUTPUT);
    digitalWrite(digitSelectPins[i], LOW);
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  URTCLIB_WIRE.begin();

  //rtc.set(0, 0, 12, 1, 23, 2, 26); 

  Serial.println("System is Ready. Press button to START.");
}

void loop() {
  rtc.refresh();
  int currentSecond = rtc.second();

  if (!isPlaying) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      delay(200); 
      isPlaying = true;
      currentCounter = 0;
      previousRTCSecond = currentSecond;
      Serial.println("GAME STARTED! Catch the 5.");
    }
  } 
  else {
    if (currentSecond != previousRTCSecond) {
      previousRTCSecond = currentSecond;
      currentCounter++;
      if (currentCounter > 10) currentCounter = 0;
      
      lastTickTime = millis(); 
      Serial.print("Digit: "); Serial.println(currentCounter);
    }

    if (digitalRead(BUTTON_PIN) == LOW) {
      handleGameEnd();
      delay(500); 
    }
  }

  displayNumber(currentCounter);
}

void displayNumber(int totalNum) {
  int tens = totalNum / 10;
  int units = totalNum % 10;

  if (totalNum >= 10) {
    drawDigit(0, tens); 
    delay(5); 
  }

  drawDigit(1, units);
  delay(5);
}

void drawDigit(int digitIndex, int num) {
  digitalWrite(digitSelectPins[0], LOW);
  digitalWrite(digitSelectPins[1], LOW);

  byte pattern = digitPatterns[num];
  for (int i = 0; i < 7; i++) {
    bool bitVal = (pattern >> i) & 1;
    digitalWrite(segmentPins[i], bitVal ? HIGH : LOW);
  }

  digitalWrite(digitSelectPins[digitIndex], HIGH);
}

void handleGameEnd() {
  unsigned long pressTime = millis();
  long reactionTime = pressTime - lastTickTime;
  
  Serial.print("Pressed on: "); Serial.println(currentCounter);
  Serial.print("Reaction Time: "); Serial.print(reactionTime); Serial.println(" ms");

  if (currentCounter == targetValue && reactionTime <= 100) {
    Serial.println(">>> SUCCESS! <<<");
    for(int i=0; i<10; i++) {
      digitalWrite(LED_PIN, HIGH); delay(50);
      digitalWrite(LED_PIN, LOW); delay(50);
    }
  } 
  else {
    Serial.println(">>> FAILED <<<");
    digitalWrite(LED_PIN, LOW); 
  }
  
  isPlaying = false;
  currentCounter = 0;
}
```

### 4.1 Including Libraries

```cpp
#include "Arduino.h"
#include "Wire.h"
#include "uRTCLib.h"
```

- **`Arduino.h`** 
		- for standard Arduino functions, like `pinMode()`, `millis()`, `delay()`.
- **`Wire.h`**
		- to enable **I2C** communication.
- **`uRTCLib.h`**
		- contains **RTC** functions to manipulate with RTC module. It is important to download this library, which is built on the logic of I2C communication, before, otherwise the functions will be unfamiliar to IDE, multiple errors will occur.

### 4.2 Pin Configuration

```cpp
const int segmentPins[] = {2, 3, 4, 5, 6, 7, 8}; 
const int digitSelectPins[] = {11, 12}; 

const int BUTTON_PIN = 9;
const int LED_PIN = 10;
```

This one is clear, since the pin mapping table is given above.

### 4.3 RTC Object Creation

``` cpp
uRTCLib rtc(0x68); 
```

As was mentioned before, each I2C peripheral has its own address, using which, master device can reache it. For **DS1307**, default address is **`0x68`**. Since only one slave device is used and there is no address conflict, it is fine to keep the default address. 

### 4.4 Digit Pattern for Common Anode

```cpp
const byte digitPatterns[11] = {
  0b11000000, // 0
  0b11111001, // 1
  0b10100100, // 2
  0b10110000, // 3
  0b10011001, // 4
  0b10010010, // 5
  0b10000010, // 6
  0b11111000, // 7
  0b10000000, // 8
  0b10010000, // 9
  0b11111111  // empty
};
```

Following bit masking makes it much more easier to display each digit. As **3461BS** display was used, which works in common anode mode, when desired segment is needed to be lit up, on the corresponding line logic **LOW** must be applied (0). **HIGH** (1) when the goal is turn the segment off.

### 4.5 Global Variables

```cpp
int currentCounter = 0;
const int targetValue = 5; 
bool isPlaying = false;
int previousRTCSecond = -1;
unsigned long lastTickTime = 0;
```

- **`currentCounter`**
		- keeps track of the numbers being displayed.
- **`targetValue`**
		- stores constant desired winning digit.
- **`isPlaying`**
		- shows the state of the game: active/idle.
- **`previousRTCSecond`**
		- used for time tracking through RTC - **real second** passed, by storing previous value from it.
- **`lastTickTime`**
		- used for defining reaction time in milliseconds.

### 4.6 Initial Setup

```cpp
void setup() {
  Serial.begin(9600);
  
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }

  for (int i = 0; i < 2; i++) {
    pinMode(digitSelectPins[i], OUTPUT);
    digitalWrite(digitSelectPins[i], LOW);
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  URTCLIB_WIRE.begin();
  
  //rtc.set(0, 0, 12, 1, 23, 2, 26); 

  Serial.println("System is Ready. Press button to START.");
}
```

#### 4.6.1 Setting Baud Rate

- `Serial.begin(9600);`
		- Setting up standard baud rate of 9600 bits per second (bps) for Arduino serial communication -> **serial monitor**. 

#### 4.6.2 Setting Pins

```cpp
for (int i = 0; i < 7; i++) {
  pinMode(segmentPins[i], OUTPUT);
}
```

- configuring all pins corresponding to 7 segments as an output. 

```cpp
for (int i = 0; i < 2; i++) {
  pinMode(digitSelectPins[i], OUTPUT);
  digitalWrite(digitSelectPins[i], LOW);
}
```

- configuring pins corresponding to digit select lines as an output and turning them off initially.

```cpp
pinMode(BUTTON_PIN, INPUT_PULLUP);
```

- configuring button as `INPUT_PULLUP` which means that the pin is set to input and internal pull-up resistor is enabled resulting in logic **HIGH** when button is released and **LOW** when button is pressed

```
pinMode(LED_PIN, OUTPUT);
```

- configuring LED as output, which will blink when user wins.

#### 4.6.3 Starting RTC's I2C Communication & Setting RTC's Timestamp

- **`URTCLIB_WIRE.begin();`**
		- to initialize I2C so the master (Arduino) can communicate with its slave (RTC).
- **`rtc.set(0, 0, 12, 1, 23, 2, 26);`**
		- `rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year);`
		- being run only once to put the timestamp.
		- commented or deleted later. If not, RTC will reset its timestamp each time the function is called.

### 4.7 Main Loop

```cpp
void loop() {
  rtc.refresh();
  int currentSecond = rtc.second();

  if (!isPlaying) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      delay(200); 
      isPlaying = true;
      currentCounter = 0;
      previousRTCSecond = currentSecond;
      Serial.println("GAME STARTED! Catch the 5.");
    }
  } 
  else {
    if (currentSecond != previousRTCSecond) {
      previousRTCSecond = currentSecond;
      currentCounter++;
      if (currentCounter > 10) currentCounter = 0;
      
      lastTickTime = millis(); 
      Serial.print("Digit: "); Serial.println(currentCounter);
    }

    if (digitalRead(BUTTON_PIN) == LOW) {
      handleGameEnd();
      delay(500); 
    }
  }

  displayNumber(currentCounter);
}
```

#### 4.7.1 Refreshing RTC and Reading Seconds

```cpp
rtc.refresh();
int currentSecond = rtc.second();
```

- Before reading RTC values, it is better to first refresh RTC using `rtc.refresh()` to synchronize the software part of the system with the hardware's timing to avoid possible drifts.
- After that, using `rtc.second();`, current value of seconds can be fetched into the program.

#### 4.7.2 Waiting before Game Start

```cpp
if (!isPlaying) {
...
}
```

Here the programs checks if the game inactive - as far as the button is not pressed, the program waits for it, indicating **0**. In serial monitor only "*System is Ready. Press button to START.*" message is shown.

#### 4.7.3 First Button Press Detection

```cpp
if (digitalRead(BUTTON_PIN) == LOW) {
  delay(200); 
  isPlaying = true;
  currentCounter = 0;
  previousRTCSecond = currentSecond;
  Serial.println("GAME STARTED! Catch the 5.");
}
```

- When button is pressed, it send LOW logic and from that point the game starts.
- by `delay(200)`, 200-millisecond delay is used, helping to avoid false triggers due to physical imperfections of button in a form of possible dribbles resulting in additional button presses. If delay is removed, there is a high chance that the game will stop immediately after it starts.
- After first button press, the game logic is changes into active by `isPlaying = true;`
- Program counter is initialized by `currentCounter = 0;`
- `previousRTCSecond = currentSecond;` prevents immediate fallacious increment.
- `Serial.println();` prints message that the game has been started "*GAME STARTED! Catch the 5.*"

#### 4.7.4 Playing State

```cpp
if (currentSecond != previousRTCSecond) {
  previousRTCSecond = currentSecond;
  currentCounter++;
  if (currentCounter > 10) currentCounter = 0; // reset
  
  lastTickTime = millis(); // to detect when the putton was pressed
  Serial.print("Digit: "); Serial.println(currentCounter);
}
```

- With each elapsed second this loop runs, incrementing counter by 1 using `currentCounter++;` 
- When counter value hits 11, it is being updated back to 0, giving solid 0 -> 10 loop.
- `lastTickTime = millis();` stores the moment when digit has changed - used later, when the button is pressed for the second time, helping to calculate the reaction time of user by `pressTime - lastTickTime` difference.
- `Serial.print("Digit: "); Serial.println(currentCounter);` print the massages each time when digit is changed. 

#### 4.7.5 Second Button Press Detection

```cpp
if (digitalRead(BUTTON_PIN) == LOW) {
  handleGameEnd();
  delay(500); 
}
```

When button is pressed for the second time, the program exits main loop and jumps to the `handleGameEnd()` function which will decide whether the user has won or not.

#### 4.7.6 Displaying on 7-segment

```cpp
displayNumber(currentCounter);
```

this will handle displaying current number on 7-segment display.

### 4.8 displayNumber() - Handling Tens and Units

```cpp
void displayNumber(int totalNum) {
  int tens = totalNum / 10;
  int units = totalNum % 10;

  if (totalNum >= 10) {
    drawDigit(0, tens); 
    delay(5); 
  }

  drawDigit(1, units);
  delay(5);
}
```

- This function handles splitting a number into tens digit and units digit. 
- `drawDigit()` - If current number is less than 10, only 1 digit on display will be shown (D4), otherwise both D3 and D4 will be activated.

### 4.9 drawDigit() - Multiplexing

```cpp
void drawDigit(int digitIndex, int num) {
  digitalWrite(digitSelectPins[0], LOW);
  digitalWrite(digitSelectPins[1], LOW);

  byte pattern = digitPatterns[num];
  for (int i = 0; i < 7; i++) {
    bool bitVal = (pattern >> i) & 1;
    digitalWrite(segmentPins[i], bitVal ? HIGH : LOW);
  }

  digitalWrite(digitSelectPins[digitIndex], HIGH);
}
```

#### 4.9.1 Deactivating both Digits Off

```cpp
  digitalWrite(digitSelectPins[0], LOW);
  digitalWrite(digitSelectPins[1], LOW);
```

- Turning both digits off first to prevent digits mixing up during switches.

#### 4.9.2 Loading Pattern and Isolating Each bit

```cpp
byte pattern = digitPatterns[num];
for (int i = 0; i < 7; i++) {
bool bitVal = (pattern >> i) & 1;
digitalWrite(segmentPins[i], bitVal ? HIGH : LOW);
}
```

- example:   $00111111_2 \; = \; 0_{10}$,
- `i = 0`:
    - `(11000000 >> 0) & 1 = 0`
    - segment **a** gets **LOW** -> segment **a** turns **ON**
        
- `i = 1`:
    - `(11000000 >> 1) & 1 = 0`
    - segment **b** gets **LOW** -> segment **b** turns **ON**    
- ...
	
- `i = 5`:
    - corresponding bit is still `0`
    - segment **f** turns **ON**
        
- `i = 6`:
    - `(11000000 >> 6) & 1 = 1`
    - segment **g** gets **HIGH** -> segment **g** stays **OFF**

then activating appropriate digit using
```cpp
digitalWrite(digitSelectPins[digitIndex], HIGH);
```

### 4.10 handleGameEnd() - End-of-Game Logic

```cpp
void handleGameEnd() {
  unsigned long pressTime = millis();
  long reactionTime = pressTime - lastTickTime;
  
  Serial.print("Pressed on: "); Serial.println(currentCounter);
  Serial.print("Reaction Time: "); Serial.print(reactionTime); Serial.println(" ms");

  if (currentCounter == targetValue && reactionTime <= 100) {
    Serial.println(">>> SUCCESS! <<<");
    for(int i=0; i<10; i++) {
      digitalWrite(LED_PIN, HIGH); delay(50);
      digitalWrite(LED_PIN, LOW); delay(50);
    }
  } 
  else {
    Serial.println(">>> FAILED <<<");
    digitalWrite(LED_PIN, LOW); 
  }
  
  isPlaying = false;
  currentCounter = 0;
}
```

#### 4.10.1 Capturing Reaction Time

```cpp
unsigned long pressTime = millis();
long reactionTime = pressTime - lastTickTime;
```

#### 4.10.2 Printing to the Serial

```cpp
Serial.print("Pressed on: "); Serial.println(currentCounter);
Serial.print("Reaction Time: "); Serial.print(reactionTime); Serial.println(" ms");
```

For example, if pressed when `currentCounter` is 3 and `reactionTime` is 662:
```
Pressed on: 3
Reaction Time: 662 ms
```

#### 4.10.3 Winning Condition and Indication

```cpp
if (currentCounter == targetValue && reactionTime <= 100) {
Serial.println(">>> SUCCESS! <<<");
for(int i=0; i<10; i++) {
  digitalWrite(LED_PIN, HIGH); delay(50);
  digitalWrite(LED_PIN, LOW); delay(50);
}
} 
```

If `currentCounter` equals `targetValue` **AND** `reactionTime` is less than or equal 100, then the game winning condition is met and user wins. The win is indicated in serial through  "*>>> SUCCESS! <<<*" message and LED, blinking 10 times.

#### 4.10.4 Failure Condition

```cpp
else {
  Serial.println(">>> FAILED <<<");
  digitalWrite(LED_PIN, LOW); 
}
```

LED stays off, "**>>> FAILED <<<**" message is printed in serial monitor.

#### 4.10.5 Resetting the Game

```cpp
isPlaying = false;
currentCounter = 0;
```



---

## 5. Results & Evidence

The system was successfully tested and worked as expected, according to the required logic of the game.

After power-up or reset, using Arduino's reset button, the serial monitor displayed the following message in serial:
```
System is Ready. Press button to START.
```

On the 7-segment display, 0 was being indicated statically and feedback LED stayed off.

After the first button-press, this message was displayed in serial:
```
GAME STARTED! Catch the 5.
```

and the game started counting from 0 to 10, showing the following messages in serial:
```
Digit: 1
Digit: 2
Digit: 3
...
Digit: 10  
Digit: 0
Digit: 1
```

and 7-segment display started to switch segments and select lines indicating the same numbers.

After second button press, success was indicated using the similar message in serial:
```
Pressed on: 5
Reaction Time: 90 ms
>>> SUCCESS! <<<
```

and LED blinked 10 times, after which game returned to idle state, waiting for another "first-press".

During failure, the following similar message appeared:
```
Pressed on: 2
Reaction Time: 427 ms
>>> FAILED <<<
```

LED stayed off, and game returned to the waiting state.

---

## 6. Conclusion

In this lab, reaction game was successfully designed and implemented using the Arduino Uno board, DS1307 RTC module, 3461BS 7-segment 4-digit display, push button, and LED/Buzzer. The system used I2C communication to read time from the RTC, using its timing as a reference for incrementing the counter once per second.

Results showed that the game logic worked just as expected: the system stayed idle at zero, started after the first button press, counted from 0 to 10 in a loop, returning to zero, and started counting once again infinitely many times, measured the user’s reaction time, and gave correct feedback through the LED or Buzzer for success or failure.

Overall, this lab demonstrates the practical use of serial communication in the form of I2C, RTC-based timing, multiplexed display control, and simple game logic.

---
## 7. References

1. https://learn.sparkfun.com/tutorials/serial-communication/all
2. A. Farrall, Embedded Systems – Lecture 3, ADA University, 2026.
3. https://www.ti.com/lit/an/sbaa565/sbaa565.pdf?ts=1773368801397&ref_url=https%253A%252F%252Fwww.google.com%252F
