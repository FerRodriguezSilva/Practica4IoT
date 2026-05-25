#ifndef AWS_SHADOW_H
#define AWS_SHADOW_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Motor.h"
#include "Sensor.h"
#include "Display.h"

class AWSShadow {
public:
  AWSShadow(const char* thingName, const char* endpoint, int port,
            const char* rootCA, const char* deviceCert, const char* privateKey,
            Motor& motor, Sensor& sensor, Display& display);

  bool connect();
  void loop();
  void requestShadowDocument();
  void publishReported(float temperature, float humidity, int speedPercent);
  void publishDesired(int speedPercent);
  void publishDesiredSpeed(int speedPercent);
  void publishDesiredSettings(bool autoMode, int tempThreshold);  // NEW

  // Getters for auto‑mode and threshold
  bool getAutoMode() const { return autoMode_; }
  int  getTempThreshold() const { return tempThreshold_; }

  // topics
  String deltaTopic;
  String getTopic;
  String updateTopic;
  String getAcceptedTopic;   // NEW

private:
  static void mqttCallback(char* topic, byte* payload, unsigned int length);
  void handleDelta(const char* json);
  void handleFullDocument(const char* json);   // NEW

  const char* thingName_;
  const char* endpoint_;
  int port_;
  const char* rootCA_;
  const char* deviceCert_;
  const char* privateKey_;

  WiFiClientSecure net_;
  PubSubClient mqtt_;

  Motor& motor_;
  Sensor& sensor_;
  Display& display_;
  static AWSShadow* instance_;

  // NEW: local copy of shadow settings
  bool autoMode_;
  int  tempThreshold_;
};

#endif