#include <Arduino.h>
#include <M5Unified.h>

#include "audio.h"
#include "buttons.h"
#include "display.h"
#include "imu_smoke.h"
#include "pins.h"
#include "power_manager.h"
#include "settings.h"
#include "tama_app.h"

namespace {
void handleSettingsShortcuts() {
  static uint32_t seen_event_seq = 0;
  const uint32_t event_seq = buttonsLastEventSeq();
  if (event_seq == seen_event_seq) {
    return;
  }
  seen_event_seq = event_seq;

  switch (buttonsLastEvent()) {
    case ButtonEvent::Menu:
      settingsCycleBrightness();
      powerManagerWake();
      audioPlayButtonTone();
      break;
    case ButtonEvent::AiWake:
      audioSetVolume(settingsCycleVolume());
      audioPlayButtonTone();
      break;
    default:
      break;
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);

  auto m5_config = M5.config();
  m5_config.fallback_board = m5::board_t::board_M5StickS3;
  M5.begin(m5_config);

  settingsInit();
  displayInit();
  displaySetBrightness(settingsActiveBrightness());
  audioInit(settingsVolume());
  buttonsInit();
  powerManagerInit();
  imuSmokeInit();
  tamaAppInit();

  Serial.printf("boot ok: %s %s\n", board::kName, board::kFirmwareVersion);
  audioPlayBootTone();
  tamaAppUpdate(imuSmokeSample());
}

void loop() {
  M5.update();
  buttonsUpdate();
  handleSettingsShortcuts();
  imuSmokeUpdate();

  tamaAppUpdate(imuSmokeSample());
  powerManagerUpdate(buttonsIsAnyPressed(), buttonsLastActivityAgeMs());
  delay(tamaAppIsRunning() ? 1 : 10);
}
