

## How the System Works

The system has four main parts:

1. AMG8833 thermal sensors measure the temperature of the appliance.
2. The ESP32 receives the sensor readings and sends telemetry through Wi-Fi using WebSocket communication.
3. The web dashboard receives and displays the live data.
4. Firebase provides administrator authentication, database storage, event history, and synchronization.

The data flow is:

```text
AMG8833 Sensor
      |
      v
ESP32 Microcontroller
      |
      | Wi-Fi / WebSocket
      v
Web Dashboard <----> Firebase Authentication and Realtime Database
      |
      v
Alerts, Thermal Map, Logs, and Reports
```

## Dashboard Explanation

After signing in with an administrator account, the dashboard displays the connected sensor cards and the current appliance temperature.

The main temperature display shows the live reading of the selected sensor. The status changes between Normal, High Heat, and Critical Breach depending on the configured safe limit.

The administrator can change the safe temperature limit for each sensor. The new limit is sent to the ESP32 so that the hardware and dashboard use the same threshold.

The dashboard also supports Celsius and Fahrenheit display units, light and dark themes, event history, CSV export, and printing.

## Alert System

When the temperature approaches the safe limit, the system displays a warning status. When the temperature reaches or exceeds the limit, the system displays a critical alert and activates the audible alarm.

The ESP32 also controls physical hardware indicators such as the red LED, green LED, buzzer, OLED display, and SMS alert module. This means the system can provide both software and hardware notifications.

## Thermal Mapping

The Thermal Mapping page displays an 8 by 8 grid based on the AMG8833 thermal sensor data. Each cell represents a temperature value, and its color indicates the heat intensity.

The page also displays the average, maximum, and minimum temperature. Map snapshots can be recorded for later review.

## Firebase and Security

Firebase Authentication protects the dashboard so that only authorized administrators can access it.

Firebase Realtime Database stores sensor information, temperature readings, sensor names, and event history. This allows the data to be synchronized and preserved beyond a single browser session.

The dashboard must be served over HTTP because Firebase uses JavaScript modules. The administrator also needs to enable the Email/Password sign-in provider and create an account in Firebase Authentication.

## Demonstration Script

1. Open the dashboard using the local server URL.
2. Sign in using the administrator account.
3. Show the connected sensor cards and select a sensor.
4. Explain the live temperature, status, safe limit, and connection indicators.
5. Change the safe limit and explain that it is synchronized with the ESP32.
6. Demonstrate the Celsius/Fahrenheit unit toggle.
7. Open the Thermal Map page and explain the 8 by 8 heat distribution.
8. Return to the dashboard and show the event history.
9. Demonstrate CSV export or printing.
10. If possible, increase the sensor temperature or use a test value to demonstrate the warning and critical alert behavior.

## Closing Statement

In conclusion, ThermE.Y.E. combines thermal sensing, ESP32 hardware, WebSocket communication, Firebase services, and a responsive web dashboard into one monitoring system.

It provides real-time temperature visibility, configurable safety limits, automatic alerts, thermal visualization, and historical records. Through this system, appliance overheating can be detected earlier and monitored more efficiently.

Thank you. We are ready for your questions.

## Short Version

ThermE.Y.E. is a web-based appliance temperature monitoring system. AMG8833 sensors measure the appliance temperature, while the ESP32 processes the readings and sends them to the dashboard through Wi-Fi and WebSockets. Firebase handles authentication, data storage, and event synchronization.

The dashboard shows live temperatures, safe-limit status, alerts, thermal maps, and history logs. When a temperature reaches the configured limit, the system activates visual and audible warnings and can notify the user through the ESP32 hardware. This helps detect overheating early and improves appliance safety.
