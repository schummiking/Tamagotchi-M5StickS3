#pragma once

#include "imu_smoke.h"

void displayInit();
void displayRender(const ImuSample& imu);
void displayRenderMissingRom(const ImuSample& imu, bool init_failed);
void displayRenderTama(const bool* pixels, const bool* icons, bool sound_on);
