#include "display.h"

#include <Arduino.h>
#include <M5Unified.h>
#include <string.h>

#include "buttons.h"
#include "pins.h"
#include "settings.h"
#include "tama_frame.h"

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
constexpr int kTamaWidth = kTamaFrameWidth;
constexpr int kTamaHeight = kTamaFrameHeight;
constexpr int kTamaIconCount = 8;
constexpr int kMenuX = 4;
constexpr int kMenuY = 118;
constexpr int kMenuCols = 4;
constexpr int kMenuCellW = 32;
constexpr int kMenuCellH = 42;

bool g_tama_frame_ready = false;
bool g_last_tama_pixels[kTamaWidth * kTamaHeight] = {};
bool g_last_tama_icons[kTamaIconCount] = {};
uint8_t g_last_brightness = UINT8_MAX;
uint8_t g_last_volume = UINT8_MAX;
int8_t g_last_menu_hint = -2;

constexpr const char* kMenuLabels[kTamaIconCount] = {
    "FOOD", "LIGHT", "GAME", "MED", "CLEAN", "STAT", "DISC", "CALL"};

constexpr const char* kMenuHints[kTamaIconCount] = {
    "Feed: meal or snack",
    "Light: room lamp",
    "Game: play",
    "Medicine: cure",
    "Clean: duck flush",
    "Status: hearts",
    "Discipline: scold",
    "Attention call",
};

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

uint8_t threeStepLevel(uint8_t value, uint8_t low, uint8_t mid) {
  if (value <= low) {
    return 1;
  }
  if (value <= mid) {
    return 2;
  }
  return 3;
}

uint8_t volumeLevel(uint8_t volume) {
  if (volume == 0) {
    return 0;
  }
  if (volume <= 32) {
    return 1;
  }
  if (volume <= 96) {
    return 2;
  }
  return 3;
}

void drawMeter(int x, int y, uint8_t level, uint8_t slots, uint16_t color) {
  for (uint8_t slot = 0; slot < slots; ++slot) {
    const int bar_x = x + slot * 8;
    const int bar_h = 4 + slot * 2;
    const int bar_y = y + 8 - bar_h;
    M5.Display.drawRect(bar_x, y, 6, 8, kDarkGray);
    if (slot < level) {
      M5.Display.fillRect(bar_x + 1, bar_y + 1, 4, bar_h - 1, color);
    } else {
      M5.Display.fillRect(bar_x + 1, y + 1, 4, 6, kBlack);
    }
  }
}

void drawVolumeMeter(int x, int y, uint8_t level) {
  for (uint8_t slot = 0; slot < 4; ++slot) {
    const int bar_x = x + slot * 7;
    M5.Display.drawRect(bar_x, y, 5, 8, kDarkGray);
    M5.Display.fillRect(bar_x + 1, y + 1, 3, 6, kBlack);
    if (level == 0 && slot == 0) {
      M5.Display.drawLine(bar_x + 1, y + 2, bar_x + 3, y + 5, kRed);
      M5.Display.drawLine(bar_x + 3, y + 2, bar_x + 1, y + 5, kRed);
    } else if (level > 0 && slot > 0 && slot <= level) {
      const int bar_h = 2 + slot * 2;
      const int bar_y = y + 8 - bar_h;
      M5.Display.fillRect(bar_x + 1, bar_y + 1, 3, bar_h - 1, kCyan);
    }
  }
}

void drawTamaStatus() {
  const uint8_t brightness = settingsActiveBrightness();
  const uint8_t volume = settingsVolume();
  if (brightness == g_last_brightness && volume == g_last_volume) {
    return;
  }

  g_last_brightness = brightness;
  g_last_volume = volume;
  M5.Display.fillRect(4, 18, 128, 10, kBlack);
  M5.Display.setTextColor(kGray, kBlack);
  M5.Display.setCursor(4, 18);
  M5.Display.print("BRI");
  drawMeter(28, 18, threeStepLevel(brightness, 64, 128), 3, kYellow);
  M5.Display.setCursor(70, 18);
  M5.Display.print("VOL");
  drawVolumeMeter(94, 18, volumeLevel(volume));
}

void drawCenteredText(const char* text, int x, int y, int width, uint16_t color) {
  const int text_width = static_cast<int>(strlen(text)) * 6;
  const int text_x = x + max(0, (width - text_width) / 2);
  M5.Display.setTextColor(color, kBlack);
  M5.Display.setCursor(text_x, y);
  M5.Display.print(text);
}

void drawFoodIcon(int cx, int y, uint16_t color) {
  M5.Display.drawLine(cx - 8, y + 12, cx + 8, y + 12, color);
  M5.Display.drawLine(cx - 6, y + 16, cx + 6, y + 16, color);
  M5.Display.drawLine(cx - 8, y + 12, cx - 6, y + 16, color);
  M5.Display.drawLine(cx + 8, y + 12, cx + 6, y + 16, color);
  M5.Display.fillCircle(cx - 4, y + 9, 2, color);
  M5.Display.fillCircle(cx, y + 8, 2, color);
  M5.Display.fillCircle(cx + 4, y + 9, 2, color);
}

