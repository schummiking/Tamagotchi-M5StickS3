#include "power_manager.h"

#include <M5Unified.h>
#include <driver/gpio.h>
#include <esp_sleep.h>
#include <Preferences.h>

#include "audio.h"
#include "display.h"
#include "settings.h"
#include "system_led.h"
#include "tama_app.h"
#include "tama_storage.h"

namespace {
constexpr uint32_t kLowBatteryCheckIntervalMs = 30000;
constexpr uint32_t kLightOffDimDelayMs = 1200;
constexpr uint32_t kDarkScreenSleepDelayMs = 60000;
constexpr uint32_t kNightLowPowerDelayMs = 180000;
constexpr uint32_t kIdleDisplaySleepDelayMs = 600000;
constexpr uint32_t kIdleLowPowerDelayMs = 180000;
constexpr int16_t kLowBatteryVoltageMv = 3450;
constexpr int32_t kCriticalBatteryLevel = 5;
constexpr uint64_t kLowPowerChunkUs = 10ULL * 60ULL * 1000ULL * 1000ULL;
constexpr uint32_t kMinTestSleepMs = 1000;
constexpr uint32_t kMaxTestSleepMs = 120000;
constexpr uint32_t kSleepMagic = 0x53504C54;  // TLPS
constexpr const char* kPowerNamespace = "tama_pwr";
constexpr const char* kSleepPendingKey = "slpPending";
constexpr const char* kSleepMsKey = "slpMs";
constexpr const char* kSleepSeqKey = "slpSeq";
constexpr const char* kSleepChainKey = "slpChain";
constexpr const char* kLastCatchupMsKey = "lastCatchMs";
constexpr const char* kLastWakeCauseKey = "lastWake";
constexpr const char* kLastCatchupSeqKey = "lastSeq";
constexpr gpio_num_t kWakeKey1 = GPIO_NUM_11;
constexpr gpio_num_t kWakeKey2 = GPIO_NUM_12;
constexpr uint64_t kWakeButtonMask = (1ULL << GPIO_NUM_11) | (1ULL << GPIO_NUM_12);

RTC_DATA_ATTR uint32_t g_rtc_sleep_magic = 0;
RTC_DATA_ATTR uint32_t g_rtc_sleep_ms = 0;
RTC_DATA_ATTR uint32_t g_rtc_sleep_seq = 0;
RTC_DATA_ATTR uint32_t g_rtc_sleep_chain = 0;

bool g_dimmed = false;
bool g_night_dimmed = false;
bool g_display_sleeping = false;
bool g_low_battery_latched = false;
bool g_resume_low_power_soon = false;
uint32_t g_last_battery_check_ms = 0;
uint32_t g_screen_dark_since_ms = 0;

void writeSleepJournal(uint32_t sleep_ms, bool chain_after_wake) {
  Preferences prefs;
  if (!prefs.begin(kPowerNamespace, false)) {
    Serial.println("power: sleep journal open failed");
    return;
  }
  prefs.putBool(kSleepPendingKey, true);
  prefs.putUInt(kSleepMsKey, sleep_ms);
  prefs.putUInt(kSleepSeqKey, g_rtc_sleep_seq);
  prefs.putBool(kSleepChainKey, chain_after_wake);
  prefs.end();
  Serial.printf("power: sleep journal set ms=%lu seq=%lu chain=%d\n",
                static_cast<unsigned long>(sleep_ms),
                static_cast<unsigned long>(g_rtc_sleep_seq),
                chain_after_wake ? 1 : 0);
}

bool readSleepJournal(uint32_t& sleep_ms, uint32_t& sleep_seq, bool& chain_after_wake) {
  Preferences prefs;
  if (!prefs.begin(kPowerNamespace, true)) {
    return false;
  }
  const bool pending = prefs.getBool(kSleepPendingKey, false);
  sleep_ms = prefs.getUInt(kSleepMsKey, 0);
  sleep_seq = prefs.getUInt(kSleepSeqKey, 0);
  chain_after_wake = prefs.getBool(kSleepChainKey, false);
  prefs.end();
  return pending && sleep_ms > 0;
}

void clearSleepJournal() {
  Preferences prefs;
  if (!prefs.begin(kPowerNamespace, false)) {
    Serial.println("power: sleep journal clear open failed");
    return;
  }
  prefs.putBool(kSleepPendingKey, false);
  prefs.putUInt(kSleepMsKey, 0);
  prefs.putBool(kSleepChainKey, false);
  prefs.end();
  Serial.println("power: sleep journal cleared");
}

void writeCatchupDiagnostics(uint32_t catchup_ms, esp_sleep_wakeup_cause_t wake_cause,
                             uint32_t sleep_seq) {
  Preferences prefs;
  if (!prefs.begin(kPowerNamespace, false)) {
    Serial.println("power: catchup diag open failed");
    return;
  }
  prefs.putUInt(kLastCatchupMsKey, catchup_ms);
  prefs.putUInt(kLastWakeCauseKey, static_cast<uint32_t>(wake_cause));
  prefs.putUInt(kLastCatchupSeqKey, sleep_seq);
  prefs.end();
}

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
  systemLedEnterDisplaySleep();
  Serial.printf("power: display standby (%s)\n", reason);
}

