#include "WiFiModule.h"
#include <WiFi.h>
#include <WiFiManager.h>

namespace WiFiModule
{
    static WiFiManager _wifiManager;
    static int _redLed, _greenLed;

    void begin(int redLedPin, int greenLedPin)
    {        
        _redLed = redLedPin;
        _greenLed = greenLedPin;

        pinMode(_redLed, OUTPUT);
        pinMode(_greenLed, OUTPUT);
        digitalWrite(_redLed, HIGH);
        digitalWrite(_greenLed, LOW);
        String apName = getUniqueId();

        if (!_wifiManager.autoConnect(apName.c_str()))
        {
            Serial.println("❌ Failed to connect, starting AP mode.");
        }

        // This part needs to be in the main loop to not block
        // during autoConnect
        if (isConnected())
        {
            digitalWrite(_redLed, LOW);
            digitalWrite(_greenLed, HIGH);
            Serial.println("WiFi Connected!");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
        }
    }

    bool isConnected()
    {
        return WiFi.status() == WL_CONNECTED;
    }

    String getSSID()
    {
        if (isConnected())
        {
            return WiFi.SSID();
        }
        return "Not Connected";
    }


    String getPassword()
    {
        if (isConnected())
        {
            return WiFi.psk();
        }
        return "Not Connected";
    }

    String getIP()
    {
        if (isConnected())
        {
            return WiFi.localIP().toString();
        }
        return "Not Connected";
    }

     String getUniqueId()
    {
        String mac = WiFi.macAddress(); // like "AB:CD:EF:12:34:56"
        mac.replace(":", "");
        mac.toUpperCase();
        // Use last 6 hex chars
        return "ESP32_IoT-Sight-" + mac.substring(mac.length() - 6);
    }

    void resetSettings()
    {
        _wifiManager.resetSettings();
        Serial.println("⚠️ WiFi settings reset. Reboot to reconfigure.");
    }
}