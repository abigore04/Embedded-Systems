// Workflow:
// 1. Enter 4 digits on keypad + press #
// 2. System locks
// 3. Enter same 4 digits on IR remote
// 4. System unlocks
// 5. RFID becomes active
// 6. RFID UID is printed as: TAG,XXXXXXXX

#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <IRremote.hpp>
#include <string.h>

// IR remote library has built in LED feedback on pin 13, 
// which conflicts with RC522 SPI SCK. Disabled.
#ifndef DISABLE_LED_FEEDBACK
#define DISABLE_LED_FEEDBACK false
#endif

// -------------------- Pins --------------------
#define IR_RECEIVE_PIN 2

#define RFID_SS_PIN   10
#define RFID_RST_PIN  9

#define GREEN_LED A2
#define RED_LED   A3

// -------------------- IR Remote Codes --------------------
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

// -------------------- Keypad --------------------
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {3, 4, 5, 6};
byte colPins[COLS] = {7, 8, A0, A1};

// keypad()
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// -------------------- RFID --------------------
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN); // Create MFRC522 instance to communicate with RC522 RFID reader
bool rfidOk = false; // Tracks if RFID initialized successfully

// -------------------- State --------------------
enum SystemState {
  WAITING_FOR_CODE,
  LOCKED,
  UNLOCKED
};

SystemState currentState = WAITING_FOR_CODE;

// -------------------- Code buffers --------------------
const byte CODE_LENGTH = 4;

// +1 for null terminator since we'll be using string functions for comparison and printing
char lockCode[CODE_LENGTH + 1] = "";      // The code set by the user on the keypad, used for both locking and unlocking
char keypadBuffer[CODE_LENGTH + 1] = "";  // Buffer for accumulating keypad input before pressing #
char irBuffer[CODE_LENGTH + 1] = "";      // Buffer for accumulating IR input before comparing with lockCode

byte keypadIndex = 0;    // Tracks how many digits have been entered on the keypad so far, used for indexing into keypadBuffer
byte irIndex = 0;        // Tracks how many digits have been entered on the IR remote so far, used for indexing into irBuffer

// -------------------- Buffer Helpers --------------------
void clearKeypadBuffer() {
  keypadIndex = 0;
  keypadBuffer[0] = '\0';
}

void clearIrBuffer() {
  irIndex = 0;
  irBuffer[0] = '\0';
}

bool isDigitKey(char key) {
  return key >= '0' && key <= '9';
}

// -------------------- IR Mapping --------------------
int irCommandToDigit(uint8_t command) {
  switch (command) {
    case IR_KEY_0: return 0;
    case IR_KEY_1: return 1;
    case IR_KEY_2: return 2;
    case IR_KEY_3: return 3;
    case IR_KEY_4: return 4;
    case IR_KEY_5: return 5;
    case IR_KEY_6: return 6;
    case IR_KEY_7: return 7;
    case IR_KEY_8: return 8;
    case IR_KEY_9: return 9;
    default: return -1;
  }
}

// -------------------- LEDs --------------------
void setStateLEDs() {
  if (currentState == LOCKED) {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
  }
  else if (currentState == UNLOCKED) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  }
}

void updateWaitingLEDs() {
  static unsigned long lastBlink = 0;
  static bool state = false;

  if (currentState != WAITING_FOR_CODE) {
    return;
  }

  // Blink both LEDs on and off every 500ms to indicate waiting for code state
  if (millis() - lastBlink >= 500) {
    lastBlink = millis();
    state = !state;

    digitalWrite(GREEN_LED, state ? HIGH : LOW);
    digitalWrite(RED_LED, state ? HIGH : LOW);
  }
}

void flashSuccess() {
  for (byte i = 0; i < 3; i++) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, HIGH);
    delay(120);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    delay(120);
  }

  setStateLEDs();
}

