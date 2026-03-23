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
