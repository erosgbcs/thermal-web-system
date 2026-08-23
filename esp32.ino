#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AMG88xx.h>
#include <esp_sleep.h>
#include <Preferences.h>

// WiFi Configuration
const char* WIFI_SSID = "Abdc_NetPro15-2.4G";
const char* WIFI_PASS = "MatheaZzy25";

// SIM800L Configuration
const char* SMS_TARGET = "+639551621325";
const int SIM800_RX = 16;
const int SIM800_TX = 17;
HardwareSerial sim800(2);

// SMS Tracking variables
bool smsSent = false;
unsigned long lastSmsTime = 0;
const unsigned long SMS_COOLDOWN_MS = 30000;

// Hardware Pin Configuration
const int RED_LED_PIN = 23;
const int GREEN_LED_PIN = 19;
const int BUZZER_PIN = 18;
const int BUTTON_PIN = 32;

// Button & Alarm variables
unsigned long buttonPressTime = 0;
bool buttonPressed = false;
bool alarmMuted = false;

// OLED Configuration (128x64 I2C)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Two AMG8833 sensors (addresses 0x69 and 0x68)
Adafruit_AMG88xx amg1;
Adafruit_AMG88xx amg2;
float amgPixels1[64];
float amgPixels2[64];
float amgMaxTemp1 = -999.0;
float amgMaxTemp2 = -999.0;
bool amg1Connected = false;
bool amg2Connected = false;
float amgCalibrationOffset = 0.0;

// Dynamic Sensor Names
String sensor1Name = "AMG1";
String sensor2Name = "AMG2";

// Dynamic Critical Breach Limit (Celsius)
float criticalThreshold = 50.0;
bool isBreached = false;
bool simulationMode = false;
bool simulationBuzzerTest = false;
float simulationTemp1 = -999.0;
float simulationTemp2 = -999.0;

// Unit Toggle State
bool isFahrenheit = false;

// WebSocket Server
WebSocketsServer webSocket = WebSocketsServer(81);

// Optimized Intervals
unsigned long lastSend = 0;
unsigned long lastOLEDUpdate = 0;
unsigned long lastSensorRead = 0;

const unsigned long SENSOR_INTERVAL_MS = 200;
const unsigned long TELEMETRY_INTERVAL_MS = 200;
const unsigned long OLED_INTERVAL_MS = 200;

// Function Declarations
void readAllSensors();
void sendTelemetryData();
void checkBreachStatus();
void sendSMSAlert(float temp, String source);
void updateOLED();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
void handleButton();
void powerOffSystem();

Preferences prefs;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, 0);

  prefs.begin("thermeye", false);
  sensor1Name = prefs.getString("s1_name", "AMG1");
  sensor2Name = prefs.getString("s2_name", "AMG2");
  criticalThreshold = prefs.getFloat("threshold", 50.0);
  prefs.end();

  Wire.begin(21, 22);
  Wire.setClock(400000);

  sim800.begin(115200, SERIAL_8N1, SIM800_RX, SIM800_TX);
  delay(1000);
  sim800.println("AT+IPR=115200");
  delay(500);
  sim800.println("ATE0");
  delay(500);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED initialization failed"));
  } else {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(20, 10);

    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
      display.println("System Waking Up");
    } else {
      display.println("System Booting");
    }

    display.drawRect(14, 30, 100, 10, WHITE);
    display.display();
  }

  if (!amg1.begin(0x69)) {
    Serial.println(F("AMG8833 #1 (0x69) failed."));
    amg1Connected = false;
  } else {
    Serial.println(F("AMG8833 #1 (0x69) Initialized."));
    amg1Connected = true;
  }

  if (!amg2.begin(0x68)) {
    Serial.println(F("AMG8833 #2 (0x68) failed."));
    amg2Connected = false;
  } else {
    Serial.println(F("AMG8833 #2 (0x68) Initialized."));
    amg2Connected = true;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");

  unsigned long startAttemptTime = millis();
  int progress = 0;

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(250);
    Serial.print(".");
    progress += 5;
    if (progress > 100) progress = 100;
    display.fillRect(14, 30, progress, 10, WHITE);
    display.display();
  }

  display.clearDisplay();
  display.setCursor(10, 20);

  if (WiFi.status() == WL_CONNECTED) {
    // --- PRINT IP ADDRESS TO SERIAL MONITOR ---
    Serial.println("\n------------------------------------");
    Serial.println("WiFi Connected Successfully!");
    Serial.print("ESP32 Local IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("------------------------------------");

    // Display IP Address on OLED
    display.println("WiFi Connected!");
    display.setCursor(10, 35);
    display.print("IP: ");
    display.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed! Booting offline.");
    display.println("WiFi Timeout.");
    display.setCursor(10, 35);
    display.println("Starting Offline...");
  }

  display.display();
  delay(1500);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  sim800.println("AT");
  delay(100);
}

