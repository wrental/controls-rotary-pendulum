// ============================================================
//  Same Sky AMT10 + Continuous Rotation Servo
//  Arduino Uno
//
//  Wiring:
//    Encoder G  (GND)   → Arduino GND
//    Encoder 5V (+5V)   → Arduino 5V
//    Encoder A          → Arduino Pin 2  (INT0)
//    Encoder B          → Arduino Pin 4
//    Encoder X (index)  → Arduino Pin 3  (INT1)  [optional]
//    Servo signal       → Arduino Pin 9
//    Servo power        → External 5V (do NOT power from Arduino 5V)
//    Servo GND          → Arduino GND
//
//  Behaviour:
//    1. On boot, records current position as home (0°)
//    2. Runs servo forward until TARGET_DEGREES is reached
//    3. Stops briefly, then runs servo in reverse back to 0°
//    4. Stops and holds at home
// ============================================================

#include <Servo.h>

// --- Pin definitions ---
#define PIN_A      2   // Interrupt pin (INT0)
#define PIN_B      4
#define PIN_INDEX  3   // Interrupt pin (INT1)
#define PIN_SERVO  9

// --- Encoder resolution: match your DIP switch setting ---
const int  PPR            = 512;
const long COUNTS_PER_REV = (long)PPR * 4;  // 2048 counts/rev

// --- Motion settings ---
const float TARGET_DEGREES = 720.0;  // how far to travel before reversing
const int   STOP_PAUSE_MS  = 1000;    // pause (ms) between forward and reverse

// Continuous servo pulse widths (tune these for your specific servo)
// Most servos: 1500 = stop, <1500 = one direction, >1500 = other direction
const int SERVO_STOP     = 1500;
const int SERVO_FORWARD  = 2000;  // increase for faster forward
const int SERVO_REVERSE  = 1000;  // decrease for faster reverse

// --- Encoder state ---
volatile long encoderCount = 0;
volatile bool lastA        = false;

// --- Servo ---
Servo servo;

// --- Program state ---
enum State { MOVING_FORWARD, PAUSING, MOVING_REVERSE, DONE };
State currentState = MOVING_FORWARD;

long targetCounts;  // counts equivalent of TARGET_DEGREES

// -------------------------------------------------------
//  ISR: Quadrature decode on channel A
// -------------------------------------------------------
void onEncoderA() {
  bool a = digitalRead(PIN_A);
  bool b = digitalRead(PIN_B);
  if (a != lastA) {
    if (a == b) {
      encoderCount--;   // CW
    } else {
      encoderCount++;   // CCW
    }
    lastA = a;
  }
}

// -------------------------------------------------------
//  Helper: convert degrees to encoder counts
// -------------------------------------------------------
long degreesToCounts(float degrees) {
  return (long)(degrees * COUNTS_PER_REV / 360.0);
}

// -------------------------------------------------------
//  Helper: get current count snapshot safely
// -------------------------------------------------------
long getCount() {
  noInterrupts();
  long c = encoderCount;
  interrupts();
  return c;
}

// -------------------------------------------------------
//  Setup
// -------------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(PIN_A,     INPUT_PULLUP);
  pinMode(PIN_B,     INPUT_PULLUP);
  pinMode(PIN_INDEX, INPUT_PULLUP);
  lastA = digitalRead(PIN_A);
  attachInterrupt(digitalPinToInterrupt(PIN_A), onEncoderA, CHANGE);

  servo.attach(PIN_SERVO);
  servo.writeMicroseconds(SERVO_STOP);
  delay(500);  // let servo settle

  targetCounts = degreesToCounts(TARGET_DEGREES);

  Serial.println(F("=== AMT10 + Servo Ready ==="));
  Serial.print(F("Target: "));
  Serial.print(TARGET_DEGREES);
  Serial.print(F("° = "));
  Serial.print(targetCounts);
  Serial.println(F(" counts"));
  Serial.println(F("Moving forward..."));

  servo.writeMicroseconds(SERVO_FORWARD);
}

// -------------------------------------------------------
//  Loop
// -------------------------------------------------------
void loop() {
  long count = getCount();

  switch (currentState) {

    case MOVING_FORWARD:
      if (count >= targetCounts) {
        servo.writeMicroseconds(SERVO_STOP);
        Serial.print(F("Target reached at "));
        Serial.print(count);
        Serial.println(F(" counts. Pausing..."));
        currentState = PAUSING;
        delay(STOP_PAUSE_MS);
        Serial.println(F("Returning to home..."));
        servo.writeMicroseconds(SERVO_REVERSE);
        currentState = MOVING_REVERSE;
      }
      break;

    case MOVING_REVERSE:
      if (count <= 0) {
        servo.writeMicroseconds(SERVO_STOP);
        Serial.print(F("Home reached at "));
        Serial.print(count);
        Serial.println(F(" counts. Done."));
        currentState = DONE;
      }
      break;

    case DONE:
      // Hold stopped — reset encoder and restart to run again,
      // or just leave it here. Uncomment below to loop forever:
      //
      // delay(2000);
      // noInterrupts(); encoderCount = 0; interrupts();
      // servo.writeMicroseconds(SERVO_FORWARD);
      // currentState = MOVING_FORWARD;
      // Serial.println(F("Restarting..."));
      break;

    default:
      break;
  }

  // --- Serial telemetry ---
  float angle      = (float)(count % COUNTS_PER_REV) * 360.0 / COUNTS_PER_REV;
  if (angle < 0) angle += 360.0;
  float totalAngle = (float)count * 360.0 / COUNTS_PER_REV;

  Serial.print(F("Angle: "));   Serial.print(angle, 1);
  Serial.print(F("°  Total: ")); Serial.print(totalAngle, 1);
  Serial.print(F("°  Counts: ")); Serial.print(count);
  Serial.print(F("  State: "));
  switch (currentState) {
    case MOVING_FORWARD:  Serial.println(F("FORWARD"));  break;
    case PAUSING:         Serial.println(F("PAUSING"));  break;
    case MOVING_REVERSE:  Serial.println(F("REVERSE"));  break;
    case DONE:            Serial.println(F("DONE"));     break;
  }

  delay(50);
}
