#ifndef SENSOR_H
#define SENSOR_H

#include <DHT.h>

class Sensor {
public:
  Sensor(uint8_t pin, uint8_t type);
  void begin();
  float readTemperature();   // NAN if failure
  float readHumidity();     // %, NAN si falla


private:
  DHT dht_;
};

#endif