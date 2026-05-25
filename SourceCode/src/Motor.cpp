// Motor.cpp
#include "Motor.h"

Motor::Motor(uint8_t in1, uint8_t ena, uint8_t pwmChannel, uint32_t freq, uint8_t resolution)
  : in1_(in1), ena_(ena), pwmChannel_(pwmChannel),
    targetPercent_(0), speedPWM_(0), shadowChanged_(false)
{
  ledcSetup(pwmChannel_, freq, resolution);
}

void Motor::begin() {
  pinMode(in1_, OUTPUT);
  digitalWrite(in1_, HIGH);
  pinMode(ena_, OUTPUT);
  ledcAttachPin(ena_, pwmChannel_);
  ledcWrite(pwmChannel_, speedPWM_);
}

void Motor::setSpeedFromShadow(int percent) {
  percent = constrain(percent, 0, 100);
  targetPercent_ = percent;
  updatePWM();
  shadowChanged_ = true;
  Serial.printf("Motor: solicitud %d%% -> PWM %d\n", percent, speedPWM_);
}

int Motor::getSpeedPercent() const {
  return targetPercent_;
}

bool Motor::isShadowChanged() const {
  return shadowChanged_;
}

void Motor::clearShadowChanged() {
  shadowChanged_ = false;
}

void Motor::setLocalSpeed(int percent) {
  percent = constrain(percent, 0, 100);
  targetPercent_ = percent;
  updatePWM();
  // No modifica shadowChanged_
}

// --- private ---
void Motor::updatePWM() {
  const int minPWM = 77;   // 30% de 255 ≈ 76.5, round up to 77. Used to activate at the minimum order from above
  if (targetPercent_ == 0) {
    speedPWM_ = 0;
  } else {
    // Maps 1..100 → minPWM..255
    speedPWM_ = map(targetPercent_, 1, 100, minPWM, 255);
  }
  ledcWrite(pwmChannel_, speedPWM_);
}