void loop() {
  while (sim800.available()) {
    Serial.write(sim800.read());
  }
  while (Serial.available()) {
    sim800.write(Serial.read());
  }
  webSocket.loop();

  handleButton();

  unsigned long currentMillis = millis();

  if (currentMillis - lastSensorRead >= SENSOR_INTERVAL_MS) {
    readAllSensors();
    checkBreachStatus();
    lastSensorRead = currentMillis;
  }

  if (currentMillis - lastSend >= TELEMETRY_INTERVAL_MS) {
    if (WiFi.status() == WL_CONNECTED) {
      sendTelemetryData();
    }
    lastSend = currentMillis;
  }

  if (currentMillis - lastOLEDUpdate >= OLED_INTERVAL_MS) {
    updateOLED();
    lastOLEDUpdate = currentMillis;
  }
}

void handleButton() {
  bool currentButtonState = (digitalRead(BUTTON_PIN) == LOW);

  if (currentButtonState && !buttonPressed) {
    buttonPressTime = millis();
    buttonPressed = true;
  } else if (!currentButtonState && buttonPressed) {
    unsigned long pressDuration = millis() - buttonPressTime;
    buttonPressed = false;

    if (pressDuration > 50 && pressDuration < 2000) {
      if (isBreached) {
        alarmMuted = true;
        Serial.println("Alarm silenced by user.");
      }
    }
  } else if (currentButtonState && buttonPressed) {
    if (millis() - buttonPressTime > 3000) {
      powerOffSystem();
    }
  }
}

void powerOffSystem() {
  Serial.println("Powering off...");

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(15, 25);
  display.println("POWER OFF");
  display.display();

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, HIGH);

  delay(1500);
  display.clearDisplay();
  display.display();

  while (digitalRead(BUTTON_PIN) == LOW) {
    delay(10);
  }

  esp_deep_sleep_start();
}

void readAllSensors() {
  if (amg1Connected) {
    amg1.readPixels(amgPixels1);
    amgMaxTemp1 = -999.0;
    bool allZeros = true;
    for (int i = 0; i < 64; i++) {
      if (amgPixels1[i] != 0.0) allZeros = false;
      if (amgPixels1[i] > amgMaxTemp1) amgMaxTemp1 = amgPixels1[i];
    }
    if (allZeros || amgMaxTemp1 < -40.0 || amgMaxTemp1 > 80.0) {
      amgMaxTemp1 = -999.0;
    } else {
      amgMaxTemp1 += amgCalibrationOffset;
    }
  } else {
    amgMaxTemp1 = -999.0;
  }

  if (amg2Connected) {
    amg2.readPixels(amgPixels2);
    amgMaxTemp2 = -999.0;
    bool allZeros = true;
    for (int i = 0; i < 64; i++) {
      if (amgPixels2[i] != 0.0) allZeros = false;
      if (amgPixels2[i] > amgMaxTemp2) amgMaxTemp2 = amgPixels2[i];
    }
    if (allZeros || amgMaxTemp2 < -40.0 || amgMaxTemp2 > 80.0) {
      amgMaxTemp2 = -999.0;
    } else {
      amgMaxTemp2 += amgCalibrationOffset;
    }
  } else {
    amgMaxTemp2 = -999.0;
  }
}

void checkBreachStatus() {
  bool previousBreach = isBreached;
  isBreached = false;
  float highestTemp = -999.0;
  String breachSource = "";
  float sensorTemp1 = simulationMode ? simulationTemp1 : amgMaxTemp1;
  float sensorTemp2 = simulationMode ? simulationTemp2 : amgMaxTemp2;

  if (sensorTemp1 != -999.0) {
    if (sensorTemp1 > highestTemp) {
      highestTemp = sensorTemp1;
      breachSource = sensor1Name;
    }
    if (sensorTemp1 >= criticalThreshold) {
      isBreached = true;
    }
  }

  if (sensorTemp2 != -999.0) {
    if (sensorTemp2 > highestTemp) {
      highestTemp = sensorTemp2;
      breachSource = sensor2Name;
    }
    if (sensorTemp2 >= criticalThreshold) {
      isBreached = true;
    }
  }

  if (simulationMode && simulationBuzzerTest) {
    isBreached = true;
    if (highestTemp == -999.0) {
      highestTemp = 0.0;
      breachSource = "Simulation";
    }
  }

  if (!isBreached && previousBreach) {
    alarmMuted = false;
  }

  if (isBreached) {
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);

    if (!alarmMuted) {
      digitalWrite(BUZZER_PIN, LOW);
      if (!simulationMode && (!smsSent || (millis() - lastSmsTime > SMS_COOLDOWN_MS))) {
        sendSMSAlert(highestTemp, breachSource);
        smsSent = true;
        lastSmsTime = millis();
      }
    } else {
      digitalWrite(BUZZER_PIN, HIGH);
    }
  } else {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    smsSent = false;
  }
}

