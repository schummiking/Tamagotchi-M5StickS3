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

constexpr uint8_t kTamaButtonA = 0x01;
constexpr uint8_t kTamaButtonB = 0x02;
constexpr uint8_t kTamaButtonC = 0x04;

void buttonsInit();
void buttonsUpdate();
void buttonsSetFeedbackEnabled(bool enabled);
uint8_t buttonsTamaMask();
bool buttonsIsAnyPressed();
ButtonEvent buttonsLastEvent();
const char* buttonsLastEventName();
uint32_t buttonsLastEventAgeMs();
uint32_t buttonsLastActivityAgeMs();
uint32_t buttonsLastEventSeq();
