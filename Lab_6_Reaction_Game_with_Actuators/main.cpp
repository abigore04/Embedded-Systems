#include <Arduino.h>
#include <Servo.h>

// ===================== PINS =====================
const byte BTN1_PIN   = 2;
const byte BTN2_PIN   = 3;
const byte BUZZER_PIN = 4;
const byte SERVO_PIN  = 5;

const byte IN1_PIN = 8;
const byte IN2_PIN = 9;
const byte IN3_PIN = 10;
const byte IN4_PIN = 11;

// ===================== CONFIG =====================
const bool USE_PASSIVE_BUZZER = false;

const int SERVO_NEUTRAL  = 90;
const int SERVO_P1_ANGLE = 0;
const int SERVO_P2_ANGLE = 180;

const int WIN_SCORE = 3;

const unsigned long DEBOUNCE_MS     = 25;
const unsigned long INTER_ROUND_MS  = 1200; // Time between rounds to show result before starting next round
const unsigned long ACTIVE_TIMEOUT_MS = 3000; // Max time to wait for button press after "GO" signal before finishing round

// 28BYJ-48 + ULN2003 in half-step mode
const int STEPS_PER_REV = 4096;           // ~360°
const int TUG_STEP_STEPS = STEPS_PER_REV / 12; // ~30° per round win

unsigned int stepDelayMicros = 1500; // Delay between stepper steps in microseconds (adjust for speed, but don't go too low or it will miss steps)

// ===================== STATE =====================
enum GameState {
  IDLE,         // waiting for match to start
  WAIT_RANDOM,  // waiting for random delay before "GO" signal 
  ACTIVE,       // "GO" signal given, waiting for players to react
  INTER_ROUND,  // waiting between rounds after showing result
  MATCH_OVER    // match ended, waiting for reset
};

GameState state = IDLE; // current game state 

// ===================== PLAYERS =====================
String player1 = "P1";
String player2 = "P2";
int score1 = 0;
int score2 = 0;

// ===================== TIME =====================
unsigned long waitStartMs = 0;        // when we entered WAIT_RANDOM state, used to calculate elapsed time for random delay
unsigned long randomDelayMs = 0;      // random delay duration in milliseconds before "GO" signal, set at start of each round in WAIT_RANDOM state
unsigned long interRoundStartMs = 0;  // when we entered INTER_ROUND state, used to calculate elapsed time before starting next round
unsigned long buzzStartMs = 0;        // when we gave "GO" signal, used to calculate elapsed time for ACTIVE state timeout and to calculate reaction times
unsigned long buzzTimeUs = 0;         // when we gave "GO" signal, in microseconds, used to calculate player reaction times in microseconds for better precision

// ===================== BUZZER =====================
bool buzzerOn = false;
unsigned long buzzerOffAtMs = 0;

// ===================== SERVO =====================
Servo winnerServo;  // servo object to indicate round winner by pointing toward the player

// ===================== STEPPER =====================
// Define the stepper motor control pins and the half-step sequence for the 28BYJ-48 stepper motor
const byte stepPins[4] = {IN1_PIN, IN2_PIN, IN3_PIN, IN4_PIN};

// Half-step sequence for 28BYJ-48 stepper motor
const byte halfStepSeq[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

int stepIndex = 0;
long currentPositionSteps = 0;   // current stepper position relative to home(0°), positive steps toward P1, negative steps toward P2

// ===================== REACTION =====================
// when -1, it means player hasn't pressed yet. when >= 0, it represents the reaction time in microseconds
long reaction1Us = -1;
long reaction2Us = -1;

// ===================== BUTTON DEBOUNCE =====================
struct ButtonDebounce {
  byte pin;
  bool lastReading;
  bool stableState;
  unsigned long lastDebounceTime;
};

ButtonDebounce btn1 = {BTN1_PIN, HIGH, HIGH, 0};
ButtonDebounce btn2 = {BTN2_PIN, HIGH, HIGH, 0};

// ===================== HELPERS =====================
bool readButtonPressed(ButtonDebounce &btn) {
  bool reading = digitalRead(btn.pin);
  bool pressed = false;

  if (reading != btn.lastReading) {
    btn.lastDebounceTime = millis();
  }

  if ((millis() - btn.lastDebounceTime) > DEBOUNCE_MS) {
    if (reading != btn.stableState) {
      btn.stableState = reading;
      if (btn.stableState == LOW) { // INPUT_PULLUP
        pressed = true;
      }
    }
  }

  btn.lastReading = reading;
  return pressed;
}

void startGoSignal() {
  if (USE_PASSIVE_BUZZER) {
    tone(BUZZER_PIN, 2000, 60);
  } else {
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerOn = true;
    buzzerOffAtMs = millis() + 60;
  }
}

void updateBuzzer() {
  if (!USE_PASSIVE_BUZZER && buzzerOn && millis() >= buzzerOffAtMs) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerOn = false;
  }
}

