#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <Arduino.h>

class Potentiometer {
public:
  Potentiometer(uint8_t pin);
  void begin();
  int readPercent();   // 0–100 con filtrado simple

private:
  uint8_t pin_;
  static const int numSamples = 8;
};

#endif