bool isWakeButtonPressed() {
  return gpio_get_level(kWakeKey1) == 0 || gpio_get_level(kWakeKey2) == 0;
}

void enterLowPowerSleepChunk(const char* reason, uint64_t chunk_us = kLowPowerChunkUs,
                             bool chain_after_wake = true) {
  Serial.printf("power: low power sleep prepare (%s)\n", reason);
  tamaStorageSave();
  audioStop();
  displaySetBrightness(0);
  systemLedEnterLowPowerSleep();

  g_rtc_sleep_magic = kSleepMagic;
  g_rtc_sleep_ms = static_cast<uint32_t>(chunk_us / 1000ULL);
  ++g_rtc_sleep_seq;
  g_rtc_sleep_chain = chain_after_wake ? 1 : 0;
  writeSleepJournal(g_rtc_sleep_ms, chain_after_wake);

  esp_sleep_enable_timer_wakeup(chunk_us);
  esp_sleep_enable_ext1_wakeup(kWakeButtonMask, ESP_EXT1_WAKEUP_ANY_LOW);

  Serial.printf("power: deep sleep enter chunk=%lluus seq=%lu chain=%d\n", chunk_us,
                static_cast<unsigned long>(g_rtc_sleep_seq), chain_after_wake ? 1 : 0);
  Serial.flush();
  esp_deep_sleep_start();
  while (true) {
    delay(1000);
  }
}
}  // namespace

void powerManagerInit() {
  g_dimmed = false;
  g_night_dimmed = false;
  g_display_sleeping = false;
  g_low_battery_latched = false;
  g_resume_low_power_soon = false;
  g_last_battery_check_ms = 0;
  g_screen_dark_since_ms = 0;
  systemLedInit();
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
  systemLedLeaveDisplaySleep();
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
    g_resume_low_power_soon = false;
    if (g_dimmed || g_night_dimmed || g_display_sleeping) {
      powerManagerWake();
      return true;
    }
    return false;
  }

  const uint32_t now = millis();

  if (g_resume_low_power_soon) {
    g_resume_low_power_soon = false;
    enterLowPowerSleepChunk("auto-chain");
  }

  if (g_display_sleeping) {
    if (game_screen_dark && g_screen_dark_since_ms != 0 &&
        now - g_screen_dark_since_ms >= kNightLowPowerDelayMs) {
      enterLowPowerSleepChunk("night");
    }
    if (!game_screen_dark &&
        idle_age_ms >= kIdleDisplaySleepDelayMs + kIdleLowPowerDelayMs) {
      enterLowPowerSleepChunk("idle");
    }
    return false;
  }

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
  g_resume_low_power_soon = false;
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

