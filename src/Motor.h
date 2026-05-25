// Motor.h
#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

class Motor {
public:
  Motor(uint8_t in1, uint8_t ena, uint8_t pwmChannel, uint32_t freq, uint8_t resolution);
  void begin();
  void setSpeedFromShadow(int percent);   // User percent of usage
  void setLocalSpeed(int percent);        // usado por potenciómetro (sin flag)
  int  getSpeedPercent() const;           // returns the requested percentage
  bool isShadowChanged() const;
  void clearShadowChanged();

private:
  void updatePWM();                       // Change the actual PWM output based on targetPercent_

  uint8_t in1_, ena_, pwmChannel_;
  int targetPercent_;   // desired percentage (0‑100)
  int speedPWM_;        // actual PWM value (0‑255)
  bool shadowChanged_;
};

#endif