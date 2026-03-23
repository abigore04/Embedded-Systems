**Course:** ENCE 3608 – Introduction to Embedded Systems  
**Instructor:** Dr. Alexz Farrall  
**Lab Number & Title:**  Lab 4 - Joystick Visualization 
**Student Name:** Farid Ibadov  
**Student ID:** 17954  
**Program / Section:** BSCE26  
**Date:** March, 2026

---
## 1. Objectives

The objectives of this lab was to reimplement the Lab task 2 - analog joystick reading with LED feedback, with the addition of Graphical User Interface (GUI) feature, allowing to monitor data by visualizing the exact position of the joystick in real time.

Additionally, instead of programming the system through Arduino IDE, this time **Visual Studio Code** was used with **PlatformIO** extension, which significantly simplifies development, compilation, and debugging, giving much more flexibility.

---
## 2. Theoretical Background

Since most of the concepts regarding hardware peripheral were explained in the Lab Repot 2, in this report, the main focus will be only on newly introduced concepts: serial communication through UART and Real-Time Data Visualization.

### 2.1 Serial Communication (UART)

In order to transfer data from Arduino into the computer, UART (Universal Asynchronous Receiver-Trasmitter) communication protocol has been used.

UART uses single communication channel to transmit the data bit-by-bit. As the name suggest, it is asynchronous, meaning, it does not depend on a clock, instead, both devices shout agree on using the same **baud rate**. This is the key idea, when talking about the UART. 

![image](images/Pasted%20image%2020260315193449.png)

In this lab, baud rate of 115200 was specified using `Serial.begin(115200);`
which initializes the serial interface to transfer **115200 bits per second**. Increasing or decreasing baud rate may have its pros and cons. One advantage of using baud rate higher that the default of 9600 is the increased speed of communication, which is a key for a smooth visualization. However, increasing baud rate too much may result in an increased rate of corrupted data. Below, in the table, all advantages and disadvantages of using low or high baud rate is specified:

| *Feature*       | *High Baud Rate*                     | *Low Baud Rate*                     |
| --------------- | ------------------------------------ | ----------------------------------- |
| **Speed**       | High throughput, fast data transfer. | Low throughput, slow data transfer. |
| **Latency**     | Low; minimal delay.                  | High; noticeable delay.             |
| **Reliability** | Prone to noise and bit errors.       | Highly stable; resistant to noise.  |
| **Distance**    | Limited to short cable runs.         | Supports longer cable lengths.      |
| **Power**       | Higher consumption (more switching). | Lower consumption.                  |
| **Precision**   | Requires very accurate clocking.     | Forgiving of timing mismatches.     |

### 2.2 Adding and Setting PlatformIO

In previous labs, the firmware was developed using Arduino IDE, however, for this lab VS Code along with PlatformIO extension were used. Those provide better development environment is a sense of greater flexibility.

PlatformIO can be added to VS Code after typing "PlatformIO IDE" in the search bar in extensions tab:

![image](images/Pasted%20image%2020260315195041.png)

After downloading and enabling it, by clicking on PlatformIO icon in the sidebar with quick-access tools, PIO home opens
![image](images/Pasted%20image%2020260315200158.png)  

here, by clicking on the New Project, the following window appears, where the name of the project, board and location can be specified:
![image](images/Pasted%20image%2020260315200247.png)

After that there are three things to do
1. in **platformio.ini** platform, board, framework, baud rate and port should be specified.
	- to identify to which COM port Arduino board is connected, on Windows, press **Win + R**, write `devmgmt.msc`, select "Ports (COM & LPT)" and there it will appear. 
2. Arduino `C++` code should be added in `src` folder.
3. Python visualization code should be added in a manually created `gui` folder. 

![image](images/Pasted%20image%2020260315200351.png)

### 2.3 PyQt6 Graphical interface

To visualize the data using GUI, python's PyQt6 library can be used. To install it, in terminal write `pip install PyQt6` and wait till it is done.

PyQt6 represents reach framework allowing to create cross-platform graphical application by providing the number of widgets.

Particularly for this lab, Pyqt6 was used to create the GUI performing following tasks:
- Establishing *Serial communication* with Arduino.
- Reading *incoming joystick data* at real time without interrupts.
- *Collecting values* from serial.
- *Displaying coordinates* where joystick points through X and Y positions. 

The main goal of implementing the graphical visualization - provide representative feedback for actions happening in real life, with precise data manipulation.   

