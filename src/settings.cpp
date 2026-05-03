#include "settings.h"

#include <Preferences.h>

namespace {
constexpr const char* kNamespace = "tama";
constexpr uint8_t kDefaultActiveBrightness = 128;
constexpr uint8_t kDefaultIdleBrightness = 32;
constexpr uint8_t kDefaultNightBrightness = 1;
constexpr uint8_t kDefaultVolume = 96;
constexpr uint32_t kDefaultIdleDimMs = 30000;

constexpr uint8_t kBrightnessLevels[] = {64, 128, 200};
constexpr uint8_t kVolumeLevels[] = {0, 32, 96, 160};

Preferences g_preferences;
bool g_ready = false;
uint8_t g_active_brightness = kDefaultActiveBrightness;
uint8_t g_idle_brightness = kDefaultIdleBrightness;
uint8_t g_night_brightness = kDefaultNightBrightness;
uint8_t g_volume = kDefaultVolume;
uint32_t g_idle_dim_ms = kDefaultIdleDimMs;

uint8_t nextLevel(const uint8_t* levels, size_t count, uint8_t current) {
  for (size_t i = 0; i < count; ++i) {
    if (current <= levels[i]) {
      return levels[(i + 1) % count];
    }
  }
  return levels[0];
}
}  // namespace

void settingsInit() {
  g_ready = g_preferences.begin(kNamespace, false);
  if (!g_ready) {
    Serial.println("settings: NVS open failed, using defaults");
    return;
  }

  g_active_brightness = g_preferences.getUChar("bright", kDefaultActiveBrightness);
  g_idle_brightness = g_preferences.getUChar("idleBright", kDefaultIdleBrightness);
  g_night_brightness = g_preferences.getUChar("nightBright", kDefaultNightBrightness);
  g_volume = g_preferences.getUChar("volume", kDefaultVolume);
  g_idle_dim_ms = g_preferences.getUInt("idleDimMs", kDefaultIdleDimMs);

  if (g_active_brightness < 16) {
    g_active_brightness = kDefaultActiveBrightness;
  }
  if (g_idle_brightness >= g_active_brightness) {
    g_idle_brightness = kDefaultIdleBrightness;
  }
  if (g_night_brightness > g_idle_brightness) {
    g_night_brightness = kDefaultNightBrightness;
  }
  if (g_idle_dim_ms < 5000) {
    g_idle_dim_ms = kDefaultIdleDimMs;
  }

  Serial.printf("settings: brightness=%u idle=%u night=%u volume=%u idle_dim=%lums\n",
                g_active_brightness, g_idle_brightness, g_night_brightness, g_volume,
                static_cast<unsigned long>(g_idle_dim_ms));
}

uint8_t settingsActiveBrightness() {
  return g_active_brightness;
}

uint8_t settingsIdleBrightness() {
  return g_idle_brightness;
}

uint8_t settingsNightBrightness() {
  return g_night_brightness;
}

uint8_t settingsVolume() {
  return g_volume;
}

uint32_t settingsIdleDimMs() {
  return g_idle_dim_ms;
}

uint8_t settingsCycleBrightness() {
  g_active_brightness = nextLevel(kBrightnessLevels,
                                  sizeof(kBrightnessLevels) / sizeof(kBrightnessLevels[0]),
                                  g_active_brightness);
  g_idle_brightness = g_active_brightness > 80 ? 32 : 16;
  g_night_brightness = 1;
  if (g_ready) {
    g_preferences.putUChar("bright", g_active_brightness);
    g_preferences.putUChar("idleBright", g_idle_brightness);
    g_preferences.putUChar("nightBright", g_night_brightness);
  }
  Serial.printf("settings: brightness=%u idle=%u night=%u\n", g_active_brightness,
                g_idle_brightness, g_night_brightness);
  return g_active_brightness;
}

uint8_t settingsCycleVolume() {
  g_volume = nextLevel(kVolumeLevels, sizeof(kVolumeLevels) / sizeof(kVolumeLevels[0]),
                       g_volume);
  if (g_ready) {
    g_preferences.putUChar("volume", g_volume);
  }
  Serial.printf("settings: volume=%u\n", g_volume);
  return g_volume;
}