void sendSMSAlert(float temp, String source) {
  Serial.println("\n--- Triggering SMS ---");
  
  sim800.println("AT+CMGF=1");
  delay(500); 
  while(sim800.available()) { Serial.write(sim800.read()); }
  
  sim800.print("AT+CMGS=\"");
  sim800.print(SMS_TARGET);
  sim800.println("\"");
  delay(500); 
  while(sim800.available()) { Serial.write(sim800.read()); }
  
  sim800.print("CRITICAL ALERT! Temperature breach detected by ");
  sim800.print(source);
  sim800.print(". Reading: ");
  sim800.print(temp, 1);
  sim800.print("C. Limit: ");
  sim800.print(criticalThreshold, 1);
  sim800.println("C.");
  delay(500); 
  
  sim800.write(26);
  
  Serial.println("\nSMS data dispatched. Waiting 5 seconds for network...");
  delay(5000); 
  
  while(sim800.available()) { Serial.write(sim800.read()); }
  
  Serial.println("\n--- SMS Routine Complete ---\n");
}

void sendTelemetryData() {
  float telemetryTemp1 = simulationMode ? simulationTemp1 : amgMaxTemp1;
  float telemetryTemp2 = simulationMode ? simulationTemp2 : amgMaxTemp2;

  if (telemetryTemp1 != -999.0) {
    StaticJsonDocument<1024> doc;
    doc["roomId"] = "room1";
    doc["sensorId"] = "AMG8833_1";

    float outTemp1 = isFahrenheit ? (telemetryTemp1 * 1.8f + 32.0f) : telemetryTemp1;
    doc["temperature"] = (float)(round(outTemp1 * 10.0) / 10.0);

    doc["isBreached"] = (telemetryTemp1 >= criticalThreshold);
    doc["limit"] = criticalThreshold;
    doc["unit"] = isFahrenheit ? "F" : "C";

    JsonArray pixels = doc.createNestedArray("pixels");
    for (int i = 0; i < 64; i++) {
      float sourcePixel = simulationMode ? telemetryTemp1 : amgPixels1[i];
      float pTemp = isFahrenheit ? (sourcePixel * 1.8f + 32.0f) : sourcePixel;
      pixels.add((float)(round(pTemp * 10.0) / 10.0));
    }

    String output;
    serializeJson(doc, output);
    webSocket.broadcastTXT(output);
  }

  if (telemetryTemp2 != -999.0) {
    StaticJsonDocument<1024> doc;
    doc["roomId"] = "room1";
    doc["sensorId"] = "AMG8833_2";

    float outTemp2 = isFahrenheit ? (telemetryTemp2 * 1.8f + 32.0f) : telemetryTemp2;
    doc["temperature"] = (float)(round(outTemp2 * 10.0) / 10.0);

    doc["isBreached"] = (telemetryTemp2 >= criticalThreshold);
    doc["limit"] = criticalThreshold;
    doc["unit"] = isFahrenheit ? "F" : "C";

    JsonArray pixels = doc.createNestedArray("pixels");
    for (int i = 0; i < 64; i++) {
      float sourcePixel = simulationMode ? telemetryTemp2 : amgPixels2[i];
      float pTemp = isFahrenheit ? (sourcePixel * 1.8f + 32.0f) : sourcePixel;
      pixels.add((float)(round(pTemp * 10.0) / 10.0));
    }

    String output;
    serializeJson(doc, output);
    webSocket.broadcastTXT(output);
  }
}