void flashWrongCode() {
  for (byte i = 0; i < 3; i++) {
    digitalWrite(RED_LED, HIGH);
    delay(150);
    digitalWrite(RED_LED, LOW);
    delay(150);
  }

  setStateLEDs();
}

// -------------------- State Change --------------------
void changeState(SystemState newState) {
  currentState = newState;

  clearKeypadBuffer();
  clearIrBuffer();

  if (currentState == WAITING_FOR_CODE) {
    Serial.println(F("STATE,WAITING_FOR_CODE"));
    Serial.println(F("INFO,Enter 4 digits on keypad, then press #"));
  }
  else if (currentState == LOCKED) {
    Serial.println(F("STATE,LOCKED"));
    Serial.println(F("INFO,RFID disabled. Use IR remote to unlock."));
    setStateLEDs();
  }
  else if (currentState == UNLOCKED) {
    Serial.println(F("STATE,UNLOCKED"));
    Serial.println(F("INFO,RFID active. Scan card now."));
    setStateLEDs();
  }
}

// -------------------- RFID Init --------------------
void initRFID() {
  // Keep SS high before SPI starts
  pinMode(RFID_SS_PIN, OUTPUT);
  digitalWrite(RFID_SS_PIN, HIGH);

  // Hardware reset pulse for RC522 to ensure it's in a known state before initialization
  pinMode(RFID_RST_PIN, OUTPUT);
  digitalWrite(RFID_RST_PIN, LOW);
  delay(50);
  digitalWrite(RFID_RST_PIN, HIGH);
  delay(100);

  SPI.begin();
  delay(50);

  // Check if RC522 responds correctly before proceeding with setup
  rfid.PCD_Init();
  delay(100);

  // Antenna must be turned on for RC522 to respond to commands, so do this before checking version
  rfid.PCD_AntennaOn();

  // Read version register to verify communication with RC522 to avoid getting stuck in loop later when trying to read tags with a non-responsive RC522
  byte version = rfid.PCD_ReadRegister(rfid.VersionReg);

  // rfid version refers to the version of the RC522 chip, not the library.
  // The version should be 0x91 for RC522, 0x92 for RC522v2, 
  // and some clones may return 0x88. If we get 0x00 or 0xFF then communication likely failed.

  // rfidOk tracks whether RFID initialized successfully.
  // If not, we'll keep trying to initialize in the main loop every 2 seconds,
  // since some RC522 modules can be finicky and may fail to initialize on the first try.

  Serial.print(F("RFID_VERSION,0x"));
  Serial.println(version, HEX);

  if (version == 0x00 || version == 0xFF) {
    Serial.println(F("ERROR,RFID_NOT_RESPONDING"));
    rfidOk = false;
  } else {
    Serial.println(F("INFO,RFID_OK"));
    rfidOk = true;
  }
}

// -------------------- Keypad Handling --------------------
void handleKeypad() {
  char key = keypad.getKey();

  // If no key is pressed, getKey() returns 0, so just return early in that case
  if (!key) {
    return;
  }

  Serial.print(F("KEYPAD,"));
  Serial.println(key);

  if (currentState == LOCKED) {
    Serial.println(F("INFO,Keypad ignored while locked"));
    return;
  }

  if (key == '*') {
    clearKeypadBuffer();
    Serial.println(F("INFO,Keypad cleared"));
    return;
  }

  if (key == '#') {
    if (keypadIndex == CODE_LENGTH) {
      strcpy(lockCode, keypadBuffer);

      Serial.println(F("INFO,Code saved"));
      changeState(LOCKED);
    } else {
      Serial.println(F("ERROR,Need 4 digits before #"));
    }

    return;
  }

  if (isDigitKey(key)) {
    if (keypadIndex < CODE_LENGTH) {
      keypadBuffer[keypadIndex] = key;
      keypadIndex++;
      keypadBuffer[keypadIndex] = '\0';

      Serial.print(F("INFO,Keypad digits="));
      Serial.println(keypadIndex);
    } else {
      Serial.println(F("ERROR,Keypad full. Use * or #"));
    }
  }
}

