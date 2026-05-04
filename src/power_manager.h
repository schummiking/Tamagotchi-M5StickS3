#pragma once

#include <Arduino.h>

void powerManagerInit();
void powerManagerWake();
void powerManagerPrepareForSleep();
bool powerManagerUpdate(bool any_pressed, uint32_t idle_age_ms, bool game_screen_dark);
bool powerManagerIsDimmed();
bool powerManagerIsDisplaySleeping();
bool powerManagerHandleWakeCatchup();
bool powerManagerRunLowPowerTest(uint32_t sleep_ms);
void powerManagerPrintDiagnostics();
