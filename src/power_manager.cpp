#include "power_manager.h"

#include <M5Unified.h>

#include "display.h"
#include "settings.h"
#include "tama_storage.h"

namespace {
constexpr uint32_t kLowBatteryCheckIntervalMs = 30000;
constexpr uint32_t kLightOffDimDelayMs = 1200;
constexpr uint32_t kDarkScreenSleepDelayMs = 60000;
constexpr uint32_t kIdleDisplaySleepDelayMs = 600000;
constexpr int16_t kLowBatteryVoltageMv = 3450;
constexpr int32_t kCriticalBatteryLevel = 5;

bool g_dimmed = false;
bool g_night_dimmed = false;
bool g_display_sleeping = false;
bool g_low_battery_latched = false;
uint32_t g_last_battery_check_ms = 0;
uint32_t g_screen_dark_since_ms = 0;

void checkLowBattery() {
  const uint32_t now = millis();
  if (g_last_battery_check_ms != 0 &&
      now - g_last_battery_check_ms < kLowBatteryCheckIntervalMs) {
    return;
  }
  g_last_battery_check_ms = now;

  const int16_t voltage_mv = M5.Power.getBatteryVoltage();
  const int32_t level = M5.Power.getBatteryLevel();
  const bool voltage_low = voltage_mv > 0 && voltage_mv < kLowBatteryVoltageMv;
  const bool level_low = level >= 0 && level <= kCriticalBatteryLevel && voltage_mv > 0 &&
                         voltage_mv < 3600;
  const bool low = voltage_low || level_low;

  if (low && !g_low_battery_latched) {
    g_low_battery_latched = true;
    Serial.printf("power: low battery level=%ld voltage=%dmV, saving state\n",
                  static_cast<long>(level), voltage_mv);
    tamaStorageSave();
  } else if (!low) {
    g_low_battery_latched = false;
  }
}

void enterDisplaySleep(const char* reason) {
  if (g_display_sleeping) {
    return;
  }
  g_display_sleeping = true;
  g_dimmed = false;
  g_night_dimmed = false;
  displaySetBrightness(0);
  Serial.printf("power: display sleep (%s)\n", reason);
}
}  // namespace

void powerManagerInit() {
  g_dimmed = false;
  g_night_dimmed = false;
  g_display_sleeping = false;
  g_low_battery_latched = false;
  g_last_battery_check_ms = 0;
  g_screen_dark_since_ms = 0;
  displaySetBrightness(settingsActiveBrightness());
}

void powerManagerWake() {
  if (!g_dimmed && !g_night_dimmed && !g_display_sleeping) {
    displaySetBrightness(settingsActiveBrightness());
    return;
  }
  g_dimmed = false;
  g_night_dimmed = false;
  g_display_sleeping = false;
  displaySetBrightness(settingsActiveBrightness());
  Serial.println("power: display active brightness");
}

void powerManagerPrepareForSleep() {
  Serial.println("power: preparing for sleep, saving state");
  tamaStorageSave();
}

bool powerManagerUpdate(bool any_pressed, uint32_t idle_age_ms, bool game_screen_dark) {
  checkLowBattery();

  if (any_pressed) {
    g_screen_dark_since_ms = 0;
    if (g_dimmed || g_night_dimmed || g_display_sleeping) {
      powerManagerWake();
      return true;
    }
    return false;
  }

  if (g_display_sleeping) {
    return false;
  }

  const uint32_t now = millis();
  if (game_screen_dark) {
    if (g_screen_dark_since_ms == 0) {
      g_screen_dark_since_ms = now;
    }
    if (!g_night_dimmed && now - g_screen_dark_since_ms >= kLightOffDimDelayMs) {
      g_dimmed = false;
      g_night_dimmed = true;
      displaySetBrightness(settingsNightBrightness());
      Serial.println("power: display night brightness");
    }
    if (now - g_screen_dark_since_ms >= kDarkScreenSleepDelayMs) {
      enterDisplaySleep("night");
    }
    return false;
  }

  g_screen_dark_since_ms = 0;
  if (g_night_dimmed) {
    powerManagerWake();
  }

  if (idle_age_ms < settingsIdleDimMs()) {
    return false;
  }

  if (!g_dimmed) {
    g_dimmed = true;
    displaySetBrightness(settingsIdleBrightness());
    Serial.println("power: display idle brightness");
  }

  if (idle_age_ms >= kIdleDisplaySleepDelayMs) {
    enterDisplaySleep("idle");
  }
  return false;
}

bool powerManagerIsDimmed() {
  return g_dimmed || g_night_dimmed || g_display_sleeping;
}

bool powerManagerIsDisplaySleeping() {
  return g_display_sleeping;
}
