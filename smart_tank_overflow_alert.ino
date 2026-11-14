
#include <EEPROM.h>
#include <Adafruit_LiquidCrystal.h>

Adafruit_LiquidCrystal lcd(0);

// =============== PIN CONFIGURATION ===============

const int GREEN_LED = 2;
const int YELLOW_LED = 3;
const int RED_LED = 4;

const int TRIG_PIN = 5;
const int ECHO_PIN = 6;

const int BUTTON_PIN = 7;
const int BUZZER_PIN = 8;
const int PUMP_PIN = 9;

// =============== PROJECT SETTINGS ===============

const int LOW_LEVEL = 40;
const int FULL_LEVEL = 80;
const int STOP_LEVEL = 90;

const float TANK_CAPACITY_L = 500.0;
const float PUMP_POWER_W = 750.0;
const float ELECTRICITY_RATE = 8.0;

const int USE_DEADBAND = 2;

const unsigned long DAY_LENGTH_MS = 120000UL;
const unsigned long MEASURE_INTERVAL = 500UL;
const unsigned long SAVE_INTERVAL = 30000UL;
const unsigned long REPORT_INTERVAL = 10000UL;

const unsigned long LONG_PRESS_MS = 1000UL;
const unsigned long PAGE_INTERVAL_MS = 2000UL;

const int LOG_ADDR = 8;
const unsigned int LOG_MAGIC = 0x51A7;

// =============== EEPROM DATA ===============

struct UsageRecord {
  unsigned int magic;
  float waterUsedToday;
  unsigned long pumpTodaySeconds;
  float yesterdayWater;
  unsigned long yesterdayPumpSeconds;
  unsigned int completedDays;
};

UsageRecord savedData;

// =============== GLOBAL VARIABLES ===============

int tankEmptyCm = 100;
int waterLevel = 0;
long distanceCm = 0;

float waterUsedToday = 0;
float yesterdayWater = 0;

unsigned long pumpRunTodayMs = 0;
unsigned long yesterdayPumpSeconds = 0;

unsigned int completedDays = 0;

bool pumpOn = false;
bool sensorOk = true;
bool logDirty = false;

int referenceLevel = -1;
byte badReads = 0;

unsigned long lastMeasure = 0;
unsigned long lastRuntimeUpdate = 0;
unsigned long lastSave = 0;
unsigned long lastReport = 0;
unsigned long dayStart = 0;
unsigned long messageUntil = 0;

bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;
bool buttonPressed = false;
bool longPressHandled = false;

unsigned long lastDebounceTime = 0;
unsigned long buttonPressStarted = 0;

bool summaryActive = false;
unsigned long summaryStart = 0;

// =============== PUMP CONTROL ===============

void setPump(bool state) {
  pumpOn = state;
  digitalWrite(PUMP_PIN, state ? HIGH : LOW);
}

// =============== ULTRASONIC SENSOR ===============

long readDistanceCm() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration =
    pulseIn(ECHO_PIN, HIGH, 30000UL);

  if (duration == 0) {
    return -1;
  }

  return duration / 58;
}

// =============== EEPROM STORAGE ===============

void saveUsage() {

  savedData.magic = LOG_MAGIC;

  savedData.waterUsedToday = waterUsedToday;

  savedData.pumpTodaySeconds =
    pumpRunTodayMs / 1000UL;

  savedData.yesterdayWater = yesterdayWater;

  savedData.yesterdayPumpSeconds =
    yesterdayPumpSeconds;

  savedData.completedDays = completedDays;

  EEPROM.put(LOG_ADDR, savedData);

  logDirty = false;
  lastSave = millis();
}

