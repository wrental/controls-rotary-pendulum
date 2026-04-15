// ============================================================
//  Same Sky AMT10 — Quadrature Encoder Reader
//  Arduino Uno
//
//  Wiring:
//    Encoder G  (GND)   → Arduino GND
//    Encoder 5V (+5V)   → Arduino 5V
//    Encoder A          → Arduino Pin 2  (INT0)
//    Encoder B          → Arduino Pin 4
//    Encoder X (index)  → Arduino Pin 3  (INT1)  [optional]
//
//  The AMT10 defaults to 2048 PPR.
//  Quadrature decoding gives 4× counts: 8192 counts per revolution.
//  Change PPR_SETTING below if you've changed the DIP switches.
// ============================================================

// --- Pin definitions ---
#define PIN_A     2   // Must be an interrupt pin (Uno: 2 or 3)
#define PIN_B     4
#define PIN_INDEX 3   // Must be an interrupt pin (Uno: 2 or 3)

// --- Resolution: set this to match your DIP switch setting ---
// Default from factory is 2048 PPR → 8192 counts/rev (PPR × 4)
const int   PPR            = 512;
const long  COUNTS_PER_REV = (long)PPR * 4;  // quadrature × 4

// --- State ---
volatile long encoderCount  = 0;   // total counts from boot position
volatile long indexCount    = 0;   // counts at last index pulse (for reference)
volatile int  rotations     = 0;   // full rotations (can be negative)
volatile bool indexDetected = false;

// Track last A state for direction
volatile bool lastA = false;

// -------------------------------------------------------
//  ISR: Channel A changed — standard quadrature decode
// -------------------------------------------------------
void onEncoderA() {
  bool a = digitalRead(PIN_A);
  bool b = digitalRead(PIN_B);

  // A leads B → CCW (per datasheet); adjust sign to taste
  if (a != lastA) {            // confirm a real edge
    if (a == b) {
      encoderCount--;          // CW
    } else {
      encoderCount++;          // CCW
    }
    lastA = a;
  }

  // Update full-rotation counter
  rotations = (int)(encoderCount / COUNTS_PER_REV);
}

// -------------------------------------------------------
//  ISR: Index pulse (one per revolution)
// -------------------------------------------------------
void onIndex() {
  indexCount    = encoderCount;
  indexDetected = true;
}

// -------------------------------------------------------
//  Helpers
// -------------------------------------------------------

// Returns current angle in degrees [0, 360)
float getCurrentAngle() {
  // Use modulo to get position within the current revolution
  long countsInRev = encoderCount % COUNTS_PER_REV;
  if (countsInRev < 0) countsInRev += COUNTS_PER_REV;  // handle negative
  return (float)countsInRev * 360.0f / (float)COUNTS_PER_REV;
}

// Returns total angle from boot position (can exceed 360 or go negative)
float getTotalAngle() {
  return (float)encoderCount * 360.0f / (float)COUNTS_PER_REV;
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

  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(PIN_A),     onEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_INDEX), onIndex,    RISING);

  Serial.println(F("=== AMT10 Encoder Ready ==="));
  Serial.print  (F("Resolution: ")); Serial.print(PPR);
  Serial.print  (F(" PPR  |  Counts/rev: ")); Serial.println(COUNTS_PER_REV);
  Serial.println(F("Boot position = 0 degrees, 0 rotations"));
  Serial.println(F("-------------------------------------------"));
}

// -------------------------------------------------------
//  Loop — print every 100 ms
// -------------------------------------------------------
void loop() {
  // Snapshot volatile values with interrupts briefly disabled
  noInterrupts();
  long  countSnap    = encoderCount;
  int   rotSnap      = rotations;
  bool  idxFlag      = indexDetected;
  long  idxCount     = indexCount;
  indexDetected      = false;         // clear flag
  interrupts();

  float angle      = getCurrentAngle();
  float totalAngle = getTotalAngle();

  Serial.print(F("Angle: "));
  Serial.print(angle, 2);
  Serial.print(F("°  |  Total: "));
  Serial.print(totalAngle, 2);
  Serial.print(F("°  |  Rotations: "));
  Serial.print(rotSnap);
  Serial.print(F("  |  Counts: "));
  Serial.print(countSnap);

  if (idxFlag) {
    Serial.print(F("  [INDEX at count "));
    Serial.print(idxCount);
    Serial.print(F("]"));
  }

  Serial.println();
  delay(100);
}
