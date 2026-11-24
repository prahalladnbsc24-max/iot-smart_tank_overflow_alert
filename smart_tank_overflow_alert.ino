
// ==========================================
// SMART WATER TANK MONITORING SYSTEM
// Arduino UNO R3
// HC-SR04 + LEDs + Buzzer + Push Button + Motor
// ==========================================

// Ultrasonic sensor pins
const int trigPin = 9;
const int echoPin = 10;

// LED pins
const int greenLED = 2;
const int yellowLED = 3;
const int redLED = 4;

// Other component pins
const int buzzerPin = 5;
const int buttonPin = 6;
const int motorPin = 7;

// Tank configuration
const float tankHeightCm = 16.0;
const float tankCapacityLitres = 1.0;

// Water-level thresholds
const float lowLevel = 25.0;
const float fullLevel = 95.0;

// Sensor settings
float emptyDistanceCm = 16.0;

// Reading variables
float distanceCm = 0;
float waterHeightCm = 0;
float waterLevelPercent = 0;
float waterLevelLitres = 0;

// Usage tracking
float waterConsumedLitres = 0;
float previousWaterLevelLitres = -1;

// Pump status
bool pumpOn = false;

// Timing
unsigned long lastSensorRead = 0;
unsigned long lastSerialSend = 0;
unsigned long lastBuzzerTime = 0;

// Button handling
bool buttonPressed = false;
bool longPressHandled = false;
unsigned long buttonPressTime = 0;

// Buzzer handling
bool buzzerOn = false;

// ==========================================
// READ ULTRASONIC SENSOR
// ==========================================

float readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  float distance = duration * 0.0343 / 2.0;

  if (distance < 2 || distance > 400) {
    return -1;
  }

  return distance;
}

// ==========================================
// UPDATE WATER LEVEL
// ==========================================

void updateWaterLevel() {

  distanceCm = readDistance();

  if (distanceCm < 0) {
    waterLevelPercent = -1;
    waterLevelLitres = 0;
    return;
  }

  waterHeightCm = emptyDistanceCm - distanceCm;

  waterHeightCm = constrain(
    waterHeightCm,
    0,
    tankHeightCm
  );

  waterLevelPercent =
    (waterHeightCm / tankHeightCm) * 100.0;

  waterLevelPercent = constrain(
    waterLevelPercent,
    0,
    100
  );

  waterLevelLitres =
    (waterLevelPercent / 100.0) *
    tankCapacityLitres;

  // Estimate consumption when the water level decreases
  if (previousWaterLevelLitres >= 0) {

    float difference =
      previousWaterLevelLitres - waterLevelLitres;

    if (difference > 0) {
      waterConsumedLitres += difference;
    }
  }

  previousWaterLevelLitres = waterLevelLitres;
}

// ==========================================
// LED INDICATORS
// ==========================================

void updateLEDs() {

  if (waterLevelPercent < 0) {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
    return;
  }

  if (waterLevelPercent < 30) {

    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);

  } else if (waterLevelPercent < 70) {

    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);

  } else {

    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
  }
}

// ==========================================
// PUMP CONTROL
// ==========================================

void updatePump() {

  // Sensor error: keep motor OFF
  if (waterLevelPercent < 0) {
    pumpOn = false;
  }

  // Start pump when water level is low
  else if (waterLevelPercent <= lowLevel) {
    pumpOn = true;
  }

  // Stop pump when tank is nearly full
  else if (waterLevelPercent >= fullLevel) {
    pumpOn = false;
  }

  digitalWrite(
    motorPin,
    pumpOn ? HIGH : LOW
  );
}

// ==========================================
// BUZZER ALERTS
// ==========================================

void updateBuzzer() {

  unsigned long currentTime = millis();

  bool alertRequired =
    waterLevelPercent < 0 ||
    waterLevelPercent >= fullLevel;

  if (!alertRequired) {
    noTone(buzzerPin);
    buzzerOn = false;
    return;
  }

  if (currentTime - lastBuzzerTime >= 1000) {

    lastBuzzerTime = currentTime;
    buzzerOn = !buzzerOn;

    if (buzzerOn) {
      tone(buzzerPin, 2000);
    } else {
      noTone(buzzerPin);
    }
  }
}

// ==========================================
// BUTTON FUNCTIONALITY
// Short press: Calibrate empty tank
// Long press: Display usage statistics
// ==========================================

void updateButton() {

  bool currentState = digitalRead(buttonPin);
  unsigned long currentTime = millis();

  if (currentState == LOW && !buttonPressed) {

    buttonPressed = true;
    longPressHandled = false;
    buttonPressTime = currentTime;
  }

  if (buttonPressed && currentState == LOW) {

    if (
      !longPressHandled &&
      currentTime - buttonPressTime >= 1000
    ) {

      longPressHandled = true;

      Serial.println(
        "EVENT: USAGE STATISTICS"
      );

      Serial.print(
        "Water consumed: "
      );

      Serial.print(waterConsumedLitres, 3);
      Serial.println(" litres");
    }
  }

  if (buttonPressed && currentState == HIGH) {

    if (!longPressHandled) {

      if (distanceCm > 0) {

        emptyDistanceCm = distanceCm;

        Serial.println(
          "Empty tank calibration successful"
        );
      }
    }

    buttonPressed = false;
  }
}

// ==========================================
// SEND DATA THROUGH SERIAL
// ==========================================

void sendSerialData() {

  if (waterLevelPercent < 0) {
    return;
  }

  Serial.print("{\"waterLevelPercent\":");
  Serial.print(waterLevelPercent, 2);

  Serial.print(",\"waterLevelLitres\":");
  Serial.print(waterLevelLitres, 3);

  Serial.print(",\"waterConsumedLitres\":");
  Serial.print(waterConsumedLitres, 3);

  // No energy sensor is included in this component list.
  // Therefore, actual energy consumption is not measured.
  Serial.print(",\"energyUsedKwh\":0");

  Serial.print(",\"pumpStatus\":\"");
  Serial.print(pumpOn ? "ON" : "OFF");

  Serial.print("\",\"source\":\"ARDUINO\"}");

  Serial.println();
}

// ==========================================
// SETUP
// ==========================================

void setup() {

  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  pinMode(buzzerPin, OUTPUT);
  pinMode(motorPin, OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(motorPin, LOW);

  Serial.println(
    "SmartTank Arduino System Started"
  );
}

// ==========================================
// MAIN LOOP
// ==========================================

void loop() {

  unsigned long currentTime = millis();

  updateButton();

  // Read sensor every 500 milliseconds
  if (currentTime - lastSensorRead >= 500) {

    lastSensorRead = currentTime;

    updateWaterLevel();
    updateLEDs();
    updatePump();
    updateBuzzer();
  }

  // Send readings every 2 seconds
  if (currentTime - lastSerialSend >= 2000) {

    lastSerialSend = currentTime;

    sendSerialData();
  }
}
