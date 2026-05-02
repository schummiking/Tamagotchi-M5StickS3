#pragma once

#include <Arduino.h>

void powerManagerInit();
void powerManagerWake();
void powerManagerPrepareForSleep();
void powerManagerUpdate(bool any_pressed, uint32_t idle_age_ms);
bool powerManagerIsDimmed();
