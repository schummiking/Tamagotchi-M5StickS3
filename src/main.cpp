#include <Arduino.h>
#include <M5Unified.h>

#include "audio.h"
#include "buttons.h"
#include "display.h"
#include "imu_smoke.h"
#include "pins.h"

namespace {
uint32_t g_last_render_ms = 0;
}

void setup() {
  Serial.begin(115200);
  delay(300);

  M5.begin();

  displayInit();
  audioInit();
  buttonsInit();
  imuSmokeInit();

  Serial.printf("boot ok: %s %s\n", board::kName, board::kSmokeTestVersion);
  audioPlayBootTone();
  displayRender(imuSmokeSample());
}

void loop() {
  M5.update();
  buttonsUpdate();
  imuSmokeUpdate();

  if (millis() - g_last_render_ms >= 250 || buttonsLastEventAgeMs() < 300) {
    g_last_render_ms = millis();
    displayRender(imuSmokeSample());
  }

  delay(10);
}

