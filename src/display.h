#pragma once

#include <Arduino.h>

#include "imu_smoke.h"

void displayInit();
void displaySetBrightness(uint8_t brightness);
void displayInvalidateTamaFrame();
void displayRender(const ImuSample& imu);
void displayRenderMissingRom(const ImuSample& imu, bool init_failed);
void displayRenderTama(const bool* pixels, const bool* icons, bool sound_on);