### 2.4 Sampling

One of the most crucial requirements of the real-time monitoring systems is to provide stable data. Using particular sampling rate, which describes how frequently data samples are produced and transmitted, the system can provide responsive data without overloading the communication channel.

For instance, when tiny delay of 10 ms is introduced in the main loop,  sampling rate will correspond to
$$Sampling\,Rate \approx \frac{1}{0.01\,s} = 100\,Hz$$
meaning that the system is able to provide up to 100 updates per second. This is sufficient for smooth and seamless visualization. 

---

## 3. Hardware & Configuration

Since this lab is a continuation of Lab 2, hardware configuration is the same.  

---

## 4. Implementation (Code Walkthrough)

### 4.1 `main.cpp` - Arduino's Firmware

```cpp title:"main.cpp"
#include <Arduino.h>

const int JOY_X = A0;
const int JOY_Y = A1;
const int ledPins[4] = {8, 9, 10, 11}; // UP, DOWN, LEFT, RIGHT
const char* names[5] = {"NEUTRAL", "UP", "DOWN", "LEFT", "RIGHT"};

const int CENTER = 512;
const int DEADZONE = 60;

void setLeds(int dir) {
  // turn off all LED first, then turn on the one corresponding to the direction
  for (int i = 0; i < 4; i++) digitalWrite(ledPins[i], LOW);
  if (dir != 0) digitalWrite(ledPins[dir - 1], HIGH);
}

void setup() {
  // baud rate 115200 for faster data transfer to Python GUI,
  // can be reduced when higher stability is preferred
  Serial.begin(115200); 
  for (int i = 0; i < 4; i++) pinMode(ledPins[i], OUTPUT);
  setLeds(0);
}

void loop() {
  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);

  int low  = CENTER - DEADZONE;
  int high = CENTER + DEADZONE;
  int dir = 0; 

  if      (x < low)  dir = 3;   // LEFT
  else if (x > high) dir = 4;   // RIGHT
  else if (y < low)  dir = 2;   // DOWN
  else if (y > high) dir = 1;   // UP

  setLeds(dir);

  // serial output format: x,y,direction
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  Serial.println(names[dir]);

  delay(10); // Sampling rate ~100Hz
}
```

Since this lab is a continuation of Lab 2 main Arduino code is almost similar, with tiny but crucial changes, such as:
- Enhanced serial data format.
- Elevated baud rate.
- Continuous data streaming for visualization.
- Increased sampling rate.
Therefore, in this lab, only those changes will be explained.

#### 4.1.1 Increased Baud Rate

```cpp
Serial.begin(115200);
```

Baud rate was increased from 9600 to 115200 to allow faster transmission of data - essential for real-time visualization.

#### 4.1.2 Continuous Data Update

In lab 2, the following condition was held:

```cpp
if (dir != lastDir)
```

this allowed data to be displayed only when direction of joystick changed. It was preferable when the main goal was just to light LEDs in corresponding direction and do not flood the serial monitor with repetitive data; but in this lab, the main objective is visual representation, therefore consistent data flow is one of the main requirements.

#### 4.1.3 Continuous Data Transmission

```cpp
Serial.print(x);
Serial.print(",");
Serial.print(y);
Serial.print(",");
Serial.println(names[dir]);
```

In lab 2, the data was sent to the serial monitor only when the direction change occurred; however, in this lab, data is being sent continuously to make sure that the displayed graphics represents the actual up-to-date information with the following format:

```cpp
x,y,direction
```

 This is convenient structure that allows easy parsing in python, where data is extracted using "split by comma" `.split(",")` and then is manipulated accordingly.

#### 4.1.4 Sampling Rate Adjustment

```cpp
delay(10);
```

Was reduced from 20 to 10 ms, to ensure smooth data transfer.

#### 4.1.5 Lab 2 vs Lab 4

With those changes, the system behaves as a real-time data acquisition system. Below, the table displays the main differences between Lab 2 and Lab 4 codes:

|Feature|Lab 2|Lab 4|
|---|---|---|
|Baud Rate|9600|115200|
|Data Transmission|On change only|Continuous|
|Output Format|Direction only|`x,y,direction`|
|Sampling Delay|20 ms|10 ms|
|Purpose|Basic control|Real-time visualization|

### 4.2 `app.py` Python Visualization Program