void loadUsage() {

  EEPROM.get(LOG_ADDR, savedData);

  bool valid =
    savedData.magic == LOG_MAGIC &&
    savedData.waterUsedToday >= 0 &&
    savedData.waterUsedToday < 1000000.0 &&
    savedData.pumpTodaySeconds < 31536000UL &&
    savedData.yesterdayWater >= 0 &&
    savedData.yesterdayWater < 1000000.0 &&
    savedData.yesterdayPumpSeconds < 31536000UL;

  if (!valid) {

    waterUsedToday = 0;
    pumpRunTodayMs = 0;

    yesterdayWater = 0;
    yesterdayPumpSeconds = 0;

    completedDays = 0;

    saveUsage();
  }
  else {

    waterUsedToday = savedData.waterUsedToday;

    pumpRunTodayMs =
      savedData.pumpTodaySeconds * 1000UL;

    yesterdayWater = savedData.yesterdayWater;

    yesterdayPumpSeconds =
      savedData.yesterdayPumpSeconds;

    completedDays = savedData.completedDays;
  }
}

// =============== WATER CONSUMPTION ===============

void trackWaterUsage() {

  if (referenceLevel < 0) {
    referenceLevel = waterLevel;
    return;
  }

  if (waterLevel >= referenceLevel) {

    referenceLevel = waterLevel;
  }
  else if (referenceLevel - waterLevel >= USE_DEADBAND) {

    int difference = referenceLevel - waterLevel;

    float consumed =
      difference * TANK_CAPACITY_L / 100.0;

    waterUsedToday += consumed;

    referenceLevel = waterLevel;

    logDirty = true;
  }
}

// =============== ENERGY CALCULATION ===============

float calculateEnergy() {

  float pumpHours =
    pumpRunTodayMs / 3600000.0;

  return pumpHours * PUMP_POWER_W / 1000.0;
}

float calculateCost() {

  return calculateEnergy() * ELECTRICITY_RATE;
}

// =============== TIME DISPLAY ===============

void printDurationLCD(unsigned long seconds) {

  unsigned long hours = seconds / 3600UL;
  unsigned long minutes = (seconds % 3600UL) / 60UL;
  unsigned long remaining = seconds % 60UL;

  if (hours < 10) lcd.print('0');
  lcd.print(hours);
  lcd.print(':');

  if (minutes < 10) lcd.print('0');
  lcd.print(minutes);
  lcd.print(':');

  if (remaining < 10) lcd.print('0');
  lcd.print(remaining);
}

void printDurationSerial(unsigned long seconds) {

  unsigned long hours = seconds / 3600UL;
  unsigned long minutes = (seconds % 3600UL) / 60UL;
  unsigned long remaining = seconds % 60UL;

  if (hours < 10) Serial.print('0');
  Serial.print(hours);
  Serial.print(':');

  if (minutes < 10) Serial.print('0');
  Serial.print(minutes);
  Serial.print(':');

  if (remaining < 10) Serial.print('0');
  Serial.print(remaining);
}

// =============== LCD ROW CONTROL ===============

void clearLCDRow(int row) {

  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
}

// =============== LCD STATISTICS ===============

void showSummary() {

  unsigned long elapsed =
    millis() - summaryStart;

  byte page = elapsed / PAGE_INTERVAL_MS;

  if (page >= 6) {
    summaryActive = false;
    return;
  }

  clearLCDRow(0);
  clearLCDRow(1);

  switch (page) {

    case 0:

      lcd.print("USED TODAY");

      lcd.setCursor(0, 1);
      lcd.print(waterUsedToday, 1);
      lcd.print(" Litres");

      break;

    case 1:

      lcd.print("PUMP RUN TODAY");

      lcd.setCursor(0, 1);
      printDurationLCD(pumpRunTodayMs / 1000UL);

      break;

    case 2:

      lcd.print("ENERGY TODAY");

      lcd.setCursor(0, 1);
      lcd.print(calculateEnergy(), 3);
      lcd.print(" kWh");

      break;

    case 3:

      lcd.print("ELECTRICITY COST");

      lcd.setCursor(0, 1);
      lcd.print("Rs ");
      lcd.print(calculateCost(), 2);

      break;

    case 4:

      lcd.print("YESTERDAY WATER");

      lcd.setCursor(0, 1);

      if (completedDays == 0) {
        lcd.print("No previous data");
      }
      else {
        lcd.print(yesterdayWater, 1);
        lcd.print(" Litres");
      }

      break;

    case 5:

      lcd.print("YDAY PUMP TIME");

      lcd.setCursor(0, 1);

      if (completedDays == 0) {
        lcd.print("No previous data");
      }
      else {
        printDurationLCD(yesterdayPumpSeconds);
      }

      break;
  }
}

