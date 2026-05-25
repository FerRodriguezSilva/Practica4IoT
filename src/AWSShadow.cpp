#include "AWSShadow.h"

AWSShadow* AWSShadow::instance_ = nullptr;

AWSShadow::AWSShadow(const char* thingName, const char* endpoint, int port,
                     const char* rootCA, const char* deviceCert, const char* privateKey,
                     Motor& motor, Sensor& sensor, Display& display)
  : thingName_(thingName), endpoint_(endpoint), port_(port),
    rootCA_(rootCA), deviceCert_(deviceCert), privateKey_(privateKey),
    mqtt_(net_),
    motor_(motor), sensor_(sensor), display_(display),
    autoMode_(false), tempThreshold_(30)     // defaults
{
  deltaTopic  = String("$aws/things/") + thingName_ + "/shadow/update/delta";
  getTopic    = String("$aws/things/") + thingName_ + "/shadow/get";
  updateTopic = String("$aws/things/") + thingName_ + "/shadow/update";
  getAcceptedTopic = getTopic + "/accepted";     // NEW

  net_.setCACert(rootCA_);
  net_.setCertificate(deviceCert_);
  net_.setPrivateKey(privateKey_);

  mqtt_.setServer(endpoint_, port_);
  mqtt_.setCallback(mqttCallback);
  mqtt_.setBufferSize(512);
  
  instance_ = this;
}

bool AWSShadow::connect() {
  display_.showMessage("Conectando AWS...", "", motor_.getSpeedPercent());
  Serial.print("Conectando a AWS IoT...");
  if (mqtt_.connect(thingName_)) {
    Serial.println(" Conectado");
    mqtt_.subscribe(deltaTopic.c_str());
    mqtt_.subscribe(getAcceptedTopic.c_str());          // NEW
    Serial.printf("Suscrito a %s y %s\n", deltaTopic.c_str(), getAcceptedTopic.c_str());
    display_.showMessage("AWS Conectado", "", motor_.getSpeedPercent());
    return true;
  } else {
    Serial.println(" Fallo conexión MQTT");
    return false;
  }
}

void AWSShadow::loop() {
  if (!mqtt_.connected()) {
    if (connect()) {
      requestShadowDocument();
    } else {
      delay(1000);
      return;
    }
  }
  mqtt_.loop();
}

void AWSShadow::requestShadowDocument() {
  mqtt_.publish(getTopic.c_str(), "{}");
  Serial.println("Solicitado shadow document");
}

void AWSShadow::mqttCallback(char* topic, byte* payload, unsigned int length) {
  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = '\0';
  Serial.printf("MQTT mensaje en %s: %s\n", topic, msg);

  if (!instance_) return;

  if (String(topic) == instance_->deltaTopic) {
    instance_->handleDelta(msg);
  }
  else if (String(topic) == instance_->getAcceptedTopic) {   // NEW
    instance_->handleFullDocument(msg);
  }
}

void AWSShadow::handleDelta(const char* json) {
  StaticJsonDocument<384> doc;   // slightly larger for new fields
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.println("Error parseando JSON delta");
    return;
  }

  if (doc.containsKey("state")) {
    JsonObject state = doc["state"];

    // Speed change
    if (state.containsKey("speed")) {
      int percent = state["speed"];
      motor_.setSpeedFromShadow(percent);
    }

    // Auto‑mode change
    if (state.containsKey("autoMode")) {
      autoMode_ = state["autoMode"];
      Serial.printf("Auto mode changed to: %s\n", autoMode_ ? "ON" : "OFF");
    }

    // Temperature threshold change
    if (state.containsKey("tempThreshold")) {
      tempThreshold_ = state["tempThreshold"];
      Serial.printf("Temperature threshold changed to: %d°C\n", tempThreshold_);
    }
  }

  // Refresh display after any delta
  float temp = sensor_.readTemperature();
  float hum  = sensor_.readHumidity();
  display_.showNormal(temp, motor_.getSpeedPercent(), hum, autoMode_, tempThreshold_);
}

// NEW: parse full shadow document (response to $aws/things/.../get)
void AWSShadow::handleFullDocument(const char* json) {
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.println("Error parseando full shadow document");
    return;
  }

  // Extract desired state
  if (doc.containsKey("state") && doc["state"].containsKey("desired")) {
    JsonObject desired = doc["state"]["desired"];

    if (desired.containsKey("autoMode")) {
      autoMode_ = desired["autoMode"];
    }
    if (desired.containsKey("tempThreshold")) {
      tempThreshold_ = desired["tempThreshold"];
    }
    if (desired.containsKey("speed")) {
      int speed = desired["speed"];
      motor_.setSpeedFromShadow(speed);
    }

    Serial.printf("Initialized: autoMode=%d, threshold=%d°C, speed=%d%%\n",
                  autoMode_, tempThreshold_, motor_.getSpeedPercent());
  }
}

void AWSShadow::publishReported(float temperature, float humidity, int speedPercent) {
  StaticJsonDocument<384> doc;
  JsonObject state = doc.createNestedObject("state");
  JsonObject reported = state.createNestedObject("reported");

  if (!isnan(temperature)) reported["temperature"] = temperature;
  if (!isnan(humidity))    reported["humidity"]    = humidity;
  reported["speed"] = speedPercent;
  reported["autoMode"] = autoMode_;               // NEW
  reported["tempThreshold"] = tempThreshold_;     // NEW

  char buffer[384];
  serializeJson(doc, buffer);
  Serial.printf("Publicando shadow update: %s\n", buffer);

  if (mqtt_.publish(updateTopic.c_str(), buffer)) {
    Serial.println("Publicado correctamente");
  } else {
    Serial.println("Fallo en publicación");
  }
}

void AWSShadow::publishDesiredSpeed(int speedPercent) {
  StaticJsonDocument<128> doc;
  JsonObject state = doc.createNestedObject("state");
  JsonObject desired = state.createNestedObject("desired");
  desired["speed"] = speedPercent;

  char buffer[128];
  serializeJson(doc, buffer);
  mqtt_.publish(updateTopic.c_str(), buffer);
  Serial.printf("Nuevo desired speed: %d%%\n", speedPercent);
}

// NEW: publish auto‑mode and threshold together (optional, for completeness)
void AWSShadow::publishDesiredSettings(bool autoMode, int tempThreshold) {
  StaticJsonDocument<256> doc;
  JsonObject state = doc.createNestedObject("state");
  JsonObject desired = state.createNestedObject("desired");
  desired["autoMode"] = autoMode;
  desired["tempThreshold"] = tempThreshold;

  char buffer[256];
  serializeJson(doc, buffer);
  mqtt_.publish(updateTopic.c_str(), buffer);
  Serial.printf("Publicados nuevos settings: autoMode=%d, threshold=%d\n", autoMode, tempThreshold);
}