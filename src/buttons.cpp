#include "buttons.h"

#include <Arduino.h>
#include <M5Unified.h>

#include "audio.h"

namespace {
ButtonEvent g_last_event = ButtonEvent::None;
uint32_t g_last_event_ms = 0;
bool g_combo_latched = false;
bool g_ignore_next_key1_click = false;
bool g_ignore_next_key2_click = false;
bool g_feedback_enabled = true;

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
  if (g_feedback_enabled) {
    audioPlayButtonTone();
  }
}
}  // namespace

void buttonsInit() {
  g_last_event = ButtonEvent::None;
  g_last_event_ms = 0;
  g_combo_latched = false;
  g_ignore_next_key1_click = false;
  g_ignore_next_key2_click = false;
  g_feedback_enabled = true;
}

void buttonsUpdate() {
  const bool key1 = M5.BtnA.isPressed();
  const bool key2 = M5.BtnB.isPressed();

  if (key1 && key2 && !g_combo_latched) {
    g_combo_latched = true;
    g_ignore_next_key1_click = true;
    g_ignore_next_key2_click = true;
    publish(ButtonEvent::C);
    return;
  }

  if (g_combo_latched) {
    if (!key1 && !key2) {
      g_combo_latched = false;
    }
    return;
  }

  if (M5.BtnB.wasHold()) {
    g_ignore_next_key2_click = true;
    publish(ButtonEvent::AiWake);
  }

  if (M5.BtnA.wasHold()) {
    g_ignore_next_key1_click = true;
    publish(ButtonEvent::Menu);
  }

  if (M5.BtnA.wasClicked()) {
    if (g_ignore_next_key1_click) {
      g_ignore_next_key1_click = false;
    } else {
      publish(ButtonEvent::A);
    }
  }

  if (M5.BtnB.wasClicked()) {
    if (g_ignore_next_key2_click) {
      g_ignore_next_key2_click = false;
    } else {
      publish(ButtonEvent::B);
    }
  }
}

void buttonsSetFeedbackEnabled(bool enabled) {
  g_feedback_enabled = enabled;
}

uint8_t buttonsTamaMask() {
  const bool key1 = M5.BtnA.isPressed();
  const bool key2 = M5.BtnB.isPressed();

  if (key1 && key2) {
    return kTamaButtonC;
  }

  uint8_t mask = 0;
  if (key1) {
    mask |= kTamaButtonA;
  }
  if (key2) {
    mask |= kTamaButtonB;
  }
  return mask;
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
