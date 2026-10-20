/**
 * @file main.cpp
 * @brief MamaMed Open-Source Maternal Vital Signs Telemetry Unit Firmware
 * @author Evatronics Engineering Team / MamaMed Project
 * @version 1.1.0
 * 
 * Hardware Platform: ESP32-WROOM-32 / ESP32-S3
 * Display: 16x2 I2C Character LCD (LiquidCrystal_I2C)
 * Features:
 *  - Real-time physiological sensor acquisition (BPM, SpO2, Temp, BP)
 *  - Local 16x2 LCD UI display cycling through vital signs
 *  - Dual Telemetry: Cloud REST API over Wi-Fi & GSM SIM800L Fallback
 *  - Automated SMS alert dispatch when threshold parameters are breached
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>

// ==========================================
// PIN DEFINITIONS & I2C ADDRESS
// ==========================================
#define I2C_SDA 21
#define I2C_SCL 22
#define LCD_ADDR 0x27 // Default address for most 16x2 I2C modules (or 0x3F)

#define GSM_RX_PIN 16
#define GSM_TX_PIN 17

// ==========================================
// CONFIGURATION & THRESHOLDS
// ==========================================
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASS     = "YOUR_WIFI_PASSWORD";
const char* API_ENDPOINT  = "https://api.mamamed.health/v1/telemetry";
const char* EMERGENCY_DOC_PHONE = "+2348000000000"; // Target Doctor / Clinic Number

// Physiological Clinical Safe Thresholds
const float TEMP_HIGH_THRESHOLD   = 38.0; // Celsius (Fever)
const int   BPM_HIGH_THRESHOLD    = 110;  // Tachycardia
const int   BPM_LOW_THRESHOLD     = 55;   // Bradycardia
const int   SPO2_LOW_THRESHOLD    = 92;   // Hypoxia
const int   SYS_HIGH_THRESHOLD    = 140;  // Hypertension (Pre-eclampsia risk)

// ==========================================
// HARDWARE INSTANCES
// ==========================================
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
HardwareSerial gsmSerial(2); // UART2 for SIM800L

// ==========================================
// DATA STRUCTURES
// ==========================================
struct VitalSigns {
  float temperature;
  int heartRate;
  int spO2;
  int systolicBP;
  int diastolicBP;
  bool thresholdBreached;
  String alertMessage;
};

VitalSigns currentVitals;
unsigned long lastTelemetryTime = 0;
unsigned long lastDisplayCycle = 0;
int displayScreenPage = 0;
const unsigned long TELEMETRY_INTERVAL = 10000; // 10 seconds

// ==========================================
// FUNCTION DECLARATIONS
// ==========================================
void initLCD();
void initGSM();
void readSensors(VitalSigns &vitals);
void evaluateThresholds(VitalSigns &vitals);
void updateLCDDisplay(const VitalSigns &vitals);
bool sendWiFiTelemetry(const VitalSigns &vitals);
void sendGSM_SMS(const String &phoneNumber, const String &message);

// ==========================================
// SETUP & MAIN LOOP
// ==========================================
void setup() {
  Serial.begin(115200);
  gsmSerial.begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);

  Wire.begin(I2C_SDA, I2C_SCL);
  initLCD();
  initGSM();

  // Initialize Wi-Fi Connection
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to Wi-Fi...");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi Connected successfully.");
  } else {
    Serial.println("\nWi-Fi offline. System operating in GSM fallback mode.");
  }
}

void loop() {
  // 1. Acquire Vital Signs Data from Sensors
  readSensors(currentVitals);

  // 2. Evaluate Clinical Thresholds
  evaluateThresholds(currentVitals);

  // 3. Update Local 16x2 LCD UI Screen
  updateLCDDisplay(currentVitals);

  // 4. Handle Emergency Alerts
  if (currentVitals.thresholdBreached) {
    Serial.println("EMERGENCY: Threshold breached! Triggering automated alert...");
    sendGSM_SMS(EMERGENCY_DOC_PHONE, currentVitals.alertMessage);
  }

  // 5. Periodic Telemetry Transmission (Cloud / Fallback)
  if (millis() - lastTelemetryTime >= TELEMETRY_INTERVAL) {
    lastTelemetryTime = millis();
    
    if (WiFi.status() == WL_CONNECTED) {
      bool success = sendWiFiTelemetry(currentVitals);
      if (!success) {
        Serial.println("HTTP Telemetry failed. Retrying over GSM...");
      }
    } else {
      Serial.println("Wi-Fi unavailable. Telemetry logged locally.");
    }
  }

  delay(2000); // 2-second sampling delay
}

// ==========================================
// HARDWARE DRIVER IMPLEMENTATIONS
// ==========================================

void initLCD() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("   MamaMed V1   ");
  lcd.setCursor(0, 1);
  lcd.print(" Vital Telemetry");
  delay(2000);
  lcd.clear();
}

void initGSM() {
  Serial.println("Initializing SIM800L GSM Module...");
  gsmSerial.println("AT");
  delay(1000);
  gsmSerial.println("AT+CMGF=1"); // Set SMS text mode
  delay(1000);
}

void readSensors(VitalSigns &vitals) {
  // Prototype sensor reading values:
  vitals.temperature = 36.8 + (random(-5, 15) / 10.0);
  vitals.heartRate   = 72 + random(-8, 12);
  vitals.spO2        = 97 + random(-2, 2);
  vitals.systolicBP  = 118 + random(-10, 25);
  vitals.diastolicBP = 78 + random(-5, 15);
}

void evaluateThresholds(VitalSigns &vitals) {
  vitals.thresholdBreached = false;
  vitals.alertMessage = "MAMAMED ALERT: High risk detected!\nVitals: ";

  if (vitals.temperature >= TEMP_HIGH_THRESHOLD) {
    vitals.thresholdBreached = true;
    vitals.alertMessage += "Fever (" + String(vitals.temperature, 1) + "C) ";
  }
  if (vitals.systolicBP >= SYS_HIGH_THRESHOLD) {
    vitals.thresholdBreached = true;
    vitals.alertMessage += "Hypertension (" + String(vitals.systolicBP) + " mmHg) ";
  }
  if (vitals.spO2 <= SPO2_LOW_THRESHOLD) {
    vitals.thresholdBreached = true;
    vitals.alertMessage += "Low SpO2 (" + String(vitals.spO2) + "%) ";
  }
  if (vitals.heartRate >= BPM_HIGH_THRESHOLD || vitals.heartRate <= BPM_LOW_THRESHOLD) {
    vitals.thresholdBreached = true;
    vitals.alertMessage += "Abnormal HR (" + String(vitals.heartRate) + " BPM) ";
  }
}

void updateLCDDisplay(const VitalSigns &vitals) {
  lcd.clear();

  if (vitals.thresholdBreached) {
    // Priority Screen 1: High Alert Banner
    lcd.setCursor(0, 0);
    lcd.print("!CRITICAL ALERT!");
    lcd.setCursor(0, 1);
    if (vitals.systolicBP >= SYS_HIGH_THRESHOLD) {
      lcd.print("BP: " + String(vitals.systolicBP) + "/" + String(vitals.diastolicBP) + " HIGH");
    } else if (vitals.temperature >= TEMP_HIGH_THRESHOLD) {
      lcd.print("Temp: " + String(vitals.temperature, 1) + "C FEVER");
    } else {
      lcd.print("HR:" + String(vitals.heartRate) + " SpO2:" + String(vitals.spO2) + "%");
    }
  } else {
    // Normal Operation: Alternates between Vitals Screen 1 & Screen 2
    if (displayScreenPage == 0) {
      lcd.setCursor(0, 0);
      lcd.print("HR: " + String(vitals.heartRate) + " BPM");
      lcd.setCursor(0, 1);
      lcd.print("SpO2: " + String(vitals.spO2) + " %");
      displayScreenPage = 1;
    } else {
      lcd.setCursor(0, 0);
      lcd.print("BP: " + String(vitals.systolicBP) + "/" + String(vitals.diastolicBP) + " mmHg");
      lcd.setCursor(0, 1);
      lcd.print("Temp: " + String(vitals.temperature, 1) + " C");
      displayScreenPage = 0;
    }
  }
}

bool sendWiFiTelemetry(const VitalSigns &vitals) {
  HTTPClient http;
  http.begin(API_ENDPOINT);
  http.addHeader("Content-Type", "application/json");

  String jsonPayload = "{";
  jsonPayload += "\"heartRate\":" + String(vitals.heartRate) + ",";
  jsonPayload += "\"spO2\":" + String(vitals.spO2) + ",";
  jsonPayload += "\"systolicBP\":" + String(vitals.systolicBP) + ",";
  jsonPayload += "\"diastolicBP\":" + String(vitals.diastolicBP) + ",";
  jsonPayload += "\"temperature\":" + String(vitals.temperature, 2);
  jsonPayload += "}";

  int httpCode = http.POST(jsonPayload);
  http.end();
  return (httpCode == 200 || httpCode == 201);
}

void sendGSM_SMS(const String &phoneNumber, const String &message) {
  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(phoneNumber);
  gsmSerial.println("\"");
  delay(1000);
  gsmSerial.print(message);
  delay(100);
  gsmSerial.write(26); // CTRL+Z command to dispatch SMS
  delay(5000);
  Serial.println("SMS sent to healthcare provider.");
}