void drawLightIcon(int cx, int y, uint16_t color) {
  M5.Display.drawCircle(cx, y + 8, 5, color);
  M5.Display.fillRect(cx - 3, y + 14, 7, 3, color);
  M5.Display.drawLine(cx - 9, y + 8, cx - 7, y + 8, color);
  M5.Display.drawLine(cx + 7, y + 8, cx + 9, y + 8, color);
  M5.Display.drawLine(cx, y, cx, y + 2, color);
  M5.Display.drawLine(cx - 6, y + 2, cx - 5, y + 4, color);
  M5.Display.drawLine(cx + 6, y + 2, cx + 5, y + 4, color);
}

void drawGameIcon(int cx, int y, uint16_t color) {
  M5.Display.drawRect(cx - 9, y + 5, 18, 12, color);
  M5.Display.drawLine(cx - 6, y + 11, cx - 2, y + 11, color);
  M5.Display.drawLine(cx - 4, y + 9, cx - 4, y + 13, color);
  M5.Display.fillCircle(cx + 4, y + 10, 1, color);
  M5.Display.fillCircle(cx + 7, y + 13, 1, color);
}

void drawMedicineIcon(int cx, int y, uint16_t color) {
  M5.Display.drawRect(cx - 7, y + 4, 14, 14, color);
  M5.Display.fillRect(cx - 2, y + 7, 5, 8, color);
  M5.Display.fillRect(cx - 5, y + 10, 11, 3, color);
}

void drawCleanIcon(int cx, int y, uint16_t color) {
  M5.Display.drawCircle(cx + 3, y + 7, 4, color);
  M5.Display.drawLine(cx + 7, y + 7, cx + 10, y + 6, color);
  M5.Display.drawLine(cx + 7, y + 8, cx + 10, y + 9, color);
  M5.Display.drawLine(cx - 8, y + 13, cx + 4, y + 13, color);
  M5.Display.drawLine(cx - 6, y + 16, cx + 6, y + 16, color);
  M5.Display.drawLine(cx - 8, y + 13, cx - 6, y + 16, color);
  M5.Display.drawLine(cx + 4, y + 13, cx + 6, y + 16, color);
  M5.Display.drawLine(cx - 10, y + 19, cx - 6, y + 19, color);
  M5.Display.drawLine(cx - 2, y + 19, cx + 2, y + 19, color);
  M5.Display.drawLine(cx + 6, y + 19, cx + 10, y + 19, color);
}

void drawStatusIcon(int cx, int y, uint16_t color) {
  M5.Display.drawRect(cx - 9, y + 4, 18, 14, color);
  M5.Display.fillRect(cx - 6, y + 13, 3, 3, color);
  M5.Display.fillRect(cx - 1, y + 10, 3, 6, color);
  M5.Display.fillRect(cx + 4, y + 7, 3, 9, color);
  M5.Display.drawLine(cx - 7, y + 7, cx - 5, y + 5, color);
  M5.Display.drawLine(cx - 5, y + 5, cx - 3, y + 7, color);
  M5.Display.drawLine(cx + 3, y + 7, cx + 5, y + 5, color);
  M5.Display.drawLine(cx + 5, y + 5, cx + 7, y + 7, color);
}

void drawDisciplineIcon(int cx, int y, uint16_t color) {
  M5.Display.drawCircle(cx, y + 10, 8, color);
  M5.Display.drawLine(cx - 5, y + 7, cx - 2, y + 8, color);
  M5.Display.drawLine(cx + 2, y + 8, cx + 5, y + 7, color);
  M5.Display.drawLine(cx - 4, y + 14, cx + 4, y + 14, color);
  M5.Display.drawLine(cx + 9, y + 3, cx + 9, y + 10, color);
  M5.Display.fillCircle(cx + 9, y + 14, 1, color);
}

void drawCallIcon(int cx, int y, uint16_t color) {
  M5.Display.drawLine(cx - 7, y + 15, cx + 7, y + 15, color);
  M5.Display.drawLine(cx - 5, y + 15, cx - 5, y + 8, color);
  M5.Display.drawLine(cx + 5, y + 15, cx + 5, y + 8, color);
  M5.Display.drawLine(cx - 5, y + 8, cx, y + 3, color);
  M5.Display.drawLine(cx + 5, y + 8, cx, y + 3, color);
  M5.Display.fillCircle(cx, y + 18, 1, color);
  M5.Display.drawLine(cx - 10, y + 5, cx - 8, y + 3, color);
  M5.Display.drawLine(cx + 8, y + 3, cx + 10, y + 5, color);
}

