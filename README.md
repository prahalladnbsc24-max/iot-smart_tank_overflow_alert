# IoT Smart Water Tank Overflow Alert

An Arduino-based smart water tank monitoring and overflow prevention system that automatically monitors water levels, controls a water pump, and provides alerts when the tank is nearly full. The upgraded design also estimates water consumption, pump running time, energy usage, and electricity costs.

The project is developed and simulated using **Tinkercad Circuits** with an Arduino UNO R3.

## 1. Project Overview

Traditional overhead water tanks often require manual monitoring. If the pump is not switched off at the correct time, water can overflow, wasting water and electricity.

The Smart Water Tank Overflow Alert system addresses this problem by continuously measuring the distance between the ultrasonic sensor and the water surface. It calculates the tank's water level and automatically controls the pump.

The upgraded system also provides water usage and estimated energy consumption information through an LCD and Serial Monitor.

## 2. Features

- **Automatic Water-Level Monitoring:** Measures the water level using an HC-SR04 ultrasonic distance sensor.
- **Three-Level LED Indicators:** Green indicates LOW, yellow indicates MEDIUM, and red indicates FULL.
- **Automatic Pump Control:** Starts the pump when the water level is below 40% and stops it when the level reaches 90%.
- **Overflow Alert:** Activates a buzzer when the water level reaches 80% or above.
- **LCD Display:** Shows the current water level, pump status, and usage statistics.
- **Water Consumption Tracking:** Estimates the amount of water used in litres based on changes in tank level.
- **Pump Runtime Monitoring:** Calculates the total time the pump remains ON.
- **Energy Consumption Estimation:** Calculates energy consumption in kilowatt-hours (kWh).
- **Electricity Cost Estimation:** Calculates the estimated electricity expense.
- **EEPROM Data Storage:** Saves usage information and previous-day statistics.
- **Statistics Display:** Shows usage information through multiple LCD pages.

## 3. Components Required

| Component | Quantity | Purpose |
|---|---:|---|
| Arduino UNO R3 | 1 | Main controller |
| HC-SR04 Ultrasonic Sensor | 1 | Water-level measurement |
| I2C LCD 16×2 | 1 | Displays water level and statistics |
| Green LED | 1 | LOW-level indicator |
| Yellow LED | 1 | MEDIUM-level indicator |
| Red LED | 1 | FULL-level indicator |
| 220 Ω Resistors | 3 | LED current limiting |
| Piezo Buzzer | 1 | Audible alert |
| Push Button | 1 | Tank calibration and statistics display |
| DC Motor | 1 | Simulated water pump |
| TIP120 Transistor | 1 | Pump switching |
| 1 kΩ Resistor | 1 | Transistor base resistor |
| Diode | 1 | Flyback protection |
| Breadboard and Jumper Wires | As required | Circuit connections |

## 4. Circuit Connections

| Arduino Pin | Component | Connection |
|---|---|---|
| D2 | Green LED | LOW-level indicator |
| D3 | Yellow LED | MEDIUM-level indicator |
| D4 | Red LED | FULL-level indicator |
| D5 | HC-SR04 | TRIG |
| D6 | HC-SR04 | ECHO |
| D7 | Push Button | Other terminal connected to GND |
| D8 | Piezo Buzzer | Positive terminal |
| D9 | TIP120 | Base through 1 kΩ resistor |
| A4 | I2C LCD | SDA |
| A5 | I2C LCD | SCL |
| 5V | Components | Positive power supply |
| GND | Components | Common ground |

### Additional Connections

- HC-SR04 VCC connects to 5V and GND connects to GND.
- LCD VCC connects to 5V and GND connects to GND.
- LCD SDA connects to A4 and SCL connects to A5.
- LED cathodes connect to GND through the appropriate circuit arrangement.
- TIP120 emitter connects to GND.
- TIP120 collector connects to the motor's negative terminal.
- Motor positive terminal connects to 5V.
- The diode is connected across the motor, with its striped end toward 5V.

**Operating Mode:** The upgraded design uses automatic operation without a separate slide switch.

## 5. Water-Level Detection and Pump Control

The ultrasonic sensor measures the distance between the sensor and the water surface.

The water-level percentage is calculated using:

Water Level (%) = ((Empty Tank Distance - Measured Distance) / Empty Tank Distance) × 100

The calculated percentage is limited to the range of 0–100%.

| Water Level | LED | Buzzer | Pump |
|---|---|---|---|
| 0–39% | Green | Short alerts | ON |
| 40–79% | Yellow | OFF | Maintains state |
| 80–89% | Red | Continuous alert | Maintains state |
| 90–100% | Red | Continuous alert | OFF |

## 6. Water Consumption and Energy Monitoring

The upgraded design includes calculations to monitor water consumption and estimated electricity usage.

### 6.1 Water Consumption

Water consumption is estimated from decreases in the measured tank level.

For the demonstration, the configured tank capacity is 500 litres.