```python title:"app.py"
import sys
import serial
import time

from PyQt6.QtWidgets import QApplication, QWidget, QVBoxLayout, QLabel, QPushButton
from PyQt6.QtCore import QTimer, Qt
from PyQt6.QtGui import QPainter, QColor


class JoystickCanvas(QWidget):
    def __init__(self, gui):
        super().__init__()
        self.gui = gui
        self.setMinimumHeight(300)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

        w = self.width()
        h = self.height()

        painter.setPen(QColor(150, 150, 150))
        painter.drawRect(0, 0, w - 1, h - 1)

        painter.setPen(QColor(220, 220, 220))
        painter.drawLine(w // 2, 0, w // 2, h)
        painter.drawLine(0, h // 2, w, h // 2)

        x = int((self.gui.jx / 1023) * w)
        y = int((1 - self.gui.jy / 1023) * h)

        painter.setBrush(QColor(0, 122, 255))
        painter.setPen(Qt.PenStyle.NoPen)
        painter.drawEllipse(x - 10, y - 10, 20, 20)


class JoystickGUI(QWidget):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("Lab 4: Joystick Test")
        self.resize(400, 500)

        self.jx = 512
        self.jy = 512
        self.direction = "NEUTRAL"
        self.running = True
        self.samples = 0
        self.last_time = time.time()

        layout = QVBoxLayout()

        self.status_label = QLabel("Data: ON")
        self.x_label = QLabel("X: 512")
        self.y_label = QLabel("Y: 512")
        self.dir_label = QLabel("Direction: NEUTRAL")
        self.rate_label = QLabel("Sample Rate: 0 Hz")

        self.canvas = JoystickCanvas(self)
        self.toggle_button = QPushButton("Stop Test")
        self.toggle_button.clicked.connect(self.toggle_test)

        layout.addWidget(self.status_label)
        layout.addWidget(self.canvas)
        layout.addWidget(self.x_label)
        layout.addWidget(self.y_label)
        layout.addWidget(self.dir_label)
        layout.addWidget(self.rate_label)
        layout.addWidget(self.toggle_button)

        self.setLayout(layout)

        try:
            self.ser = serial.Serial("COM24", 115200, timeout=0.01)
            self.status_label.setText("Data: ON")
        except Exception as e:
            self.ser = None
            self.status_label.setText(f"Data: ERROR ({e})")

        self.timer = QTimer()
        self.timer.timeout.connect(self.read_serial)
        self.timer.start(10)

    def toggle_test(self):
        self.running = not self.running

        if self.running:
            self.status_label.setText("Data: ON")
            self.toggle_button.setText("Stop Test")
        else:
            self.status_label.setText("Data: PAUSED")
            self.toggle_button.setText("Start Test")

    def read_serial(self):
        if not self.running:
            return

        if not self.ser:
            return

        if self.ser.in_waiting <= 0:
            return

        try:
            line = self.ser.readline().decode("utf-8").strip()

            if not line:
                return

            parts = line.split(",")

            if len(parts) != 3:
                return

            self.jx = int(parts[0])
            self.jy = int(parts[1])
            self.direction = parts[2]

            self.x_label.setText(f"X: {self.jx}")
            self.y_label.setText(f"Y: {self.jy}")
            self.dir_label.setText(f"Direction: {self.direction}")

            self.samples += 1

            now = time.time()
            if now - self.last_time >= 1.0:
                self.rate_label.setText(f"Sample Rate: {self.samples} Hz")
                self.samples = 0
                self.last_time = now

            self.canvas.update()

        except:
            pass


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = JoystickGUI()
    window.show()
    sys.exit(app.exec())
```

#### 4.2.1 GUI Structure
 
 The application was done using `JoystickGUI` class inheriting from `Qwidget`. The interface of the window consists of:
 - *Status* Indicator
 - Real-time numerical data of:
	 - *X value* - horizontal
	 - *Y value* - vertical
	 - *Direction*
- *Sample rate* indicator
- *Start/Stop* control button

![image](images/Pasted%20image%2020260318152021.png)

For organization of the layout `QVBoxLayout()` was used, which stacks all elements vertically.

#### 4.2.2 Serial Communication

Serial communication with Arduino was achieved using the following function:
```python
self.ser = serial.Serial("COM24", 115200, timeout=0.01)
```

Very important to make sure that the baud rate is consistent with the one configured in Arduino firmware, otherwise asynchronous communication of UART may fail due to several reasons:
- **Framing/Data Errors**: receiver will interpret the transmitter's bits as different values since it is looking at the line at the wrong moment.
- **Garbage Data**: receiving device receives random characters instead of the intended message.
- **No Communication**: if the difference is significant, the receiver will not detect the start bit correctly, causing the frame to be ignored entirely.

