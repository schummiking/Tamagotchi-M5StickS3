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
constexpr int kTamaX = 4;
constexpr int kTamaY = 44;
constexpr int kTamaScale = 4;
constexpr int kTamaWidth = 32;
constexpr int kTamaHeight = 16;
constexpr int kTamaIconCount = 8;

bool g_tama_frame_ready = false;
bool g_last_tama_pixels[kTamaWidth * kTamaHeight] = {};
bool g_last_tama_icons[kTamaIconCount] = {};
bool g_last_tama_sound = false;
uint32_t g_last_tama_second = UINT32_MAX;

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

void drawTamaStatus(bool sound_on) {
  const uint32_t seconds = millis() / 1000;
  if (seconds == g_last_tama_second && sound_on == g_last_tama_sound) {
    return;
  }

  g_last_tama_second = seconds;
  g_last_tama_sound = sound_on;
  M5.Display.fillRect(4, 18, 128, 10, kBlack);
  M5.Display.setTextColor(kGray, kBlack);
  M5.Display.setCursor(4, 18);
  M5.Display.printf("%lus  sound:%s", seconds, sound_on ? "on" : "off");
}

void drawTamaButtonStatus() {
  M5.Display.fillRect(4, 132, 128, 10, kBlack);
  M5.Display.setTextColor(kYellow, kBlack);
  M5.Display.setCursor(4, 132);
  M5.Display.printf("Button: %s", buttonsLastEventName());
}

void drawTamaStaticFrame(bool sound_on) {
  M5.Display.fillScreen(kBlack);

  M5.Display.setTextColor(kGreen, kBlack);
  M5.Display.setCursor(4, 4);
  M5.Display.print("Tamagotchi P1");

  g_last_tama_second = UINT32_MAX;
  drawTamaStatus(sound_on);

  M5.Display.drawRect(kTamaX - 2, kTamaY - 2, kTamaWidth * kTamaScale + 4,
                      kTamaHeight * kTamaScale + 4, kGray);
  M5.Display.fillRect(kTamaX, kTamaY, kTamaWidth * kTamaScale, kTamaHeight * kTamaScale,
                      kBlack);

  M5.Display.setTextColor(kGray, kBlack);
  M5.Display.setCursor(4, 116);
  M5.Display.print("icons");
  for (int icon = 0; icon < kTamaIconCount; ++icon) {
    const int icon_x = 40 + icon * 11;
    M5.Display.fillRect(icon_x, 116, 7, 7, kDarkGray);
    g_last_tama_icons[icon] = false;
  }

  drawTamaButtonStatus();
  drawControls(kWhite);

  M5.Display.setTextColor(kGray, kBlack);
  M5.Display.setCursor(4, 222);
  M5.Display.print("TamaLIB local ROM");

  for (int i = 0; i < kTamaWidth * kTamaHeight; ++i) {
    g_last_tama_pixels[i] = false;
  }
  g_tama_frame_ready = true;
}
}  // namespace

void displayInit() {
  M5.Display.setRotation(0);
  displaySetBrightness(128);
  M5.Display.fillScreen(kBlack);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(kWhite, kBlack);
}

void displaySetBrightness(uint8_t brightness) {
  M5.Display.setBrightness(brightness);
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
  if (!g_tama_frame_ready) {
    drawTamaStaticFrame(sound_on);
  }

  drawTamaStatus(sound_on);
  if (buttonsLastEventAgeMs() < 350) {
    drawTamaButtonStatus();
  }

  M5.Display.startWrite();
  for (int row = 0; row < kTamaHeight; ++row) {
    for (int col = 0; col < kTamaWidth; ++col) {
      const int index = row * kTamaWidth + col;
      const bool pixel_on = pixels[index];
      if (pixel_on != g_last_tama_pixels[index]) {
        const uint16_t color = pixel_on ? kTamaPixel : kBlack;
        M5.Display.fillRect(kTamaX + col * kTamaScale, kTamaY + row * kTamaScale,
                            kTamaScale, kTamaScale, color);
        g_last_tama_pixels[index] = pixel_on;
      }
    }
  }

  for (int icon = 0; icon < kTamaIconCount; ++icon) {
    if (icons[icon] != g_last_tama_icons[icon]) {
      const int icon_x = 40 + icon * 11;
      const uint16_t color = icons[icon] ? kYellow : kDarkGray;
      M5.Display.fillRect(icon_x, 116, 7, 7, color);
      g_last_tama_icons[icon] = icons[icon];
    }
  }
  M5.Display.endWrite();
}
