#pragma once

#include <Arduino.h>

void settingsInit();
uint8_t settingsActiveBrightness();
uint8_t settingsIdleBrightness();
uint8_t settingsVolume();
uint32_t settingsIdleDimMs();
uint8_t settingsCycleBrightness();
uint8_t settingsCycleVolume();