void setStepperOutput(const byte pattern[4]) {
  for (byte i = 0; i < 4; i++) {
    digitalWrite(stepPins[i], pattern[i]);
  }
}

void releaseStepper() {
  for (byte i = 0; i < 4; i++) {
    digitalWrite(stepPins[i], LOW);
  }
}

void stepMotor(int direction) {
  stepIndex += direction;

  if (stepIndex > 7) stepIndex = 0;
  if (stepIndex < 0) stepIndex = 7;

  setStepperOutput(halfStepSeq[stepIndex]);
  delayMicroseconds(stepDelayMicros);
}

void moveStepperSteps(int steps, int direction) {
  for (int i = 0; i < steps; i++) {
    stepMotor(direction);
  }
  releaseStepper();
}

void moveTugTowardWinner(int winner) {
  if (winner == 1) {
    moveStepperSteps(TUG_STEP_STEPS, +1);
    currentPositionSteps += TUG_STEP_STEPS;
  } else if (winner == 2) {
    moveStepperSteps(TUG_STEP_STEPS, -1);
    currentPositionSteps -= TUG_STEP_STEPS;
  }
}

void centerTug() {
  if (currentPositionSteps > 0) {
    moveStepperSteps(currentPositionSteps, -1);
  } else if (currentPositionSteps < 0) {
    moveStepperSteps(-currentPositionSteps, +1);
  }

  currentPositionSteps = 0;
  releaseStepper();
}

void victorySpinAndReturnHome() {
  // Full 360° spin from current angle
  moveStepperSteps(STEPS_PER_REV, +1);

  // Return to initial home angle (0°)
  centerTug();
}

void setServoToWinner(int winner) {
  if (winner == 1) {
    winnerServo.write(SERVO_P1_ANGLE);
  } else if (winner == 2) {
    winnerServo.write(SERVO_P2_ANGLE);
  } else {
    winnerServo.write(SERVO_NEUTRAL);
  }
}

void resetRoundData() {
  // no reaction 
  reaction1Us = -1;
  reaction2Us = -1;
}

void startRound() {
  resetRoundData();
  randomDelayMs = random(1000, 20001); // 1..20 sec
  waitStartMs = millis();
  state = WAIT_RANDOM;
  Serial.println("ROUND_WAIT");
}

void resetMatch() {
  score1 = 0;
  score2 = 0;
  state = IDLE;
  resetRoundData();
  winnerServo.write(SERVO_NEUTRAL);
  centerTug();
}

void sendRoundResult(int winner, bool falseStart) {
  long r1ms = (reaction1Us < 0) ? -1 : reaction1Us / 1000;
  long r2ms = (reaction2Us < 0) ? -1 : reaction2Us / 1000;

  Serial.print("ROUND,");
  Serial.print(winner);
  Serial.print(",");
  Serial.print(r1ms);
  Serial.print(",");
  Serial.print(r2ms);
  Serial.print(",");
  Serial.print(falseStart ? 1 : 0);
  Serial.print(",");
  Serial.print(score1);
  Serial.print(",");
  Serial.println(score2);
}

// sends match result with winner name or "NONE" in case of tie, in format: MATCH,WinnerName or MATCH,NONE
void sendMatchResult(int winner) {
  Serial.print("MATCH,");
  if (winner == 1) Serial.println(player1);
  else if (winner == 2) Serial.println(player2);
  else Serial.println("NONE");
}

void finishRound(int winner, bool falseStart) {
  if (winner == 1) {
    score1++;
    setServoToWinner(1);
    moveTugTowardWinner(1);
  } else if (winner == 2) {
    score2++;
    setServoToWinner(2);
    moveTugTowardWinner(2);
  } else {
    setServoToWinner(0);
  }

  sendRoundResult(winner, falseStart);

  if (score1 >= WIN_SCORE) {
    victorySpinAndReturnHome();
    sendMatchResult(1);
    state = MATCH_OVER;
  } else if (score2 >= WIN_SCORE) {
    victorySpinAndReturnHome();
    sendMatchResult(2);
    state = MATCH_OVER;
  } else {
    interRoundStartMs = millis();
    state = INTER_ROUND;
  }
}

