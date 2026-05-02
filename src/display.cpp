#include "display.h"

#include <Arduino.h>
#include <M5Unified.h>

#include "buttons.h"
#include "pins.h"

namespace {
constexpr uint16_t kBlack = 0x0000;
constexpr uint16_t kWhite = 0xFFFF;
constexpr uint16_t kGreen = 0x07E0;
constexpr uint16_t kCyan = 0x07FF;
constexpr uint16_t kYellow = 0xFFE0;
constexpr uint16_t kGray = 0x8410;
constexpr uint16_t kDarkGray = 0x4208;

void drawTamaViewport() {
  constexpr int x = 4;
  constexpr int y = 48;
  constexpr int w = 128;
  constexpr int h = 64;

  M5.Display.drawRect(x - 1, y - 1, w + 2, h + 2, kGray);
  M5.Display.fillRect(x, y, w, h, kBlack);
  M5.Display.drawRect(x + 8, y + 8, w - 16, h - 16, kDarkGray);
  M5.Display.setTextColor(kGray, kBlack);
  M5.Display.setCursor(x + 13, y + 27);
  M5.Display.print("TamaLIB 128x64");
}
}  // namespace

void displayInit() {
  M5.Display.setRotation(0);
  M5.Display.setBrightness(128);
  M5.Display.fillScreen(kBlack);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(kWhite, kBlack);
}

void displayRender(const ImuSample& imu) {
  M5.Display.fillScreen(kBlack);

  M5.Display.setTextColor(kGreen, kBlack);
  M5.Display.setCursor(4, 4);
  M5.Display.printf("%s smoke", board::kName);

  M5.Display.setTextColor(kWhite, kBlack);
  M5.Display.setCursor(4, 18);
  M5.Display.printf("boot ok  %lus", millis() / 1000);

  M5.Display.setTextColor(kCyan, kBlack);
  M5.Display.setCursor(4, 32);
  M5.Display.print(board::kSmokeTestVersion);

  drawTamaViewport();

  M5.Display.setTextColor(kYellow, kBlack);
  M5.Display.setCursor(4, 124);
  M5.Display.printf("Button: %s", buttonsLastEventName());

  M5.Display.setTextColor(kWhite, kBlack);
  M5.Display.setCursor(4, 140);
  M5.Display.print("A:key1     B:key2");
  M5.Display.setCursor(4, 154);
  M5.Display.print("C:power    AI:k1+k2");
  M5.Display.setCursor(4, 168);
  M5.Display.print("Menu:key2 hold");

  M5.Display.setCursor(4, 184);
  if (imu.valid) {
    M5.Display.printf("acc %5.2f %5.2f %5.2f", imu.ax, imu.ay, imu.az);
    M5.Display.setCursor(4, 198);
    M5.Display.printf("gyr %5.1f %5.1f %5.1f", imu.gx, imu.gy, imu.gz);
  } else {
    M5.Display.print("IMU waiting...");
  }

  M5.Display.setTextColor(kGray, kBlack);
  M5.Display.setCursor(4, 222);
  M5.Display.print("Serial: 115200 COM4");
}
