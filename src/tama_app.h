#pragma once

#include "imu_smoke.h"

void tamaAppInit();
void tamaAppUpdate(const ImuSample& imu);
bool tamaAppHasRom();
bool tamaAppIsRunning();
bool tamaAppIsScreenDark();