// handleCommand() will be called with each line received from serial (without newline chars)
// it will parse the line and execute the corresponding command, then send response back via serial
void handleCommand(String line) {
  line.trim();
  if (line.length() == 0) return;

  if (line == "PING") {
    Serial.println("PONG");
    return;
  }

  if (line == "RESET") {
    resetMatch();
    Serial.println("RESET_OK");
    return;
  }

  // START command format: START,Player1Name,Player2Name
  if (line.startsWith("START,")) {
    int firstComma = line.indexOf(',');
    int secondComma = line.indexOf(',', firstComma + 1);

    if (secondComma == -1) {
      Serial.println("ERR,BAD_START");
      return;
    }

    player1 = line.substring(firstComma + 1, secondComma);
    player2 = line.substring(secondComma + 1);

    // Trim player names to remove extra spaces and ensure they are not empty
    player1.trim();
    player2.trim();

    // If player names are empty, assign default names
    if (player1.length() == 0) player1 = "P1";
    if (player2.length() == 0) player2 = "P2";

    score1 = 0;
    score2 = 0;
    winnerServo.write(SERVO_NEUTRAL);
    centerTug(); // reset tug to center at start of match

    Serial.println("START_OK");
    startRound();
    return;
  }

  Serial.println("ERR,UNKNOWN_CMD");
}

// check serial for commands and handle buzzer timing
// when line comes fully, handleCommand() will be called with the line (without newline chars)
void updateSerial() {
  static String line = "";

  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (line.length() > 0) {
        handleCommand(line);
        line = "";
      }
    } else {
      line += c;
    }
  }
}

void setup() {
  Serial.begin(115200);
  // A0 is used because it's an unconnected analog pin, so it will read random noise which is good for
  // seeding the random number generator to get different random delays each time the game is played
  randomSeed(analogRead(A0)); // Seed random number generator with noise from an unconnected analog pin

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // stepper motor pins
  for (byte i = 0; i < 4; i++) {
    pinMode(stepPins[i], OUTPUT);
    digitalWrite(stepPins[i], LOW);
  }

  winnerServo.attach(SERVO_PIN); // attach servo to pin is needed before we can write to it because it initializes the servo library and sets the pin mode
  winnerServo.write(SERVO_NEUTRAL); // set to neutral position

  Serial.println("READY");
}

void loop() {
  updateSerial(); // to check whether there are any incoming serial commands and handle them
  updateBuzzer(); // turn off buzzer if its time has come

  // check whether buttons are pressed with debounce handling, and update game state accordingly
  bool p1Pressed = readButtonPressed(btn1);
  bool p2Pressed = readButtonPressed(btn2);

  switch (state) {
    case IDLE:
      break;

    case WAIT_RANDOM:
      if (p1Pressed) {
        finishRound(2, true); // false start by P1
      } else if (p2Pressed) {
        finishRound(1, true); // false start by P2
      } else if (millis() - waitStartMs >= randomDelayMs) {
        buzzStartMs = millis();
        buzzTimeUs = micros();
        startGoSignal();
        Serial.println("GO");
        state = ACTIVE;
      }
      break;

    case ACTIVE:
      if (p1Pressed && reaction1Us < 0) {
        reaction1Us = (long)(micros() - buzzTimeUs);
      }

      if (p2Pressed && reaction2Us < 0) {
        reaction2Us = (long)(micros() - buzzTimeUs);
      }

      if (reaction1Us >= 0 && reaction2Us >= 0) {
        if (reaction1Us < reaction2Us) finishRound(1, false);
        else if (reaction2Us < reaction1Us) finishRound(2, false);
        else finishRound(0, false);
      } else if (millis() - buzzStartMs >= ACTIVE_TIMEOUT_MS) {
        if (reaction1Us >= 0 && reaction2Us < 0) finishRound(1, false);
        else if (reaction2Us >= 0 && reaction1Us < 0) finishRound(2, false);
        else finishRound(0, false);
      }
      break;

    case INTER_ROUND:
      if (millis() - interRoundStartMs >= INTER_ROUND_MS) {
        startRound();
      }
      break;

    case MATCH_OVER:
      break;
  }
}