// =============== NORMAL LCD DISPLAY ===============

void updateLCD() {

  if (summaryActive) {

    showSummary();

    if (summaryActive) {
      return;
    }
  }

  clearLCDRow(0);

  if (!sensorOk) {

    lcd.print("SENSOR ERROR");

    clearLCDRow(1);
    lcd.print("PUMP: OFF");

    return;
  }

  lcd.print("LEVEL:");
  lcd.print(waterLevel);
  lcd.print("% ");

  if (waterLevel < LOW_LEVEL) {
    lcd.print("LOW");
  }
  else if (waterLevel < FULL_LEVEL) {
    lcd.print("MEDIUM");
  }
  else {
    lcd.print("FULL");
  }

  clearLCDRow(1);

  if (millis() < messageUntil) {

    lcd.print("TANK SET ");
    lcd.print(tankEmptyCm);
    lcd.print("cm");
  }
  else {

    lcd.print("PUMP:");

    if (pumpOn) {
      lcd.print("ON ");
    }
    else {
      lcd.print("OFF");
    }

    lcd.print("AUTO");
  }
}

// =============== WATER LEVEL CONTROL ===============

void measureAndControl() {

  long measuredDistance = readDistanceCm();

  if (measuredDistance < 2 || measuredDistance > 400) {

    if (badReads < 3) {
      badReads++;
    }
  }
  else {

    distanceCm = measuredDistance;
    badReads = 0;
  }

  sensorOk = badReads < 3;

  if (!sensorOk) {

    setPump(false);

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);

    updateLCD();
    return;
  }

  long percentage =
    ((long)tankEmptyCm - distanceCm) * 100L /
    tankEmptyCm;

  if (percentage < 0) percentage = 0;
  if (percentage > 100) percentage = 100;

  waterLevel = (int)percentage;

  trackWaterUsage();

  // Automatic pump control.

  if (waterLevel < LOW_LEVEL) {
    setPump(true);
  }
  else if (waterLevel >= STOP_LEVEL) {
    setPump(false);
  }

  // LED indicators.

  digitalWrite(
    GREEN_LED,
    waterLevel < LOW_LEVEL ? HIGH : LOW
  );

  digitalWrite(
    YELLOW_LED,
    waterLevel >= LOW_LEVEL &&
    waterLevel < FULL_LEVEL ? HIGH : LOW
  );

  digitalWrite(
    RED_LED,
    waterLevel >= FULL_LEVEL ? HIGH : LOW
  );

  updateLCD();
}

// =============== BUTTON ACTION ===============

void calibrateTank() {

  if (
    sensorOk &&
    distanceCm >= 5 &&
    distanceCm <= 250
  ) {

    tankEmptyCm = (int)distanceCm;

    EEPROM.update(0, (byte)tankEmptyCm);

    referenceLevel = -1;

    messageUntil = millis() + 1500UL;

    Serial.print("Tank calibrated: ");
    Serial.print(tankEmptyCm);
    Serial.println(" cm");
  }
}

// =============== BUTTON CONTROL ===============

void handleButton() {

  unsigned long now = millis();

  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonReading) {
    lastDebounceTime = now;
  }

  if (now - lastDebounceTime > 50UL) {

    if (reading != stableButtonState) {

      stableButtonState = reading;

      if (stableButtonState == LOW) {

        buttonPressed = true;

        buttonPressStarted = now;

        longPressHandled = false;
      }
      else if (buttonPressed) {

        buttonPressed = false;

        if (!longPressHandled) {
          calibrateTank();
        }
      }
    }
  }

  lastButtonReading = reading;

  // Hold for one second to display statistics.

  if (
    buttonPressed &&
    !longPressHandled &&
    now - buttonPressStarted >= LONG_PRESS_MS
  ) {

    longPressHandled = true;

    summaryActive = true;

    summaryStart = now;
  }
}

// =============== PUMP RUNTIME ===============

