#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

class WiFiManager {
public:
  WiFiManager(const char* ssid, const char* pass);
  bool connect(unsigned long timeoutMs = 10000);
  bool isConnected() const;
  String localIP() const;

private:
  const char* ssid_;
  const char* password_;
};

#endif