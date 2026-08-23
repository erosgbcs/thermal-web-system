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
- (Optional) Node.js if you want to serve the files locally with a simple HTTP server.

---

## Installation / Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/your-org/thermal-web-system.git
   cd thermal-web-system
```

2. Configure Firebase
   · Replace the firebaseConfig object inside the <script type="module"> tag in index.html with your own Firebase project credentials.
   · Ensure your Realtime Database rules allow read/write for authenticated users:
     ```json
     {
       "rules": {
         ".read": "auth != null",
         ".write": "auth != null"
       }
     }
     ```
3. Run the application
  · Serve the project over HTTP so Firebase ES modules and admin login can initialize correctly:
     ```bash
     npx serve .
     ```
     Then visit http://localhost:3000.
4. Connect an ESP32
   · Enter the ESP32’s IP address (e.g., 192.168.1.50) in the dashboard’s ESP32 Host / IP field.
   · The connection will be automatically established via WebSocket on port 81.

---

Usage

1. Sign In – Use your admin email and password (created via Firebase Authentication).
2. Select a Sensor – Click on any sensor card in the top row to view its live data.
3. Set Safe Limits – Adjust the threshold slider; the value is immediately synced to the connected ESP32.
4. Monitor – Watch the hero display and the thermal map (available via the “Thermal Map” button).
5. Logs & Export – View event history, export as CSV, or print directly.

---

Thermal Mapping Page

The mapping page renders an 8×8 grid that simulates an AMG8833 heat distribution based on the real‑time temperature of the selected sensor.

· Each cell’s colour indicates temperature intensity.
· Statistics (avg, max, min, hotspots) are displayed on the side.
· History of map snapshots is kept locally.

---

Project Structure


thermal-web-system/
├── index.html          # Main dashboard (all HTML, CSS, JS inline)
├── thermeye_logo.png   # App logo
├── ffhnas_logo.png     # School logo
└── README.md           # This file


Note: The entire application is self‑contained in a single HTML file for simplicity.

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

· The dashboard must remain open and connected to receive live data from the ESP32.
· A data watchdog automatically zeroes out sensor readings if no data is received for 10 seconds.
· Firewall or network restrictions may block WebSocket connections; ensure port 81 is open.
· Firebase free tier limits apply – monitor your database usage.

---

ThermE.Y.E. – Keeping appliances cool and safe.

```
