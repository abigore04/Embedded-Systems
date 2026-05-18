/*
  IR Receiver Test for Arduino UNO

  Goal:
  - Press buttons 0 to 9 on the remote.
  - Serial Monitor prints Protocol, Command, RawData in HEX.
  - Send this output to ChatGPT later so we can map buttons correctly.

  Wiring:
  IR module S / OUT  -> Arduino D2
  IR module + / VCC  -> Arduino 5V
  IR module - / GND  -> Arduino GND
*/

#include <IRremote.hpp>

#define IR_RECEIVE_PIN 2

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println(F("==================================="));
  Serial.println(F("IR Receiver Test Started"));
  Serial.println(F("Wiring: S/OUT -> D2, + -> 5V, - -> GND"));
  Serial.println(F("Press buttons 0 to 9 one by one."));
  Serial.println(F("Do NOT hold the button. Press shortly."));
  Serial.println(F("==================================="));

  // Start IR receiver on D2.
  // ENABLE_LED_FEEDBACK makes Arduino onboard LED blink when signal is received.
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void loop() {
  if (IrReceiver.decode()) {

    // Ignore repeat frames caused by holding a button
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
      Serial.println(F("REPEAT frame ignored"));
      IrReceiver.resume();
      return;
    }

    Serial.println(F("----- IR BUTTON DETECTED -----"));

    // Built-in short result print
    IrReceiver.printIRResultShort(&Serial);

    // Manual important values
    Serial.print(F("Protocol number: "));
    Serial.println(IrReceiver.decodedIRData.protocol);

    Serial.print(F("Address HEX: 0x"));
    Serial.println(IrReceiver.decodedIRData.address, HEX);

    Serial.print(F("Command HEX: 0x"));
    Serial.println(IrReceiver.decodedIRData.command, HEX);

    Serial.print(F("RawData HEX: 0x"));
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

    Serial.print(F("Bits: "));
    Serial.println(IrReceiver.decodedIRData.numberOfBits);

    Serial.println(F("------------------------------"));
    Serial.println();

    // Prepare receiver for next button
    IrReceiver.resume();
  }
}