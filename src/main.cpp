#include <Arduino.h>
#include <M5Unified.h>

#include "audio.h"
#include "buttons.h"
#include "display.h"
#include "imu_smoke.h"
#include "pins.h"
#include "tama_app.h"

void setup() {
  Serial.begin(115200);
  delay(300);

  M5.begin();

  displayInit();
  audioInit();
  buttonsInit();
  imuSmokeInit();
  tamaAppInit();

  Serial.printf("boot ok: %s %s\n", board::kName, board::kFirmwareVersion);
  audioPlayBootTone();
  tamaAppUpdate(imuSmokeSample());
}

void loop() {
  M5.update();
  buttonsUpdate();
  imuSmokeUpdate();

  tamaAppUpdate(imuSmokeSample());
  delay(tamaAppIsRunning() ? 1 : 10);
}
