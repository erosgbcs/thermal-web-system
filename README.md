# ThermE.Y.E. – Appliances Thermal Monitoring System

A real‑time web dashboard for monitoring appliance temperatures using AMG8833 thermal sensors, ESP32 microcontrollers, and Firebase.  
Designed for **Fortunato F. Halili Agricultural National High School** to help prevent overheating in electrical appliances.

---

## Features

- **Live Temperature Monitoring** – View current temperature, historical logs, and trend data for multiple sensors.
- **Multi‑Sensor Support** – Monitor up to several appliances simultaneously, each with its own safe threshold.
- **Configurable Safe Limits** – Set temperature limits per sensor; the system triggers audible/visual alerts when exceeded.
- **Thermal Mapping** – 8×8 heatmap visualisation generated from sensor data (AMG8833 IR array emulation).
- **Firebase Backend** – Real‑time database for telemetry, event logging, and sensor naming synchronisation.
- **ESP32 WebSocket Integration** – Direct two‑way communication with ESP32 devices (live temp, threshold sync, unit settings).
- **Light/Dark Theme** – Toggleable UI theme with persistent user preference.
- **Export & Print** – Download event logs as CSV or print directly.
- **Secure Admin Auth** – Firebase Authentication (email/password) with password reset.
- **Shared Simulation Mode** – Use the school logo to open simulation mode and send adjustable test readings to other authenticated devices.

---

## Technology Stack

| Layer          | Technology |
|----------------|------------|
| Frontend       | HTML5, CSS3, Vanilla JavaScript (ES6+) |
| Backend / DB   | Firebase Realtime Database, Firebase Auth |
| Hardware       | ESP32, AMG8833 thermal sensor |
| Communication  | WebSockets (ESP32 ↔ Browser) |
| Styling        | CSS Custom Properties (theming), Responsive Grid |

---

## Prerequisites

- A modern web browser (Chrome, Edge, Firefox)
- A Firebase project with:
  - Realtime Database enabled
  - Authentication (Email/Password) enabled
- ESP32 device(s) flashed with firmware that:
  - Streams temperature data over WebSocket (port 81)
  - Accepts commands for `safeLimit`, unit change, etc.
- Node.js and npm for running the local development server.

---

## Installation / Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/your-org/thermal-web-system.git
   cd thermal-web-system
```

2. Configure Firebase
  - Replace the `firebaseConfig` object in `firebase.js` with your Firebase project credentials.
  - Enable **Authentication > Sign-in method > Email/Password**.
  - Create an administrator account in Firebase Authentication.
  - Ensure your Realtime Database rules allow read/write for authenticated users:
     ```json
     {
       "rules": {
         ".read": "auth != null",
         ".write": "auth != null"
       }
     }
     ```
3. Run the application
  - Serve the project over HTTP so Firebase ES modules and admin login can initialize correctly:
     ```bash
     npx serve .
     ```
  - Open http://localhost:3000 in your browser. Do not open `index.html` directly with `file://`.
4. Connect an ESP32
  - Enter the ESP32’s IP address (for example, `192.168.1.50`) in the dashboard’s ESP32 Host / IP field.
  - The connection will be automatically established via WebSocket on port 81.

---

Usage

1. Sign In - Use your administrator email and password created in Firebase Authentication.
2. Select a Sensor - Click a sensor card to view its live data.
3. Set Safe Limits - Adjust the safe temperature limit; it is synced to the connected ESP32.
4. Monitor - View the live temperature, status, alerts, and thermal map.
5. Logs & Export - View event history, export it as CSV, or print it directly.

### Shared Simulation Mode

After signing in, click the school logo at the top of the dashboard to open Simulation Mode. Adjust the temperature between 10°C and 100°C and select **Apply Test Reading**. The **Start Random Simulation** button generates a new random reading in that range every two seconds. The reading is stored at `simulation/current` in Firebase and appears on other authenticated devices that have Simulation Mode open for the same sensor.

All devices must use the same Firebase project, be signed in, and have database rules that allow authenticated users to read and write the `simulation` path.

---

Thermal Mapping Page

The mapping page renders an 8×8 grid that simulates an AMG8833 heat distribution based on the real‑time temperature of the selected sensor.

- Each cell’s colour indicates temperature intensity.
- Statistics such as average, maximum, and minimum temperature are displayed beside the map.
- Map snapshots are kept locally in the browser.

---

Project Structure


thermal-web-system/
├── index.html          # Dashboard markup
├── styles.css          # Dashboard styles and themes
├── script.js           # Dashboard state, UI, maps, and ESP32 WebSockets
├── firebase.js         # Firebase Authentication and Realtime Database integration
├── esp32.ino           # ESP32 firmware for sensors and hardware alarms
├── thermeye_logo.png   # App logo
├── ffhnas_logo.png     # School logo
└── README.md           # This file

The web dashboard uses inline SVG icons and is split into separate HTML, CSS, and JavaScript files.

---

Sponsored by;

· Giero E. Delos Santos
· Gabriel P. Morallos
· Romelie Z. Ocuaman
· Jahyra Marie F. Salmeron
· Jericho Salas

School: Fortunato F. Halili Agricultural National High School

---

License

This project is provided for educational purposes. Contact the developers for usage permissions.

---

Important Notes

- The dashboard must remain open and connected to receive live data from the ESP32.
- A data watchdog automatically zeroes out sensor readings if no data is received for 10 seconds.
- Firewall or network restrictions may block WebSocket connections; ensure port 81 is open.
- Firebase free tier limits apply; monitor your database usage.

---

ThermE.Y.E. – Keeping appliances cool and safe.

```
