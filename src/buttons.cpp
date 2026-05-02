#include "buttons.h"

#include <Arduino.h>
#include <M5Unified.h>

#include "audio.h"

namespace {
ButtonEvent g_last_event = ButtonEvent::None;
uint32_t g_last_event_ms = 0;
bool g_left_long_reported = false;
bool g_right_long_reported = false;
bool g_combo_reported = false;

const char* eventName(ButtonEvent event) {
  switch (event) {
    case ButtonEvent::A:
      return "A";
    case ButtonEvent::B:
      return "B";
    case ButtonEvent::C:
      return "C";
    case ButtonEvent::AiWake:
      return "AI_WAKE";
    case ButtonEvent::Menu:
      return "MENU";
    case ButtonEvent::None:
    default:
      return "NONE";
  }
}

void publish(ButtonEvent event) {
  g_last_event = event;
  g_last_event_ms = millis();
  Serial.printf("button event: %s\n", eventName(event));
  audioPlayButtonTone();
}
}  // namespace

void buttonsInit() {
  g_last_event = ButtonEvent::None;
  g_last_event_ms = 0;
  g_left_long_reported = false;
  g_right_long_reported = false;
  g_combo_reported = false;
}

void buttonsUpdate() {
  const bool left = M5.BtnA.isPressed();
  const bool right = M5.BtnB.isPressed();

  if (left && right) {
    if (!g_combo_reported) {
      g_combo_reported = true;
      publish(ButtonEvent::AiWake);
    }
    return;
  }

  if (!left && !right) {
    g_combo_reported = false;
  }

  if (M5.BtnA.wasPressed()) {
    g_left_long_reported = false;
  }
  if (M5.BtnB.wasPressed()) {
    g_right_long_reported = false;
  }

  if (left && !g_left_long_reported && M5.BtnA.pressedFor(600)) {
    g_left_long_reported = true;
    publish(ButtonEvent::C);
  }

  if (right && !g_right_long_reported && M5.BtnB.pressedFor(1500)) {
    g_right_long_reported = true;
    publish(ButtonEvent::Menu);
  }

  if (M5.BtnA.wasReleased() && !g_left_long_reported) {
    publish(ButtonEvent::A);
  }

  if (M5.BtnB.wasReleased() && !g_right_long_reported) {
    publish(ButtonEvent::B);
  }
}

ButtonEvent buttonsLastEvent() {
  return g_last_event;
}

const char* buttonsLastEventName() {
  return eventName(g_last_event);
}

uint32_t buttonsLastEventAgeMs() {
  if (g_last_event == ButtonEvent::None) {
    return UINT32_MAX;
  }
  return millis() - g_last_event_ms;
}

