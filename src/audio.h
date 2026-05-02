#pragma once

#include <Arduino.h>

void audioInit(uint8_t volume);
void audioSetVolume(uint8_t volume);
uint8_t audioVolume();
void audioPlayBootTone();
void audioPlayButtonTone();
