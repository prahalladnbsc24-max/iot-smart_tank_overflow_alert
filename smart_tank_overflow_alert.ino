/*
  Smart Water Tank Overflow Alert (Tinkercad simulation)
  -------------------------------------------------------
  A potentiometer on A0 stands in for the water-level sensor.
  Level (0-100 %) is shown on three LEDs, a buzzer warns when the
  tank is nearly full, and a pump (DC motor via TIP120) runs until
  the tank is almost full. A push button toggles a manual pump
  override. Status is printed to the Serial Monitor.

  Pins
    D2  green LED   (LOW)      D8  buzzer
    D3  yellow LED  (MEDIUM)   D9  pump driver (TIP120 base)
    D4  red LED     (FULL)     D7  push button (to GND, internal pull-up)
    A0  potentiometer wiper (water level)
*/

const int ledLow = 2, ledMed = 3, ledFull = 4;
const int buzzer = 8, pumpPin = 9, btn = 7;
bool pumpEnabled = true;

void setup() {
  pinMode(ledLow, OUTPUT);
  pinMode(ledMed, OUTPUT);
  pinMode(ledFull, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(btn, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  if (digitalRead(btn) == LOW) {          // toggle pump override
    pumpEnabled = !pumpEnabled;
    delay(300);
  }

  int level = map(analogRead(A0), 0, 1023, 0, 100);
  String status;

  digitalWrite(ledLow, level < 40);
  digitalWrite(ledMed, level >= 40 && level < 80);
  digitalWrite(ledFull, level >= 80);

  if (level < 40)       status = "LOW";
  else if (level < 80)  status = "MEDIUM";
  else                  status = "FULL";

  // Pump runs until the tank is nearly full
  bool pumpOn = pumpEnabled && level < 90;
  digitalWrite(pumpPin, pumpOn ? HIGH : LOW);

  // Alert when nearly full
  if (level >= 80) tone(buzzer, 1000);
  else noTone(buzzer);

  Serial.println("Water Level: " + String(level) + "% | " + status +
                 " | Pump: " + (pumpOn ? "ON" : "OFF"));
  delay(500);
}