void drawMenuIcon(int icon, bool active) {
  const int col = icon % kMenuCols;
  const int row = icon / kMenuCols;
  const int x = kMenuX + col * kMenuCellW;
  const int y = kMenuY + row * kMenuCellH;
  const uint16_t border = active ? kYellow : kDarkGray;
  const uint16_t ink = active ? kWhite : kGray;
  const uint16_t label = active ? kYellow : kGray;
  const int cx = x + kMenuCellW / 2;

  M5.Display.fillRect(x, y, kMenuCellW - 2, kMenuCellH - 3, kBlack);
  M5.Display.drawRect(x, y, kMenuCellW - 2, kMenuCellH - 3, border);
  switch (icon) {
    case 0:
      drawFoodIcon(cx, y + 4, ink);
      break;
    case 1:
      drawLightIcon(cx, y + 4, ink);
      break;
    case 2:
      drawGameIcon(cx, y + 4, ink);
      break;
    case 3:
      drawMedicineIcon(cx, y + 4, ink);
      break;
    case 4:
      drawCleanIcon(cx, y + 2, ink);
      break;
    case 5:
      drawStatusIcon(cx, y + 4, ink);
      break;
    case 6:
      drawDisciplineIcon(cx, y + 4, ink);
      break;
    case 7:
      drawCallIcon(cx, y + 4, ink);
      break;
    default:
      break;
  }
  drawCenteredText(kMenuLabels[icon], x, y + 28, kMenuCellW - 2, label);
}

int8_t selectedMenuIcon(const bool* icons) {
  for (int icon = 0; icon < kTamaIconCount; ++icon) {
    if (icons[icon]) {
      return static_cast<int8_t>(icon);
    }
  }
  return -1;
}

void drawMenuHint(const bool* icons, bool force) {
  const int8_t selected = selectedMenuIcon(icons);
  if (!force && selected == g_last_menu_hint) {
    return;
  }
  g_last_menu_hint = selected;
  M5.Display.fillRect(4, 204, 128, 20, kBlack);
  M5.Display.setCursor(4, 207);
  if (selected >= 0) {
    M5.Display.setTextColor(selected == 7 ? kYellow : kWhite, kBlack);
    M5.Display.print(kMenuHints[selected]);
  } else {
    M5.Display.setTextColor(kGray, kBlack);
    M5.Display.print("Select a care icon");
  }
}

void drawTamaStaticFrame() {
  M5.Display.fillScreen(kBlack);

  M5.Display.setTextColor(kGreen, kBlack);
  M5.Display.setCursor(4, 4);
  M5.Display.print("Tamagotchi P1");

  g_last_brightness = UINT8_MAX;
  g_last_volume = UINT8_MAX;
  drawTamaStatus();

  M5.Display.drawRect(kTamaX - 2, kTamaY - 2, kTamaWidth * kTamaScale + 4,
                      kTamaHeight * kTamaScale + 4, kGray);
  M5.Display.fillRect(kTamaX, kTamaY, kTamaWidth * kTamaScale, kTamaHeight * kTamaScale,
                      kBlack);

  for (int icon = 0; icon < kTamaIconCount; ++icon) {
    drawMenuIcon(icon, false);
    g_last_tama_icons[icon] = false;
  }
  g_last_menu_hint = -2;
  bool empty_icons[kTamaIconCount] = {};
  drawMenuHint(empty_icons, true);

  M5.Display.setTextColor(kGray, kBlack);
  M5.Display.setCursor(4, 226);
  M5.Display.print("TamaLIB P1 local ROM");

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

void displayInvalidateTamaFrame() {
  g_tama_frame_ready = false;
  g_last_brightness = UINT8_MAX;
  g_last_volume = UINT8_MAX;
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

void displayRenderTama(const bool* pixels, const bool* icons, bool) {
  if (!g_tama_frame_ready) {
    drawTamaStaticFrame();
  }

  drawTamaStatus();

  const bool dark_room = tamaFrameLooksLikeDarkRoom(pixels);
  M5.Display.startWrite();
  for (int row = 0; row < kTamaHeight; ++row) {
    for (int col = 0; col < kTamaWidth; ++col) {
      const int index = row * kTamaWidth + col;
      const bool pixel_on = dark_room ? !pixels[index] : pixels[index];
      if (pixel_on != g_last_tama_pixels[index]) {
        const uint16_t color = pixel_on ? kTamaPixel : kBlack;
        M5.Display.fillRect(kTamaX + col * kTamaScale, kTamaY + row * kTamaScale,
                            kTamaScale, kTamaScale, color);
        g_last_tama_pixels[index] = pixel_on;
      }
    }
  }
  M5.Display.endWrite();

  for (int icon = 0; icon < kTamaIconCount; ++icon) {
    if (icons[icon] != g_last_tama_icons[icon]) {
      drawMenuIcon(icon, icons[icon]);
      g_last_tama_icons[icon] = icons[icon];
    }
  }
  drawMenuHint(icons, false);
}
