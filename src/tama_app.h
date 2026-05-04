#pragma once

#include <Arduino.h>

#include "imu_smoke.h"

void tamaAppInit();
void tamaAppUpdate(const ImuSample& imu);
bool tamaAppHasRom();
bool tamaAppIsRunning();
bool tamaAppIsScreenDark();
uint32_t tamaAppFastForward(uint32_t elapsed_ms);
void tamaAppPrintDebugFrame();