With slight difference between baud rates (less than 2-3%) some tolerance may still exist; however, system will become extremely unreliable, which is unacceptable in the scope of this lab.

`timeout=0.01` makes sure that the reading is non-blocking, which otherwise may result in GUI freezing.

#### 4.2.3 Data Acquisition using Qtimer()

```python
self.timer = QTimer()
self.timer.timeout.connect(self.read_serial)
self.timer.start(10)
```

`QTimer` triggers triggers `read_serial()` every 10 milliseconds - the same sampling rate specified in Arduino - around 100 Hz. Every 10 milliseconds, the data will be read, parsed and visualized.

#### 4.2.4 Parsing Data from Serial

```python
line = self.ser.readline().decode("utf-8").strip()
parts = line.split(",")
```

- Data is split into with respect to commas:
- `jx` → X axis
- `jy` → Y axis
- `direction` → textual state

To make sure that exactly 3 parameters are parsed, can double-check using:
```python
if len(parts) != 3:
    return
```

represents by itself extra safety precaution.

#### 4.2.5 Real-Time Updates

After parsing, labels are updated immediately with new incoming values, providing instant feedback to the user:
```python
self.x_label.setText(...)
self.y_label.setText(...)
self.dir_label.setText(...)
```

#### 4.2.6 Sampling Rate Measurement

- `self.samples += 1` is responsible for counting received samples.

```python
if now - self.last_time >= 1.0:
    self.rate_label.setText(f"Sample Rate: {self.samples} Hz")
```

Each second, effective sample rate is calculated and the real system performance is displayed, instead of ideal 100 Hz sampling frequency. This can be traced in the above image showing GUI window, where sampling rate is indicated as 101 Hz. System may operate at slightly higher or lower frequencies, this is because the execution time of the code is not perfectly constant: minor delays introduced by ADC readings, serial transmission, GUI processing, corrupted bits and etc. causing those deviations.

#### 4.2.7 Graphical Visualization

`JoystickCanvas` class is responsible for rendering the joystick position.

```python
x = int((self.gui.jx / 1023) * w)
y = int((1 - self.gui.jy / 1023) * h)
```

By converting 10-bit ADC values (0-1023) into coordinates, it scales them proportionally to the window size, so that when window display size is changed by the cursor, those the steps of those coordinates will increase/decrease respectively.

Since the screen origin by default is located at the top left, the Y axis should be mirrored: `1 - self.gui.jy / 1023`

```python
painter.drawEllipse(x - 10, y - 10, 20, 20)
```
 Represents joystick position as a moving point.

Cross lines are divided into 4 regions. Border defines the visualization limits.

#### 4.2.8 Start and Stop Control

The button, mentioned earlier, allows user to pause or resume the program's data acquisition

```python
self.toggle_button.clicked.connect(self.toggle_test)
```

This line connects the button press event to the `toggle_test()` function, so that, whenever the clicks the button is clicked, program switches between running and paused states.

### 4.3 System Diagram

The overall flow of the system and how two programs work in junction can be better understood by looking at the system diagram below

![image](images/Arduino%20Joystick%20Input-2026-03-18-151047.png)

---
## 5. Conclusion

Lab 2’s joystick-based system was transformed into an actual real-time monitoring and display system for the purposes of this lab. The Arduino code used in this lab was written using PlatformIO and Visual Studio Code and continuously monitored the X and Y inputs from the joystick. It then used the inputs to determine the associated direction and send the data to the computer via UART serial communication. 

A PyQt6 based graphical user interface (GUI) was developed to provide a better visual representation of the data coming into the system. The GUI provided a visual representation of the X and Y readings currently being received from the joystick, identified the current direction of the joystick, reported on the system status, reported on the effective sampling rate of the system, and visually represented the location of the joystick. This method allowed the observer to see the performance of the analog input device as it relates to the system in comparison to viewing the data in a serial monitor. 

Testing demonstrated that the system performed as expected and responded quickly with the measured update rate being close to the target 100Hz. The minor variations around this target due to execution and communication overhead did not affect the overall functionality of the system as had been expected.

This lab showed how embedded programming, serial communication, and desktop-based visualization can be combined to allow for processing and monitoring of microcontroller data in real time using a high level graphical interface.