// -------------------- IR Handling --------------------
void handleIR() {
  // If no IR signal has been received, decode() returns false. Just return early in that case.
  if (!IrReceiver.decode()) {
    return;
  }

  // If the received signal is a repeat code, we can ignore it since we only care about the initial code to unlock,
  // and ignoring repeat codes prevents issues with button hold sending multiple codes.
  if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
    IrReceiver.resume();
    return;
  }

  uint8_t command = IrReceiver.decodedIRData.command; // Hex code of the button pressed, e.g. 0x16 for '0' key
  int digit = irCommandToDigit(command);

  Serial.print(F("IR,0x"));
  Serial.println(command, HEX);

  if (currentState != LOCKED) {
    IrReceiver.resume();
    return;
  }

  if (digit < 0) {
    Serial.println(F("ERROR,Unknown IR key"));
    IrReceiver.resume();
    return;
  }

  if (irIndex < CODE_LENGTH) {
    irBuffer[irIndex] = char('0' + digit);
    irIndex++;
    irBuffer[irIndex] = '\0';

    Serial.print(F("INFO,IR digits="));
    Serial.println(irIndex);
  }

  if (irIndex == CODE_LENGTH) {
    if (strcmp(irBuffer, lockCode) == 0) {
      Serial.println(F("RESULT,IR_CODE_CORRECT"));
      changeState(UNLOCKED);
      flashSuccess();
    } else {
      Serial.println(F("RESULT,IR_CODE_WRONG"));
      clearIrBuffer();
      flashWrongCode();
    }
  }

  IrReceiver.resume();
}

// -------------------- RFID Handling --------------------
void printUidAsTagLine(MFRC522::Uid *uid) {
  Serial.print(F("TAG,"));

  for (byte i = 0; i < uid->size; i++) {
    if (uid->uidByte[i] < 0x10) {
      Serial.print(F("0"));
    }

    Serial.print(uid->uidByte[i], HEX);
  }

  Serial.println();
}

void handleRFID() {
  // If we're not unlocked, RFID should be inactive, so just return early in that case
  if (currentState != UNLOCKED) {
    return;
  }

  // If RFID failed during setup, try reinitializing every 2 seconds
  // This is a workaround for finicky RC522 modules that may fail to initialize on the first try,
  // and since RFID is only needed in the unlocked state, we can keep trying to initialize without
  // affecting the user until it succeeds.
  if (!rfidOk) {
    static unsigned long lastRetry = 0;

    if (millis() - lastRetry >= 2000) {
      lastRetry = millis();
      Serial.println(F("INFO,Retry RFID init"));
      initRFID();
    }

    return;
  }

  // PICC = Proximity Integrated Circuit Card, basically the RFID tag/card.
  // This checks if a new card has been placed near the reader since the last check.
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // This tries to read the card's UID (Unique Identifier). If it fails, we print an error and return early.
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println(F("ERROR,RFID_READ_FAIL"));
    rfid.PICC_HaltA();       // Halt the card, putting it back to sleep. Good practice after reading.
    rfid.PCD_StopCrypto1();  // Stop encryption on PCD (Proximity Coupling Device, i.e. the RC522 reader) after communicating with the card. Good practice.
    return;
  }

  // If we got here, we successfully read a card, so we print the UID as a tag line.
  printUidAsTagLine(&rfid.uid);

  flashSuccess();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(700);
}

// -------------------- Setup / Loop --------------------
void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  Serial.println(F("LAB7_SECURITY_SYSTEM_START"));

  initRFID();

  // IMPORTANT:
  // D13 is SPI SCK for RC522, so IR LED feedback must be disabled.
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);

  changeState(WAITING_FOR_CODE);
}

void loop() {
  updateWaitingLEDs();

  handleKeypad();
  handleIR();
  handleRFID();
}
