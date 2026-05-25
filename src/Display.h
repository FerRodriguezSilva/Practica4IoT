#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Display {
public:
  Display(uint8_t addr, int sda, int scl);
  bool begin();
  void showNormal(float temperature, int speedPercent, float humidity,
                  bool autoMode, int tempThreshold);   // MODIFIED
  void showMessage(const char* line1, const char* line2, int speedPercent);

private:
  Adafruit_SSD1306 oled_;
};

#endif