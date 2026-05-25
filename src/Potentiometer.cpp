#include "Potentiometer.h"

Potentiometer::Potentiometer(uint8_t pin) : pin_(pin) {}

void Potentiometer::begin() {
  pinMode(pin_, INPUT);
  analogRead(pin_); // descartar primera lectura
}

int Potentiometer::readPercent() {
  long sum = 0;
  for (int i = 0; i < numSamples; i++) {
    sum += analogRead(pin_);
    delay(1);
  }
  int raw = sum / numSamples;
  return map(raw, 0, 4095, 0, 100);
}