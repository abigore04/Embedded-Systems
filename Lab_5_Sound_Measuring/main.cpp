#include <Arduino.h>
#include <LiquidCrystal.h>
#include <math.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 6);

const byte LED_PIN = 8;
const byte MIC_AO  = A0;
const byte MIC_DO  = 2;

const float ADC_REF_VOLTAGE = 5.0;
const float DB_REFERENCE_V  = 0.02;
float DB_THRESHOLD = 15.0;

const unsigned long WINDOW_MS = 50;
const unsigned long LCD_MS    = 200;
const unsigned long SERIAL_MS = 100;
const unsigned long LED_ON_MS = 300;

unsigned long sampleCount = 0;
double sumSamples = 0.0;
double sumSquares = 0.0;

float vrms = 0.0;
float dbLevel = 0.0;
bool aboveThreshold = false;

volatile bool soundInterruptFlag = false;
volatile unsigned long interruptCount = 0;

unsigned long lastWindowStart = 0;
unsigned long lastLcdUpdate   = 0;
unsigned long lastSerialSend  = 0;
unsigned long ledStartTime    = 0;

bool ledOn = false;

void soundISR() {
  soundInterruptFlag = true;
  interruptCount++;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(MIC_DO, INPUT);

  digitalWrite(LED_PIN, LOW);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Sound dB Meter");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(MIC_DO), soundISR, FALLING);

  lastWindowStart = millis();
  lastLcdUpdate   = millis();
  lastSerialSend  = millis();
}

void loop() {
  unsigned long now = millis();

  int sample = analogRead(MIC_AO);

  sampleCount++;
  sumSamples += sample;
  sumSquares += (double)sample * (double)sample;

  if (now - lastWindowStart >= WINDOW_MS) {
    if (sampleCount > 0) {
      double mean = sumSamples / sampleCount;
      double meanSquare = sumSquares / sampleCount;

      double variancePart = meanSquare - (mean * mean);
      if (variancePart < 0) variancePart = 0;
      double rmsCounts = sqrt(variancePart);

      vrms = (float)(rmsCounts * ADC_REF_VOLTAGE / 1023.0);

      if (vrms < 0.001) {
        vrms = 0.001;
      }

      dbLevel = 20.0 * log10(vrms / DB_REFERENCE_V);

      aboveThreshold = (dbLevel >= DB_THRESHOLD);
    }

    sampleCount = 0;
    sumSamples = 0.0;
    sumSquares = 0.0;
    lastWindowStart = now;
  }

  if (soundInterruptFlag) {
    noInterrupts();
    soundInterruptFlag = false;
    interrupts();

    ledOn = true;
    ledStartTime = now;
    digitalWrite(LED_PIN, HIGH);
  }

  if (ledOn && (now - ledStartTime >= LED_ON_MS)) {
    ledOn = false;
    digitalWrite(LED_PIN, LOW);
  }

  if (now - lastLcdUpdate >= LCD_MS) {
    lastLcdUpdate = now;

    lcd.setCursor(0, 0);
    lcd.print("dB:");
    lcd.print(dbLevel, 1);
    lcd.print("    ");

    lcd.setCursor(10, 0);
    lcd.print("T:");
    lcd.print(DB_THRESHOLD, 0);
    lcd.print(" ");

    lcd.setCursor(0, 1);
    lcd.print("Evt:");
    lcd.print(interruptCount);
    lcd.print("   ");

    lcd.setCursor(10, 1);
    if (aboveThreshold) {
      lcd.print("LOUD ");
    } else {
      lcd.print("quiet");
    }
  }

  if (now - lastSerialSend >= SERIAL_MS) {
    lastSerialSend = now;

    Serial.print(now);
    Serial.print(",");
    Serial.print(dbLevel, 1);
    Serial.print(",");
    Serial.print(aboveThreshold ? 1 : 0);
    Serial.print(",");
    Serial.println(interruptCount);
  }
}
