#pragma once

#include <Arduino.h>

enum class ButtonEvent {
  None,
  A,
  B,
  C,
  AiWake,
  Menu,
};

void buttonsInit();
void buttonsUpdate();
ButtonEvent buttonsLastEvent();
const char* buttonsLastEventName();
uint32_t buttonsLastEventAgeMs();

