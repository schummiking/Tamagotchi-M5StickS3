#include "display.h"

#include <Arduino.h>
#include <M5Unified.h>

#include "buttons.h"
#include "pins.h"

namespace {
constexpr uint16_t kBlack = 0x0000;
constexpr uint16_t kWhite = 0xFFFF;
constexpr uint16_t kGreen = 0x07E0;
constexpr uint16_t kTamaPixel = 0x9FE6;
constexpr uint16_t kCyan = 0x07FF;
constexpr uint16_t kYellow = 0xFFE0;
constexpr uint16_t kRed = 0xF800;
constexpr uint16_t kGray = 0x8410;
constexpr uint16_t kDarkGray = 0x4208;

void drawTamaViewportFrame() {
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

void drawControls(uint16_t color) {
  M5.Display.setTextColor(color, kBlack);
  M5.Display.setCursor(4, 140);
  M5.Display.print("A:key1     B:key2");
  M5.Display.setCursor(4, 154);
  M5.Display.print("C:key1+key2");
  M5.Display.setCursor(4, 168);
  M5.Display.print("Menu:k1h   AI:k2h");
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

  drawTamaViewportFrame();

  M5.Display.setTextColor(kYellow, kBlack);
  M5.Display.setCursor(4, 124);
  M5.Display.printf("Button: %s", buttonsLastEventName());

  drawControls(kWhite);

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

void displayRenderMissingRom(const ImuSample& imu, bool init_failed) {
  M5.Display.fillScreen(kBlack);

  M5.Display.setTextColor(kGreen, kBlack);
  M5.Display.setCursor(4, 4);
  M5.Display.print("M5StickS3 TamaLIB");

  M5.Display.setTextColor(init_failed ? kRed : kYellow, kBlack);
  M5.Display.setCursor(4, 20);
  M5.Display.print(init_failed ? "ROM init failed" : "ROM not found");

  M5.Display.setTextColor(kWhite, kBlack);
  M5.Display.setCursor(4, 38);
  M5.Display.print("Generate data/rom.h");
  M5.Display.setCursor(4, 52);
  M5.Display.print("tools/rom_to_header.py");

  drawTamaViewportFrame();

  M5.Display.setTextColor(kYellow, kBlack);
  M5.Display.setCursor(4, 124);
  M5.Display.printf("Button: %s", buttonsLastEventName());

  drawControls(kWhite);

  M5.Display.setTextColor(kWhite, kBlack);
  M5.Display.setCursor(4, 190);
  if (imu.valid) {
    M5.Display.printf("acc %5.2f %5.2f %5.2f", imu.ax, imu.ay, imu.az);
  } else {
    M5.Display.print("IMU waiting...");
  }

  M5.Display.setTextColor(kGray, kBlack);
  M5.Display.setCursor(4, 222);
  M5.Display.print("power is system-only");
}

void displayRenderTama(const bool* pixels, const bool* icons, bool sound_on) {
  constexpr int x = 4;
  constexpr int y = 44;
  constexpr int scale = 4;
  constexpr int width = 32;
  constexpr int height = 16;

  M5.Display.fillScreen(kBlack);

  M5.Display.setTextColor(kGreen, kBlack);
  M5.Display.setCursor(4, 4);
  M5.Display.print("Tamagotchi P1");

  M5.Display.setTextColor(kGray, kBlack);
  M5.Display.setCursor(4, 18);
  M5.Display.printf("%lus  sound:%s", millis() / 1000, sound_on ? "on" : "off");

  M5.Display.drawRect(x - 2, y - 2, width * scale + 4, height * scale + 4, kGray);
  M5.Display.fillRect(x, y, width * scale, height * scale, kBlack);

  for (int row = 0; row < height; ++row) {
    for (int col = 0; col < width; ++col) {
      if (pixels[row * width + col]) {
        M5.Display.fillRect(x + col * scale, y + row * scale, scale, scale, kTamaPixel);
      }
    }
  }

  M5.Display.setTextColor(kGray, kBlack);
  M5.Display.setCursor(4, 116);
  M5.Display.print("icons");
  for (int icon = 0; icon < 8; ++icon) {
    const int icon_x = 40 + icon * 11;
    const uint16_t color = icons[icon] ? kYellow : kDarkGray;
    M5.Display.fillRect(icon_x, 116, 7, 7, color);
  }

  M5.Display.setTextColor(kYellow, kBlack);
  M5.Display.setCursor(4, 132);
  M5.Display.printf("Button: %s", buttonsLastEventName());

  drawControls(kWhite);

  M5.Display.setTextColor(kGray, kBlack);
  M5.Display.setCursor(4, 222);
  M5.Display.print("TamaLIB local ROM");
}
