#include "Sensor.h"

Sensor::Sensor(uint8_t pin, uint8_t type) : dht_(pin, type) {}

void Sensor::begin() {
  dht_.begin();
}

float Sensor::readTemperature() {
  float t = dht_.readTemperature();
  if (isnan(t)) {
    Serial.println("Sensor: error lectura");
  }
  return t;
}

float Sensor::readHumidity() {
  float h = dht_.readHumidity();
  if (isnan(h)) {
    Serial.println("Sensor: error lectura humedad");
  }
  return h;
}