void updatePumpRuntime() {

  unsigned long now = millis();

  unsigned long elapsed =
    now - lastRuntimeUpdate;

  lastRuntimeUpdate = now;

  if (pumpOn) {

    pumpRunTodayMs += elapsed;

    logDirty = true;
  }
}

// =============== DAILY HISTORY ===============

void rolloverDay() {

  yesterdayWater = waterUsedToday;

  yesterdayPumpSeconds =
    pumpRunTodayMs / 1000UL;

  completedDays++;

  waterUsedToday = 0;
  pumpRunTodayMs = 0;

  referenceLevel = -1;

  logDirty = true;

  saveUsage();

  Serial.println("NEW DEMO DAY");
  Serial.println("Yesterday saved.");
}

// =============== SERIAL REPORT ===============

void printUsageReport() {

  Serial.println();
  Serial.println("======= USAGE REPORT =======");

  Serial.print("Water Used: ");
  Serial.print(waterUsedToday, 2);
  Serial.println(" Litres");

  Serial.print("Pump Runtime: ");
  printDurationSerial(pumpRunTodayMs / 1000UL);
  Serial.println();

  Serial.print("Energy Used: ");
  Serial.print(calculateEnergy(), 4);
  Serial.println(" kWh");

  Serial.print("Electricity Cost: Rs ");
  Serial.println(calculateCost(), 2);

  Serial.println("----------------------------");

  if (completedDays > 0) {

    Serial.print("Yesterday Water: ");
    Serial.print(yesterdayWater, 2);
    Serial.println(" Litres");

    Serial.print("Yesterday Pump Time: ");
    printDurationSerial(yesterdayPumpSeconds);
    Serial.println();
  }
  else {

    Serial.println("Yesterday: No data yet");
  }

  Serial.println("============================");
}

// =============== BUZZER ALERTS ===============

void updateBuzzer() {

  unsigned long now = millis();

  if (!sensorOk || waterLevel >= FULL_LEVEL) {

    tone(BUZZER_PIN, 1000);
  }
  else if (waterLevel < LOW_LEVEL) {

    if ((now % 1000UL) < 150UL) {
      tone(BUZZER_PIN, 500);
    }
    else {
      noTone(BUZZER_PIN);
    }
  }
  else {

    noTone(BUZZER_PIN);
  }
}

// =============== SETUP ===============

void setup() {

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  setPump(false);

  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SMART WATER TANK");

  lcd.setCursor(0, 1);
  lcd.print("SYSTEM STARTING");

  // Load saved tank height.

  int savedDistance = EEPROM.read(0);

  if (savedDistance >= 5 && savedDistance <= 250) {
    tankEmptyCm = savedDistance;
  }

  // Load saved usage history.

  loadUsage();

  lastRuntimeUpdate = millis();
  lastMeasure = millis();
  lastReport = millis();
  lastSave = millis();

  dayStart = millis();

  Serial.println("SMART WATER TANK SYSTEM");
  Serial.println("AUTOMATIC MODE");
  Serial.println("USAGE TRACKING ENABLED");

  delay(1500);

  lcd.clear();
}

// =============== MAIN LOOP ===============

void loop() {

  unsigned long now = millis();

  updatePumpRuntime();

  handleButton();

  // Two-minute demonstration day.

  if (now - dayStart >= DAY_LENGTH_MS) {

    dayStart = now;

    rolloverDay();
  }

  // Measure water level every 500 ms.

  if (now - lastMeasure >= MEASURE_INTERVAL) {

    lastMeasure = now;

    measureAndControl();
  }

  // Save when the pump is stopped.

  if (
    !pumpOn &&
    logDirty &&
    now - lastSave >= 1000UL
  ) {

    saveUsage();
  }

  // Periodic EEPROM backup.

  if (
    logDirty &&
    now - lastSave >= SAVE_INTERVAL
  ) {

    saveUsage();
  }

  // Print statistics every 10 seconds.

  if (now - lastReport >= REPORT_INTERVAL) {

    lastReport = now;

    printUsageReport();
  }

  updateBuzzer();
}
