#include "power_manager.h"

#include <M5Unified.h>

#include "display.h"
#include "settings.h"
#include "tama_storage.h"

namespace {
constexpr uint32_t kLowBatteryCheckIntervalMs = 30000;
constexpr int16_t kLowBatteryVoltageMv = 3450;
constexpr int32_t kCriticalBatteryLevel = 5;

bool g_dimmed = false;
bool g_low_battery_latched = false;
uint32_t g_last_battery_check_ms = 0;

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
}  // namespace

void powerManagerInit() {
  g_dimmed = false;
  g_low_battery_latched = false;
  g_last_battery_check_ms = 0;
  displaySetBrightness(settingsActiveBrightness());
}

void powerManagerWake() {
  if (!g_dimmed) {
    displaySetBrightness(settingsActiveBrightness());
    return;
  }
  g_dimmed = false;
  displaySetBrightness(settingsActiveBrightness());
  Serial.println("power: display active brightness");
}

void powerManagerPrepareForSleep() {
  Serial.println("power: preparing for sleep, saving state");
  tamaStorageSave();
}

void powerManagerUpdate(bool any_pressed, uint32_t idle_age_ms) {
  checkLowBattery();

  if (any_pressed || idle_age_ms < settingsIdleDimMs()) {
    if (g_dimmed) {
      powerManagerWake();
    }
    return;
  }

  if (!g_dimmed) {
    g_dimmed = true;
    displaySetBrightness(settingsIdleBrightness());
    Serial.println("power: display idle brightness");
  }
}

bool powerManagerIsDimmed() {
  return g_dimmed;
}