void updateOLED() {
  static bool blink = false;
  blink = !blink;

  display.clearDisplay();

  char u = isFahrenheit ? 'F' : 'C';

  float displayTemp1 = simulationMode ? simulationTemp1 : amgMaxTemp1;
  float displayTemp2 = simulationMode ? simulationTemp2 : amgMaxTemp2;
  float dAmg1 = (displayTemp1 == -999.0) ? -999.0 : (isFahrenheit ? displayTemp1 * 1.8 + 32 : displayTemp1);
  float dAmg2 = (displayTemp2 == -999.0) ? -999.0 : (isFahrenheit ? displayTemp2 * 1.8 + 32 : displayTemp2);

  // HEADER
  display.drawLine(0, 11, 127, 11, WHITE);
  display.setTextSize(1);
  display.setCursor(0, 1);
  display.print("THERMAL");

  if (WiFi.status() == WL_CONNECTED) {
    display.setCursor(80, 1);
    display.print("<WiFi>");
  } else {
    display.setCursor(104, 1);
    display.print("OFF");
  }

  // SENSOR DISPLAY
  String disp1 = sensor1Name.length() > 8 ? sensor1Name.substring(0, 8) : sensor1Name;
  display.setCursor(2, 16);
  display.print(disp1);
  display.setCursor(75, 16);
  if (dAmg1 == -999.0) display.print("--");
  else display.print(dAmg1, 1);

  String disp2 = sensor2Name.length() > 8 ? sensor2Name.substring(0, 8) : sensor2Name;
  display.setCursor(2, 28);
  display.print(disp2);
  display.setCursor(75, 28);
  if (dAmg2 == -999.0) display.print("--");
  else display.print(dAmg2, 1);

  // FOOTER
  display.drawLine(0, 52, 127, 52, WHITE);

  display.setCursor(2, 56);
  display.print("L:");
  display.print(isFahrenheit ? criticalThreshold * 1.8 + 32 : criticalThreshold, 0);
  display.print((char)247);
  display.print(u);

  if (isBreached) {
    if (blink || alarmMuted) {
      display.fillRoundRect(66, 54, 60, 10, 2, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(72, 56);
      if (alarmMuted) display.print("MUTED");
      else display.print("ALERT");
      display.setTextColor(WHITE);
    } else {
      display.drawRoundRect(66, 54, 60, 10, 2, WHITE);
      display.setCursor(72, 56);
      display.print("ALERT");
    }
  } else {
    display.drawRoundRect(70, 54, 56, 10, 2, WHITE);
    display.setCursor(84, 56);
    display.print("SAFE");
  }

  display.display();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) return;

    if (doc.containsKey("type") && doc["type"] == "updateSensorName") {
      String targetId = doc["sensorId"].as<String>();
      String newName = doc["sensorName"].as<String>();

      prefs.begin("thermeye", false);
      if (targetId == "AMG8833_1") {
        sensor1Name = newName;
        prefs.putString("s1_name", newName);
      } else if (targetId == "AMG8833_2") {
        sensor2Name = newName;
        prefs.putString("s2_name", newName);
      }
      prefs.end();
      updateOLED();
      return;
    }

    if (doc.containsKey("safeLimit")) {
      float newLimit = doc["safeLimit"].as<float>();
      if (newLimit > 0 && newLimit < 150) {
        criticalThreshold = newLimit;
        Serial.printf("Threshold updated to %.1f C\n", criticalThreshold);
        
        prefs.begin("thermeye", false);
        prefs.putFloat("threshold", criticalThreshold);
        prefs.end();
        checkBreachStatus();
      }
      return;
    }

    if (doc.containsKey("unit")) {
      String unit = doc["unit"].as<String>();
      if (unit == "F") {
        isFahrenheit = true;
      } else if (unit == "C") {
        isFahrenheit = false;
      }
      Serial.printf("Unit set to %s\n", isFahrenheit ? "F" : "C");
      updateOLED();
      return;
    }

    if (doc.containsKey("simulation")) {
      simulationMode = doc["simulation"].as<bool>();
      simulationBuzzerTest = simulationMode && doc["buzzerTest"].as<bool>();
      if (simulationMode && doc.containsKey("sensorId") && doc.containsKey("temperature")) {
        String sensorId = doc["sensorId"].as<String>();
        float simulatedTemp = doc["temperature"].as<float>();
        if (simulatedTemp >= 0.0 && simulatedTemp <= 150.0) {
          if (sensorId == "AMG8833_1") {
            simulationTemp1 = simulatedTemp;
          }
          if (sensorId == "AMG8833_2") {
            simulationTemp2 = simulatedTemp;
          }
        }
      } else if (!simulationMode) {
        simulationBuzzerTest = false;
        simulationTemp1 = -999.0;
        simulationTemp2 = -999.0;
      }
      checkBreachStatus();
      updateOLED();
      return;
    }

    if (doc.containsKey("type") && doc["type"] == "connected") {
      StaticJsonDocument<200> response;
      response["safeLimit"] = criticalThreshold;
      response["unit"] = isFahrenheit ? "F" : "C";
      String resp;
      serializeJson(response, resp);
      webSocket.sendTXT(num, resp);
      return; 
    }
  }
}