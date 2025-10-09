
#include "config.h"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoJson.h>


#include "WiFiModule.h"
#include "WaterLevelMonitor.h"
#include "WaterPumpModule.h"
#include "EnergyMeterModule.h"
#include "CTModule.h"
#include "MQTTModule.h"

// --- Hardware Pins ---
const int ledPinRed      = 14;   // WiFi Red LED
const int ledPinGreen    = 13;   // WiFi Green LED

const int motorRelayPin  = 26;

const int acs712Pin      = 34;

const int triggerPin     = 5;
const int echoPin        = 4;

const int ctPin          = 35;

const int buttonPin      = 22; // Data toggle button
const int blueLedPin    = 17; // Data status LED (Orange)
const int orangeLedPin      = 16; // Data status LED (Blue)

// --- Blynk state ---
bool isSendingEnabled = false;   // Start in the 'sending' state
BlynkTimer timer;
volatile bool buttonPressed = false;

// --- Debounce ---
volatile unsigned long lastButtonPress = 0;  // store last press time
const unsigned long debounceDelay = 350;     // ms

// --- Calibration ---
const float voltageCalibration = 225.0f;
const float sensitivity        = 185.0f;   // For ACS712-5A (change if using 20A or 30A)
const float ctCalibration = 1550.5f; // For ZMCT103C-5A

// --- Tank Calibration ---
float tankMinDistance = 8.0;   // Full tank (distance in cm)
float tankMaxDistance = 50.0;  // Empty tank (distance in cm)
float tankLevelPercent = 0.0;

// Pump threshold levels (in percentage)
float pumpOnLevelPercent  = 20.0;  // Turn ON when below 20%
float pumpOffLevelPercent = 90.0;  // Turn OFF when above 90%

// --- State variables ---
float minWaterLevel    = 20.0;
float maxWaterLevel    = 5.0;
bool autoModeEnabled   = true;
bool manualOverride    = false;

// --- Module status ---
bool isWaterSensorConnected = false;
bool isEnergyMeterConnected = false;
bool isWaterPumpConnected   = false;
bool isCTConnected = false;

// --- Blynk Virtual Pin Handlers ---
BLYNK_WRITE(V3) {  // Manual Pump Override
  manualOverride = param.asInt();
  Serial.printf("📱 Manual Override set to: %d\n", manualOverride);
}

BLYNK_WRITE(V4) {  // Auto Mode Toggle
  autoModeEnabled = param.asInt();
  Serial.printf("📱 Auto Mode set to: %d\n", autoModeEnabled);
}



// --- Send data to Blynk ---
void sendDataToBlynk() {
  if (isSendingEnabled) {
    if (isEnergyMeterConnected) {
      float power = EnergyMeterModule::getPower();
      float cumulativeEnergy = EnergyMeterModule::getCumulativeEnergy();
      Blynk.virtualWrite(V0, power);
      Blynk.virtualWrite(V1, cumulativeEnergy); 
    }
    if (isCTConnected) {
      float current = CTModule::getCurrent();
      Blynk.virtualWrite(V5, current);
    }
    if (isWaterSensorConnected) {
      float levelPercent = WaterLevelMonitor::getLevelPercent();
      Blynk.virtualWrite(V2, levelPercent);
    }
  }
}

// --- ISR with debounce ---
void IRAM_ATTR buttonPressHandler() {
  unsigned long currentTime = millis();
  if (currentTime - lastButtonPress > debounceDelay) {
    buttonPressed = true;
    lastButtonPress = currentTime;
  }
}

String deviceID;

void setup() {
  Serial.begin(115200);
  delay(DELAY);

  // Generate Unique Device ID
  char chipId[15];
  sprintf(chipId, "%06X", (uint32_t)(ESP.getEfuseMac() & 0xFFFFFF));
  deviceID = "ESP32-" + String(chipId);
  Serial.println("Device ID: " + deviceID);

  WiFi.disconnect(true);  // Disconnect and clear Wi-Fi credentials

  Serial.println("\n=== Smart Hub Booting... ===");

  // --- WiFi & Blynk Setup via WiFiModule ---
  WiFiModule::begin(ledPinRed, ledPinGreen);

  if (WiFiModule::isConnected()) {
    Blynk.config(BLYNK_AUTH_TOKEN);
    if (Blynk.connect(5000)) {
      Serial.println("✅ Blynk connected!");
    } else {
      Serial.println("⚠️ Blynk connection failed, will retry...");
    }
  }

  // --- Button & LEDs ---
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(blueLedPin, OUTPUT);
  pinMode(orangeLedPin, OUTPUT);
  
  digitalWrite(blueLedPin, LOW); // Green LED on initially
  digitalWrite(orangeLedPin, HIGH);

  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPressHandler, FALLING);

  // --- Initialize Modules ---
  Serial.println("🔍 Starting module discovery...");

  WaterLevelMonitor::begin(triggerPin, echoPin);
  isWaterSensorConnected = WaterLevelMonitor::isConnected();

  if (isWaterSensorConnected) {
    WaterLevelMonitor::calibrate(tankMinDistance, tankMaxDistance);
    WaterPumpModule::begin(motorRelayPin);
    isWaterPumpConnected = true;
    WaterPumpModule::turnOn();
    // Apply user-defined calibration
    Serial.println("✅ Water Level Calibration Applied:");
    Serial.printf("   Full tank distance: %.2f cm\n", tankMinDistance);
    Serial.printf("   Empty tank distance: %.2f cm\n", tankMaxDistance);
}

  EnergyMeterModule::begin(acs712Pin, voltageCalibration, sensitivity);
  isEnergyMeterConnected = EnergyMeterModule::isConnected();

