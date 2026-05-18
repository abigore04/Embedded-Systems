# Lab 7 - Contactless Communication Systems / Security System

**Course:** ENCE 3608 – Introduction to Embedded Systems  
**Instructor:** Dr. Alexz Farrall  
**Lab Number & Title:** Lab 7 - Contactless Communication Systems / Security System  
**Student Name:** Farid Ibadov  
**Student ID:** 17954  
**Program / Section:** BSCE26  
**Date:** May, 2026  

---

## 0. Table of Contents

- [1. Objectives](#1-objectives)
- [2. Theoretical Background](#2-theoretical-background)
  - [2.1 Contactless Communication](#21-contactless-communication)
  - [2.2 Infrared Communication](#22-infrared-communication)
  - [2.3 IR Code Mapping](#23-ir-code-mapping)
  - [2.4 IR Limitations in This System](#24-ir-limitations-in-this-system)
  - [2.5 RFID-RC522 Operating Principle](#25-rfid-rc522-operating-principle)
- [3. Hardware & Configuration](#3-hardware--configuration)
  - [3.1 System Components](#31-system-components)
  - [3.2 Required Libraries](#32-required-libraries)
  - [3.3 Pin Mapping](#33-pin-mapping)
  - [3.4 Keypad Configuration](#34-keypad-configuration)
  - [3.5 IR Receiver Configuration](#35-ir-receiver-configuration)
  - [3.6 RFID-RC522 Configuration](#36-rfid-rc522-configuration)
  - [3.7 LED State Configuration](#37-led-state-configuration)
  - [3.8 Serial and GUI Configuration](#38-serial-and-gui-configuration)
- [4. Implementation](#4-implementation-full-codes-in-the-github-link-below)
  - [4.1 General Program Logic](#41-general-program-logic)
  - [4.2 Setup Function](#42-setup-function)
  - [4.3 Main Loop](#43-main-loop)
  - [4.4 Keypad Handling](#44-keypad-handling)
  - [4.5 IR Handling](#45-ir-handling)
  - [4.6 RFID Handling](#46-rfid-handling)
  - [4.7 GUI and Database Handling](#47-gui-and-database-handling)
- [5. Results & Evidence](#5-results--evidence)
  - [5.1 System Startup and RFID Initialization](#51-system-startup-and-rfid-initialization)
  - [5.2 Keypad Locking Test](#52-keypad-locking-test)
  - [5.3 IR Remote Unlock Test](#53-ir-remote-unlock-test)
  - [5.4 IR Oscilloscope Evidence](#54-ir-oscilloscope-evidence)
  - [5.5 RFID Scan and GUI Database Test](#55-rfid-scan-and-gui-database-test)
  - [5.6 RFID Oscilloscope Evidence](#56-rfid-oscilloscope-evidence)
- [6. Conclusion](#6-conclusion)
- [7. References](#7-references)

## 1. Objectives

The objective of this lab was to design and implement a small security system using Arduino Uno, membrane keypad, IR remote with IR receiver, RFID-RC522 reader, two LEDs, and a supplementary PyQt6 GUI.

The system operates in three main modes: the first is waiting, the second is locked, and the third is unlocked. Initially, the user enters a four-digit code using the buttons on the membrane keypad and confirms this code using the bar `#` symbol. The system then enters a locked state, while the RFID reader is inactive and does not respond to tags in its scanning field. To unlock the system, the same four-digit code must be entered again, this time using the infrared remote control. If the codes match, the system enters unlocked mode, where RFID scanning becomes active.

Two LEDs were used as simple visual indicators of the system status: in the waiting state, both LEDs flash; in the locked state, the red LED remains on; and in the unlocked state, the green LED is on. When the correct unlock code is dialed, or a valid RFID signal is received, both LEDs flash rapidly to indicate a successful action.

It's worth noting that before writing the final code for the system, a separate code was written and utilized to determine the correct hex values ​​transferred from the IR control, namely buttons 0 through 9. This was necessary because different remote controls can send different hex codes for the same values, so they had to be determined first and then transferred to the main program.

Additionally, the final system includes a PC-side GUI. When RFID tags are scanned in the unlocked state, Arduino sends the tag UID through serial communication in the format `TAG,UID`. The PyQt6 app gets this line, stores tag in SQLite database, assigns unique ID to each new tag, and increments the scan count if the same tag is scanned again.

---

## 2. Theoretical Background

### 2.1 Contactless Communication

Unlike to what was done in previous labs, where the communication was done using physical wires, like in such protocols as UART, SPI, I2C, this lab implements contactless communication it the form of IR and RFID signal transmissions. Here, signals are transferred without direct electrical connection among transmitting and receiving devices. 

This type of communication is essential in systems where being mobile and secure are key requirements, like in access control systems, remote controls, payment cards and etc. In this lab, as mentioned earlier, two contactless technologies were utilized:

- **IR communication** - remote control buttons are transmitted using infrared light.
- **RFID communication** - a passive tag is powered and identified by the electromagnetic field of the reader.

In a simplified form and more clear way, the whole system can be represented like this:

![image](images/mermaid-diagram.png)
### 2.2 Infrared Communication

![image](images/Pasted%20image%2020260518032021.png)

Infrared light is an electromagnetic radiation with wavelength longer than visible light. IR's wave length roughly has the following range:

$$\lambda \approx 700\,nm \;to\; 1\,mm$$

The relation between wavelength and frequency is inversely proportional:

![image](images/Pasted%20image%2020260518032336.png)

$$c = \lambda \cdot f$$

where:

- $c = 3 \times 10^8\,m/s$ - speed of light,
- $\lambda$ - wavelength,
- $f$ - frequency.

IR communication employs an IR LED in the remote control (light of which is invisible to human eye, but can be viewed using regular camera)

![image](images/Pasted%20image%2020260518033509.png)

and an IR receiver module on the Arduino side. 

![image](images/Pasted%20image%2020260518033538.png)

The remote does not send seamless stream of light. It sends only short bursts of IR light modulated around a carrier frequency, usually close to:

$$f_c = 38\,kHz$$

This frequency was chosen deliberately, so that external factors, such as UV-light from the sun or other sources, could interfere minimally with the signal. This is the specification of NEC IR remote protocol which is a standard for remotes, widely used in consumer electronics. NEC frame consists of 32 bits, equally divided into 4 main parts:

- **Address** - to identify the specific device/brand.
- **Logical Inverse Address** - exact opposite of address, error checking.
- **Command** - this is the *hex code* we are interested in!
- **Logical Inverse Command** - additional error-check measure.

![image](images/Pasted%20image%2020260518034901.png)

The corresponding carrier period is:

$$T_c = \frac{1}{f_c} = \frac{1}{38000} \approx 26.3\,\mu s$$

IR receiver module is designed to detect this kind of modulated signal. Important to note that Arduino does not directly receive every $26.3\,\mu s$ carrier pulse. IR receiver module demodulates the signal using its internal circuitry and outputs a cleaner digital waveform, which represents the transmitted code.

![image](images/Pasted%20image%2020260518034057.png)

### 2.3 IR Code Mapping

Since different remotes can send different hex values for the same command, a separate testing sketch was used to detect actual command values of the remote used in this lab

The sketch outputted the detected protocol, address, command and raw data. The remote used in this lab was decoded as NEC protocol, discussed before, with 32 bits, LSB first.

The final detected values were:

| Button | Command HEX | RawData HEX  |
| :----: | :---------: | :----------: |
|  `0`   |   `0x16`    | `0xE916FF00` |
|  `1`   |   `0x0C`    | `0xF30CFF00` |
|  `2`   |   `0x18`    | `0xE718FF00` |
|  `3`   |   `0x5E`    | `0xA15EFF00` |
|  `4`   |   `0x08`    | `0xF708FF00` |
|  `5`   |   `0x1C`    | `0xE31CFF00` |
|  `6`   |   `0x5A`    | `0xA55AFF00` |
|  `7`   |   `0x42`    | `0xBD42FF00` |
|  `8`   |   `0x52`    | `0xAD52FF00` |
|  `9`   |   `0x4A`    | `0xB54AFF00` |


After this, the main program directly compared received IR commands with this mapping.

As a useful functionality, `IRremote` library reports repeat frames when a button is held. In this system, repeat frames are ignored, since one physical button press should directly correspond to only one digit entered into the unlock code.

### 2.4 IR Limitations in This System

IR communication is simple, cheap, and reliable. However, it has some limitations that should be acknowledged.

Most importantly, it requires **line of sight**. If the remote is not pointed directly towards the receiver or if it is blocked by an object, the command may not be reached or decoded correctly.

Another important weakness of IR communication is the intensity of the signal, which decays very quickly with distance. The received intensity approximately follows inverse-square behavior:

$$E_2 = E_1 \cdot \left(\frac{d_1}{d_2}\right)^2$$

So, if the distance is doubled, the IR energy gets reduced 4 times:

$$E_2 = E_1 \cdot \left(\frac{1}{2}\right)^2 = \frac{E_1}{4}$$

That is why IR remote range and angle matter - even if remote controller is working, moving it farther or pointing it in wrong direction can make the receiver miss the signal.

Lastly, IR can be affected by strong ambient light or reflections.

Even though it is not suitable for hidden or long-range authentication, IR is highly practical for short-range control.

### 2.5 RFID-RC522 Operating Principle

Radio Frequency Identification (RFID) pair consists of a reader and a tag, that can be either passive or active. Passive tag has no internal power source, while active has it. In this lab, RC522 module was used as a reader, and as the tags were passive RFID cards/tags 

![image](images/Pasted%20image%2020260518042016.png)

The RC522 operates at:

$$f = 13.56\,MHz$$

The reader's coil has the alternating current flowing inside which creates changing magnetic field around it. Whenever passive tag intrudes in this field, magnetic flux going through the coil of the tag changes and, according to Faraday Law, this changing magnetic flux induces some voltage:


$$\varepsilon = -N \cdot \frac{d\Phi}{dt}$$

- $\varepsilon$ - induced EMF,
- $N$ - number of turns in the tag coil,
- $\Phi$ - magnetic flux.

Magnetic flux depends on field strength ($B$), coil area ($A$) and orientation( $cos(\theta)$ ):

$$\Phi = B \cdot A \cdot \cos(\theta)$$

This formula answers to the question why tag distance and angle matter. If the tag is angled badly, $\cos(\theta)$ becomes smaller and, as a result, less flux passes through tag coil, and tag ends up with the probability of not receiving enough energy to power the microchip embedded in it.

Since the RC522 field is alternating, the flux is also changing with time. For a sinusoidal field:

$$\Phi(t)=B_{peak}\cdot A \cdot \cos(2\pi ft)$$

The maximum induced EMF becomes:

$$|\varepsilon|_{max}=N \cdot B_{peak}\cdot A \cdot 2\pi f$$

This makes clear why a passive tag can work without a battery. The reader’s high-frequency field induces enough voltage in the tag coil to power its small chip. 

The tag does not create its own magnetic field, since it is way too energy consuming. The tag responds to the reader by slightly changing the load on its antenna. The reader detects this change as a modulated response. This is the backscatter or load modulation principle.

![image](images/Pasted%20image%2020260518043644.png)

---

## 3. Hardware & Configuration

### 3.1 System Components

The main components used in the system were:

- Arduino Uno
- 4x4 membrane keypad
- IR receiver module
- IR remote controller
- RFID-RC522 reader
- passive RFID tags/cards
- 2 LEDs
- resistors for LEDs
- computer, running PyQt6 GUI


The system can be separated into 3 main functional layers:

![image](images/mermaid-diagram%20%281%29.png)

Membrane keypad - for setting the lock code, IR remote - for unlocking, and RC522 reader - after the system enters the unlocked state.

### 3.2 Required Libraries

Before running the final firmware, appropriate Arduino libraries should to be installed. Since project was written in PlatformIO, those dependencies were added in `platformio.ini` file as:

```ini
lib_deps = 
    miguelbalboa/MFRC522@^1.4.12
    chris--a/Keypad@^3.1.1
    z3t0/IRremote@^4.7.1
```

In the Arduino code, they were included as:

```c
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <IRremote.hpp>
```

- `SPI.h` - for RC522 communicating over the SPI bus
- `MFRC522.h` - for initializing the RFID reader and reading tag UIDs
- `Keypad.h` - for simplified scanning of the membrane keypad matrix
- `IRremote.hpp` - for decoding IR remote signals and extracting command hex values

### 3.3 Pin Mapping

![image](images/Pasted%20image%2020260518053445.png)

![image](images/Pasted%20image%2020260518053541.png)

| Component | Pin / Terminal | Arduino Connection | Purpose |
|---|---|---|---|
| IR Receiver | OUT / S | **D2** | receives IR command waveform |
| IR Receiver | VCC | **5V** | module power |
| IR Receiver | GND | **GND** | common ground |
| RC522 | SDA / SS | **D10** | SPI chip select |
| RC522 | SCK | **D13** | SPI clock |
| RC522 | MOSI | **D11** | Arduino to RC522 data |
| RC522 | MISO | **D12** | RC522 to Arduino data |
| RC522 | RST | **D9** | reset line |
| RC522 | 3.3V | **3.3V** | RFID reader power |
| RC522 | GND | **GND** | common ground |
| Keypad Row 1 | R1 | **D3** | keypad row scan |
| Keypad Row 2 | R2 | **D4** | keypad row scan |
| Keypad Row 3 | R3 | **D5** | keypad row scan |
| Keypad Row 4 | R4 | **D6** | keypad row scan |
| Keypad Column 1 | C1 | **D7** | keypad column scan |
| Keypad Column 2 | C2 | **D8** | keypad column scan |
| Keypad Column 3 | C3 | **A0** | keypad column scan |
| Keypad Column 4 | C4 | **A1** | keypad column scan |
| Green LED | anode | **A2** | unlocked indicator |
| Red LED | anode | **A3** | locked indicator |
| LEDs | cathode | **GND through resistor** | current-limited return path |
| Arduino USB | TX/RX over USB | **PC GUI** | serial data transfer |

Important to note that the RC522 must be powered from 3.3V, since higher voltage can damage the module's components.
### 3.4 Keypad Configuration

Membrane keypad is a 4x4 matrix. Due to matrix design, it uses 4 row pins and 4 column pins, reducing number of required pins double times, if every button had to be connected separately.

![image](images/Pasted%20image%2020260518050944.png)

The program scans this matrix and determines which key was pressed by checking row-column combinations.

By checking row-column combinations, the key pressed is determined easily. The keypad layout in the code was:

```c
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
```

Only numerical keys (9-0), and `*` with `#` are used for this lab. 
### 3.5 IR Receiver Configuration

The IR receiver output was connected to **D2**. Before writing the final code, the separate `IRTester.ino` sketch was used to find the correct command values for buttons `0` to `9`.


After running the testing sketch to determine correct command values for remote buttons of the remote, these values were defined as constants:

```c
#define IR_KEY_0 0x16
#define IR_KEY_1 0x0C
#define IR_KEY_2 0x18
#define IR_KEY_3 0x5E
#define IR_KEY_4 0x08
#define IR_KEY_5 0x1C
#define IR_KEY_6 0x5A
#define IR_KEY_7 0x42
#define IR_KEY_8 0x52
#define IR_KEY_9 0x4A
```

One crucial configuration detail was that IR library LED feedback was disabled, since, normally, `IRremote` library can blink the onboard LED on pin **D13** when a signal is received, and this can conflict with more important functionality - **D13** is dedicated for the SPI's SCK, which is used by the RC522. This conflict may disrupt the program flow and because of that LED feedback had to be disabled:

```c
#define DISABLE_LED_FEEDBACK false
IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
```

### 3.6 RFID-RC522 Configuration

![image](images/Pasted%20image%2020260518052420.png)

RC522 communicates with Arduino through SPI protocol. The following two pins defined for that:

```c
#define RFID_SS_PIN   10
#define RFID_RST_PIN  9
```

Rest of the SPI pins are fixed by Arduino Uno hardware:

![image](images/Pasted%20image%2020260518052339.png)

| SPI Signal | Arduino Uno Pin |
|---|---|
| `MOSI` | **D11** |
| `MISO` | **D12** |
| `SCK` | **D13** |
| `SS/SDA` | **D10** |

RFID reader object was created in the code like this:

```c
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
```

During initialization, the program reads the RC522 version register:

```c
byte version = rfid.PCD_ReadRegister(rfid.VersionReg);
```

This is a useful practice since if the returned value is `0x00`/`0xFF`, communication with the module most likely failed, in which case, the system marks RFID as not ready, retrying initialization later.

Also note that RC522's `IRQ` pin was not used as the project handled RFID scanning by periodically polling the reader in code. This is simpler and sufficient for low-speed tag logging system.

### 3.7 LED State Configuration

Two LEDs were used as the system state indicators:

| System State                  | LED Behavior                 |
| ----------------------------- | ---------------------------- |
| `WAITING_FOR_CODE`            | both LEDs blink every 500 ms |
| `LOCKED`                      | red LED ON, green LED OFF    |
| `UNLOCKED`                    | green LED ON, red LED OFF    |
| successful unlock / RFID scan | both LEDs flash shortly      |
| wrong IR code                 | red LED flashes              |

### 3.8 Serial and GUI Configuration

Arduino communicates with the PyQt6 GUI over USB serial at Baud Rate of 9600.

Arduino sends short text messages, like:

```text
STATE,LOCKED
STATE,UNLOCKED
TAG,XXXXXXXX
ERROR,RFID_NOT_RESPONDING
```

The GUI reads incoming serial lines using `QThread`, to avoid interface freezing while waiting for data. When line starts with `TAG,`, the GUI extracts the UID and saves it into `rfid_tags.db` database.

The database table stores:

| Field | Purpose |
|---|---|
| `id` | automatically assigned unique ID |
| `uid` | RFID tag UID |
| `scan_count` | number of times the tag was scanned |
| `first_seen` | timestamp of first scan |
| `last_seen` | timestamp of latest scan |

If the scanned UID is new for the system, GUI creates new database row. However, if UID already exists - increments `scan_count` and updates `last_seen`.

---

## 4. Implementation (full codes in the GitHub link below)

### 4.1 General Program Logic

As always, the implementation consists of two parts: Arduino firmware and PyQt6 GUI. 

Arduino is responsible for handling the security logic, like: keypad input, IR unlock, RFID scan, LED indication, serial messaging. GUI only listens for RFID tag messages coming from the Arduino and stores those in the database. 

General flow is:

![image](images/mermaid-diagram%20%282%29.png)

Program uses 3 main states:

```c
enum SystemState {
  WAITING_FOR_CODE,
  LOCKED,
  UNLOCKED
};
```

### 4.2 Setup Function

`setup()` function prepares serial communication, LEDs, RFID reader, IR receiver, and initial state.

```c
void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  Serial.println(F("LAB7_SECURITY_SYSTEM_START"));

  initRFID();

  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);

  changeState(WAITING_FOR_CODE);
}
```

`Serial.begin(9600)` establishes communication between Arduino and the PC-side GUI. This baud rate is sufficient as Arduino only sends short text messages like:
- `STATE,LOCKED`
- `STATE,UNLOCKED`
- `TAG,UID`

`initRFID()` initializes RC522 module through SPI. IR receiver is started on pin **D2** and LED feedback of the IR library is disabled.

### 4.3 Main Loop

The `loop()` function has been made deliberately short, making the code more readable, as each hardware part has its own function where it is handled correspondingly.

```c
void loop() {
  updateWaitingLEDs();

  handleKeypad();
  handleIR();
  handleRFID();
}
```


- `updateWaitingLEDs()` - blinks both LEDs only in waiting state
- `handleKeypad()` - checks keypad input for setting the lock code
- `handleIR()` - checks IR remote digits for unlocking
- `handleRFID()` - checks RFID tags only when the system is unlocked

### 4.4 Keypad Handling

The keypad is read using:

```c
char key = keypad.getKey();
```

If no key was pressed, `getKey()` returns nothing useful, so the function just exits. If a digit is pressed, it is placed into the `keypadBuffer`. The variable `keypadIndex` tracks how many digits were already entered.

The `#` key confirms the code:

```c
if (key == '#') {
  if (keypadIndex == CODE_LENGTH) {
    strcpy(lockCode, keypadBuffer);
    changeState(LOCKED);
  } else {
    Serial.println(F("ERROR,Need 4 digits before #"));
  }
}
```

The system accepts the keypad code only if exactly 4 digits were entered. The `*` key clears the current input, if typo was made. Once the system becomes locked, the keypad is no longer used for unlocking, passing this responsibility to IR remote controller.
### 4.5 IR Handling

Function for the IR remote to unlock the system first checks whether the receiver decoded anything:

```c
if (!IrReceiver.decode()) {
  return;
}
```

Repeat frames are ignored:

```c
if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
  IrReceiver.resume();
  return;
}
```

Since holding a remote button can generate repeat frames, in this system, one button press corresponds to one digit only.

After, command byte is extracted:

```c
uint8_t command = IrReceiver.decodedIRData.command;
int digit = irCommandToDigit(command);
```

The function `irCommandToDigit()` compares the received command with the hex values determined earlier. For instance, button `0` corresponds to `0x16`, button `1` - to `0x0C`, and so on.

After 4 IR digits are entered, program compares the IR input with the saved keypad code:

```c
if (strcmp(irBuffer, lockCode) == 0) {
  Serial.println(F("RESULT,IR_CODE_CORRECT"));
  changeState(UNLOCKED);
  flashSuccess();
} else {
  Serial.println(F("RESULT,IR_CODE_WRONG"));
  clearIrBuffer();
  flashWrongCode();
}
```

### 4.6 RFID Handling

RFID is processed only in the `UNLOCKED` state and is disabled when system is locked until correct IR code unlocks it:

```c
if (currentState != UNLOCKED) {
  return;
}
```

If RFID initialization failed, code retries initialization later - is useful as RC522 modules may sometimes fail to respond immediately after reset or unstable wiring.

Tag detection is done in 2 distinct steps:

```c
if (!rfid.PICC_IsNewCardPresent()) {
  return;
}

if (!rfid.PICC_ReadCardSerial()) {
  Serial.println(F("ERROR,RFID_READ_FAIL"));
  return;
}
```

- `PICC_IsNewCardPresent()` checks whether a new tag is near the reader.
- `PICC_ReadCardSerial()` reads the tag UID and if reading is successful, Arduino sends the UID to the GUI in this format:

```text
TAG,UID
```

After reading a tag, the code halts communication with that card:

```c
rfid.PICC_HaltA();
rfid.PCD_StopCrypto1();
```

This is a good practice after each RFID transaction, because it properly ends the card communication session.

### 4.7 GUI and Database Handling

GUI allows user to select a serial port, connect to Arduino, view incoming serial messages, and see the database table. It uses separate serial thread:

```python
class SerialThread(QThread):
    line_received = pyqtSignal(str)
```

This is important because reading from serial port can block the program. If serial reading was done directly in main GUI thread, interface could freeze. Using `QThread`, GUI remains responsive while continuously listening to Arduino.

When Arduino sends a line beginning with `TAG,`, GUI extracts UID:

```python
if line.startswith("TAG,"):
    uid = line.split(",", 1)[1].strip().upper()
```

Then function `save_tag(uid)` stores it in `rfid_tags.db`.

The database table stores:

| Field | Meaning |
|---|---|
| `id` | automatically assigned database ID |
| `uid` | RFID tag UID |
| `scan_count` | number of times the tag was scanned |
| `first_seen` | timestamp of first scan |
| `last_seen` | timestamp of latest scan |

---

## 5. Results & Evidence

### 5.1 System Startup and RFID Initialization

After uploading code, serial first confirmed that the firmware started and RC522 module responded to Arduino:

![image](images/Pasted%20image%2020260518060831.png)

So the program started correctly and SPI communication with RC522 was working. System entered the initial waiting state.

### 5.2 Keypad Locking Test

In the waiting state, both LEDs blinked. After entering 4 digits on the keypad and pressing `#`, Arduino saved the code and changed system state to locked.

![image](images/Pasted%20image%2020260518060936.png)

At that moment, red LED stayed ON and green LED stayed OFF. RFID was logically disabled.
### 5.3 IR Remote Unlock Test

IR remote was tested after the system entered locked state. When the same 4-digit code was entered using the remote, Arduino printed each received IR command and counted the digits.

![image](images/Pasted%20image%2020260518061130.png)

After the correct IR code, both LEDs flashed shortly, then the green LED stayed on. This confirmed that the system moved from locked to unlocked state.

For a wrong code, the following output was displayed:
```cpp
...
RESULT,IR_CODE_WRONG
```
and the red LED flashed.

### 5.4 IR Oscilloscope Evidence

While pressing a remote button, IR receiver's output was captured using oscilloscope. The waveform showed the demodulated digital output of the receiver, not the raw 38 kHz carrier.

![image](images/Pasted%20image%2020260518061501.png)

In the photo above, first long pulse corresponds to NEC header burst. Following after it shorter pulses and spaces represent data bits. This proves that IR receiver demodulates optical remote signal into digital waveform readable by Arduino on pin D2.

### 5.5 RFID Scan and GUI Database Test

After unlocking the system, RFID scanning became active. When a tag was presented to the reader, Arduino sent the UID to the GUI.

GUI received the line, inserted the UID into `rfid_tags.db`, and displayed it in the table. If the same tag was scanned again, GUI did not create a duplicate row. Instead, it incremented `scan_count` and updated `last_seen`.

![image](images/Pasted%20image%2020260518061807.png)

### 5.6 RFID Oscilloscope Evidence

RC522 antenna signal was captured from the `TX1` / `TX2` antenna-side point using oscilloscope.

Without card brought close:

![image](images/Pasted%20image%2020260518062512.png)

Card introduced:

![image](images/Pasted%20image%2020260518062538.png)

In both photos, signal appeared as dense wide band - not a clearly separated sine wave. This is expected behavior since the RC522 antenna operates at high frequencies, around $13.56\,MHz$, and oscilloscope view compresses many fast oscillations into one visible band.

Most important observation here is the difference in the vertical width of the band. Without a card near the reader, the band appeared slightly wider. When the card was placed near the antenna, the band became narrower - the indication that passive tag affected the reader's antenna field by coupling to it and drawing energy from it. As it was noted before, the card do not generate its own signal, but changed (modulates) the loading of the RC522 antenna, which demonstrates the RFID load modulation/backscatter principle.

---

## 6. Conclusion

In this lab, security system was implemented using keypad-based locking, IR remote unlocking, RFID tag reading, LED state indication, and PyQt6 database GUI. The system showed how contactless communication can be integrated with state-based embedded logic, where RFID scanning is only accepted after the correct IR unlock code. Oscilloscope captures also helped to verify physical behavior of both IR receiver output and RC522 antenna activity. Lab combined hardware input, wireless/contactless sensing, serial communication, and persistent tag logging into one complete security pipeline.

---

## 7. References

1. Lecture 7 - Contactless Communication Systems, ENCE 3608, Dr. Alexz Farrall.
2. Arduino IRremote Library Documentation: https://github.com/Arduino-IRremote/Arduino-IRremote
3. MFRC522 Arduino Library Documentation: https://github.com/miguelbalboa/rfid
4. Keypad Arduino Library Documentation: https://github.com/Chris--A/Keypad