Water Used (Litres) = Percentage Decrease × Tank Capacity / 100

The software uses a 2% deadband to reduce small fluctuations caused by sensor readings.

### 6.2 Pump Running Time

The Arduino tracks how long the pump remains switched ON.

Pump Runtime = Total accumulated pump operating time.

The result is displayed in hours, minutes, and seconds.

### 6.3 Energy Consumption

The system estimates energy consumption using the configured pump power and operating time.

Energy (kWh) = Pump Power (W) × Pump Operating Time (Hours) / 1000

Demonstration pump power: 750 W.

### 6.4 Electricity Cost

Estimated Electricity Cost = Energy Consumed (kWh) × Electricity Rate

Demonstration electricity rate: ₹8 per kWh.

**Note:** Water usage and electricity consumption are software estimates. The prototype does not use a physical flow meter or electrical energy meter.

## 7. Data Storage and LCD Statistics

EEPROM is used to retain usage information between simulation sessions, where supported.

The stored information includes:

- Water consumed during the current demonstration day.
- Pump operating time.
- Previous-day water consumption.
- Previous-day pump operating time.
- Number of completed demonstration days.
- Calibrated empty-tank distance.

The LCD displays six statistics pages:

1. Water consumed today (litres).
2. Pump running time.
3. Energy consumed (kWh).
4. Estimated electricity cost (₹).
5. Previous-day water consumption.
6. Previous-day pump running time.

**Button Operations:**

- Short press: Calibrates the empty-tank distance.
- Long press (1 second): Opens the LCD usage statistics.

## 8. Demonstration Configuration

The following values are used for the project demonstration and can be adjusted in the Arduino program.

| Parameter | Configured Value |
|---|---:|
| Tank Capacity | 500 litres |
| Pump Power | 750 W |
| Electricity Rate | ₹8/kWh |
| Water-Level Measurement Interval | 500 ms |
| Water Usage Deadband | 2% |
| Demonstration Day Length | 2 minutes |

The two-minute demonstration day allows previous-day statistics to be demonstrated quickly. For a real 24-hour period, the day-length setting must be changed to 86,400,000 milliseconds.

## 9. Example Usage Report

The following figures are illustrative examples to demonstrate the reporting format, not actual measured simulation results.

| Parameter | Example Value |
|---|---:|
| Water Used | 300 litres |
| Pump Running Time | 47 minutes |
| Pump Power | 750 W |
| Energy Used | 0.5875 kWh |
| Electricity Rate | ₹8/kWh |
| Estimated Electricity Cost | ₹4.70 |

## 10. Testing Procedure

1. Open the project in Tinkercad Circuits.
2. Select the Arduino UNO R3 and open the Code panel.
3. Insert the upgraded Arduino sketch in Text mode.
4. Start the simulation.
5. Adjust the ultrasonic sensor distance to simulate different water levels.
6. Observe the green, yellow, and red LED indicators.
7. Verify that the pump starts at low water levels and stops at 90%.
8. Observe the buzzer when the water level reaches 80%.
9. Check the current water level and pump status on the LCD.
10. Hold the push button for one second to view the usage statistics.
11. Open the Serial Monitor at 9600 baud to view the detailed usage report.

**Implementation status:** The upgraded firmware and its additional statistics are undergoing Tinkercad compilation and simulation verification. The reported calculations should be treated as estimates until the complete upgraded sketch has been successfully tested.

## 11. Advantages

- Prevents water overflow through automatic pump control.
- Reduces unnecessary pump operation.
- Provides continuous water-level information.
- Estimates household water consumption.
- Estimates pump energy usage and electricity costs.
- Stores previous usage information.
- Provides audible and visual alerts.
- Can be extended into a Wi-Fi-enabled IoT monitoring system.

## 12. Future Scope

- Replace Arduino UNO with ESP8266 or ESP32 for Wi-Fi connectivity.
- Develop a web or mobile dashboard for remote monitoring.
- Add a physical water-flow sensor for more accurate consumption measurements.
- Use an energy-monitoring sensor for direct electricity measurements.
- Implement cloud-based storage and historical usage graphs.
- Add mobile notifications when the tank reaches critical levels.
- Extend the system to monitor multiple water tanks.

## 13. GitHub Repository

**Project Repository:**  
https://github.com/prahalladnbsc24-max/iot-smart_tank_overflow_alert

The repository is intended to contain the Arduino source code, circuit diagram, project documentation, and implementation details.

## 14. Conclusion

The Smart Water Tank Overflow Alert project demonstrates how sensors, microcontrollers, displays, and actuators can be combined to create an automated water-management system.

The upgraded design extends basic water-level monitoring with water-consumption tracking, pump runtime monitoring, estimated energy usage, electricity-cost calculations, and EEPROM-based usage history.

The project provides a foundation for a more advanced IoT-based water-management solution that can help reduce water wastage and improve household resource monitoring.