// ... after initializing other modules
  CTModule::begin(ctPin, ctCalibration);
  isCTConnected = CTModule::isConnected();

// Connect a known load (e.g., a 100W light bulb)
// The current draw of a 100W bulb at 220V is approx. 0.45A.
// If you're in the US (120V), the current would be around 0.83A.
// Use the actual current of your known load.

// TEMPORARY CALIBRATION CODE:
// This will print the new value to the serial monitor.
// CTModule::calibrate(0.45f); 

  Serial.println("✅ Module discovery complete.");

  // Schedule send data every 15 sec
  timer.setInterval(15000L, sendDataToBlynk);
}

void loop() {  // ✅ keep WiFi status & LEDs updated
  Blynk.run();
  timer.run();

  // --- Handle button press ---
  if (buttonPressed) {
    buttonPressed = false;
    isSendingEnabled = !isSendingEnabled;

    if (isSendingEnabled) {
      digitalWrite(blueLedPin, HIGH);
      digitalWrite(orangeLedPin, LOW);
      Serial.println("🟢 Data sending enabled.");
    } else {
      digitalWrite(blueLedPin, LOW);
      digitalWrite(orangeLedPin, HIGH);
      Serial.println("🔴 Data sending paused.");
    }
  }

  // --- Read sensors ---
 float currentWaterLevel = -1.0;
  float waterLevelPercent = -1.0;
  if (isWaterSensorConnected) {
    currentWaterLevel = WaterLevelMonitor::getLevel();
    waterLevelPercent = WaterLevelMonitor::getLevelPercent();
    // Serial.printf("💧 Water Level: %.2f cm (%.1f%%)\n", currentWaterLevel, waterLevelPercent);
  }

  // --- Water Pump Control ---
  if (isWaterPumpConnected) {
    float maxLevel = pumpOffLevelPercent;   // full tank
    float minLevel = pumpOnLevelPercent;    // empty tank

    WaterPumpModule::update(
      waterLevelPercent,
      maxLevel,
      minLevel,
      autoModeEnabled,
      manualOverride
    );
  }

  // Reset manual override after action
  if (manualOverride) {
    manualOverride = false;
  }

  // --- Energy Meter Update ---
  if (isEnergyMeterConnected) {
    EnergyMeterModule::update();
  }
  //  --- CT Module Update ---
  if (isCTConnected) {
    CTModule::update();
  }

  // --- Debug Print every 2s ---
  static unsigned long lastSerialPrint = 0;
  if (millis() - lastSerialPrint > 3000) {
    lastSerialPrint = millis();

    Serial.printf("💧 Water Level: ");
    if (isWaterSensorConnected) {
      Serial.printf("💧 Water Level: %.2f cm (%.1f%%)\n", currentWaterLevel, waterLevelPercent);

    } else {
      Serial.printf("N/A");
    }

    Serial.printf(" | ⚡ Power: ");
    if (isEnergyMeterConnected) {
      Serial.printf("%.2f W | Total Units: %.4f kWh",
        EnergyMeterModule::getPower(),
        EnergyMeterModule::getCumulativeEnergy()
      );
    } else {
      Serial.printf("N/A");
    }

    Serial.printf(" | CT: ");
    if (isCTConnected) {
      Serial.printf("%.2f A", CTModule::getCurrent());
    } else {
      Serial.printf("N/A");
    }
    
    
    Serial.println();
    
  }
  
  // 👉 Raw ADC values
// int rawACS = analogRead(acs712Pin);
// int rawCT = analogRead(ctPin);
// Serial.printf(" | ACS712 Raw: %d | rawCT Raw: %d", rawACS, rawCT);
// Serial.println();


}
