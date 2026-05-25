#include "WiFiManager.h"

WiFiManager::WiFiManager(const char* ssid, const char* pass)
  : ssid_(ssid), password_(pass) {}

bool WiFiManager::connect(unsigned long timeoutMs) {
  WiFi.begin(ssid_, password_);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    return true;
  } else {
    Serial.println("\nWiFi failed");
    return false;
  }
}

bool WiFiManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::localIP() const {
  return WiFi.localIP().toString();
}