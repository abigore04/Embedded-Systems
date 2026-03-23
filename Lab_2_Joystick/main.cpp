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