bool powerManagerHandleWakeCatchup() {
  const esp_sleep_wakeup_cause_t wake_cause = esp_sleep_get_wakeup_cause();
  uint32_t pending_ms = 0;
  uint32_t pending_seq = 0;
  bool pending_chain = false;
  const bool rtc_pending = g_rtc_sleep_magic == kSleepMagic && g_rtc_sleep_ms > 0;
  const bool journal_pending = readSleepJournal(pending_ms, pending_seq, pending_chain);
  if (rtc_pending) {
    pending_ms = g_rtc_sleep_ms;
    pending_seq = g_rtc_sleep_seq;
    pending_chain = g_rtc_sleep_chain != 0;
  }
  const bool has_pending_sleep = rtc_pending || journal_pending;
  Serial.printf("power: wake diag cause=%d pending=%d rtc=%d journal=%d pending_ms=%lu seq=%lu chain=%d\n",
                static_cast<int>(wake_cause), has_pending_sleep ? 1 : 0,
                rtc_pending ? 1 : 0, journal_pending ? 1 : 0,
                static_cast<unsigned long>(pending_ms),
                static_cast<unsigned long>(pending_seq), pending_chain ? 1 : 0);

  if (!has_pending_sleep) {
    return false;
  }

  g_rtc_sleep_magic = 0;
  g_rtc_sleep_ms = 0;
  g_rtc_sleep_chain = 0;
  clearSleepJournal();

  if (wake_cause != ESP_SLEEP_WAKEUP_TIMER) {
    writeCatchupDiagnostics(0, wake_cause, pending_seq);
    g_resume_low_power_soon = false;
    Serial.println("power: catchup skipped, woke before timer");
    return false;
  }

  Serial.printf("power: catchup from deep sleep %lums\n",
                static_cast<unsigned long>(pending_ms));
  audioStop();
  displaySetBrightness(0);
  const uint32_t advanced_ms = tamaAppFastForward(pending_ms);
  writeCatchupDiagnostics(advanced_ms, wake_cause, pending_seq);
  tamaStorageSave();
  g_resume_low_power_soon = pending_chain;
  if (!pending_chain) {
    powerManagerWake();
  }
  return true;
}

bool powerManagerRunLowPowerTest(uint32_t sleep_ms) {
  if (sleep_ms < kMinTestSleepMs) {
    sleep_ms = kMinTestSleepMs;
  }
  if (sleep_ms > kMaxTestSleepMs) {
    sleep_ms = kMaxTestSleepMs;
  }
  enterLowPowerSleepChunk("serial-test", static_cast<uint64_t>(sleep_ms) * 1000ULL, false);
  return false;
}

void powerManagerPrintDiagnostics() {
  Preferences prefs;
  bool pending = false;
  uint32_t pending_ms = 0;
  uint32_t pending_seq = 0;
  bool pending_chain = false;
  uint32_t last_catchup_ms = 0;
  uint32_t last_wake = 0;
  uint32_t last_seq = 0;
  if (prefs.begin(kPowerNamespace, true)) {
    pending = prefs.getBool(kSleepPendingKey, false);
    pending_ms = prefs.getUInt(kSleepMsKey, 0);
    pending_seq = prefs.getUInt(kSleepSeqKey, 0);
    pending_chain = prefs.getBool(kSleepChainKey, false);
    last_catchup_ms = prefs.getUInt(kLastCatchupMsKey, 0);
    last_wake = prefs.getUInt(kLastWakeCauseKey, 0);
    last_seq = prefs.getUInt(kLastCatchupSeqKey, 0);
    prefs.end();
  }
  Serial.printf("power: diag display_sleep=%d dimmed=%d pending=%d pending_ms=%lu pending_seq=%lu pending_chain=%d last_catchup_ms=%lu last_wake=%lu last_seq=%lu\n",
                g_display_sleeping ? 1 : 0, powerManagerIsDimmed() ? 1 : 0,
                pending ? 1 : 0, static_cast<unsigned long>(pending_ms),
                static_cast<unsigned long>(pending_seq), pending_chain ? 1 : 0,
                static_cast<unsigned long>(last_catchup_ms),
                static_cast<unsigned long>(last_wake),
                static_cast<unsigned long>(last_seq));
}
