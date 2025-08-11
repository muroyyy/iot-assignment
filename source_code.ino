/*
  Smart Distance-Based Security Alarm
  Hardware: Arduino Uno/Nano, HC-SR04, Active Buzzer, LED (+220Ω)
  Pins:
    TRIG  -> D9
    ECHO  -> D8
    BUZZ  -> D6
    LED   -> D5
*/

#define TRIG_PIN 9
#define ECHO_PIN 8
#define BUZZER_PIN 6
#define LED_PIN 5

// --- Tunables ---
const float THRESHOLD_CM       = 50.0;   // intrusion radius
const uint8_t SAMPLE_COUNT     = 5;      // samples per reading (noise reduction)
const uint8_t BREACH_REQUIRED  = 2;      // consecutive breaches before alarm
const unsigned long ALARM_MS   = 5000;   // how long to sound the alarm
const unsigned long COOLDOWN_MS= 20000;  // wait time after an alarm
const unsigned long POLL_MS    = 150;    // distance check interval

// Buzzer pattern (non-blocking): on/off toggles every BUZZ_TOGGLE_MS
const unsigned long BUZZ_TOGGLE_MS = 120;

// --- Internal state ---
unsigned long lastPoll = 0;
unsigned long alarmStartedAt = 0;
unsigned long cooldownUntil = 0;
unsigned long lastBuzzToggle = 0;

bool alarmActive = false;
bool buzzerOn = false;
uint8_t consecutiveBreaches = 0;

float measureDistanceCmOnce() {
  // Send 10us pulse on TRIG, then read ECHO duration
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Timeout 30,000us (~5m distance) to avoid blocking indefinitely
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);

  if (duration == 0) {
    // No echo within timeout – return invalid reading
    return -1.0;
  }

  // Speed of sound ~343 m/s => 0.0343 cm/us; divide by 2 (out-and-back)
  return (duration * 0.0343) / 2.0;
}

float measureDistanceCmFiltered(uint8_t samples) {
  uint8_t valid = 0;
  float sum = 0.0;

  for (uint8_t i = 0; i < samples; i++) {
    float d = measureDistanceCmOnce();
    if (d > 0) {  // ignore invalids
      sum += d;
      valid++;
    }
    delay(10); // small spacing between pings
  }

  if (valid == 0) return -1.0;
  return sum / valid; // simple average; robust enough for HC-SR04
}

void startAlarm() {
  alarmActive = true;
  alarmStartedAt = millis();
  lastBuzzToggle = 0; // force immediate buzz
  buzzerOn = false;
  digitalWrite(LED_PIN, HIGH);
}

void stopAlarm() {
  alarmActive = false;
  digitalWrite(BUZZER_PIN, LOW);
  buzzerOn = false;
  digitalWrite(LED_PIN, LOW);
  cooldownUntil = millis() + COOLDOWN_MS;
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  delay(200);
  Serial.println(F("Smart Distance-Based Security Alarm: armed."));
}

void loop() {
  unsigned long now = millis();

  // Cooldown period after an alarm
  if (now < cooldownUntil) {
    // Still cooling down; do nothing except idle
  } else {
    // Time to poll distance?
    if (now - lastPoll >= POLL_MS) {
      lastPoll = now;

      float distance = measureDistanceCmFiltered(SAMPLE_COUNT);
      if (distance > 0) {
        Serial.print(F("Distance: "));
        Serial.print(distance, 1);
        Serial.println(F(" cm"));
      } else {
        Serial.println(F("Distance: invalid"));
      }

      // Decide breach
      if (distance > 0 && distance <= THRESHOLD_CM) {
        if (consecutiveBreaches < 255) consecutiveBreaches++;
      } else {
        consecutiveBreaches = 0;
      }

      // Trigger alarm if threshold met and not already active
      if (!alarmActive && (now >= cooldownUntil) &&
          consecutiveBreaches >= BREACH_REQUIRED) {
        Serial.println(F(">>> INTRUSION: threshold crossed. Starting alarm."));
        startAlarm();
      }
    }
  }

  // Handle alarm lifecycle & buzzer pattern
  if (alarmActive) {
    // Toggle buzzer at fixed interval (non-blocking beeps)
    if (lastBuzzToggle == 0 || (now - lastBuzzToggle >= BUZZ_TOGGLE_MS)) {
      lastBuzzToggle = now;
      buzzerOn = !buzzerOn;
      digitalWrite(BUZZER_PIN, buzzerOn ? HIGH : LOW);
    }

    // End alarm after ALARM_MS
    if (now - alarmStartedAt >= ALARM_MS) {
      Serial.println(F(">>> Alarm window elapsed. Entering cooldown."));
      stopAlarm();
      consecutiveBreaches = 0;
    }
  }
}
