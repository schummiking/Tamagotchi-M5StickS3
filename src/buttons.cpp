#include "buttons.h"

#include <Arduino.h>
#include <M5Unified.h>

#include "audio.h"

namespace {
constexpr uint32_t kHoldThresholdMs = 650;
constexpr uint32_t kTamaNavPulseMs = 220;
constexpr uint32_t kTamaConfirmPulseMs = 520;

ButtonEvent g_last_event = ButtonEvent::None;
uint32_t g_last_event_ms = 0;
uint32_t g_last_activity_ms = 0;
uint32_t g_last_event_seq = 0;
uint32_t g_key1_down_ms = 0;
uint32_t g_key2_down_ms = 0;
uint32_t g_tama_pulse_until_ms = 0;
uint8_t g_tama_pulse_mask = 0;
uint8_t g_tama_mask = 0;
uint8_t g_injected_mask = 0;
uint32_t g_injected_until_ms = 0;
bool g_combo_latched = false;
uint8_t g_combo_mask = 0;
bool g_feedback_enabled = true;
bool g_suppress_until_release = false;
bool g_last_key1 = false;
bool g_last_key2 = false;

const char* eventName(ButtonEvent event) {
  switch (event) {
    case ButtonEvent::A:
      return "A";
    case ButtonEvent::B:
      return "B";
    case ButtonEvent::C:
      return "C";
    case ButtonEvent::AC:
      return "A+C";
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
  g_last_activity_ms = g_last_event_ms;
  ++g_last_event_seq;
  Serial.printf("button event: %s\n", eventName(event));
  if (g_feedback_enabled) {
    audioPlayButtonTone();
  }
}

bool timeReached(uint32_t deadline_ms) {
  return static_cast<int32_t>(millis() - deadline_ms) >= 0;
}

void emitTamaPulse(uint8_t mask, uint32_t duration_ms) {
  g_tama_pulse_mask = mask;
  g_tama_pulse_until_ms = millis() + duration_ms;
  g_tama_mask = mask;
}

void clearTamaPulse() {
  g_tama_pulse_mask = 0;
  g_tama_pulse_until_ms = 0;
}

void refreshInjectedMask() {
  if (g_injected_mask != 0 && timeReached(g_injected_until_ms)) {
    g_injected_mask = 0;
    g_injected_until_ms = 0;
  }
}

void refreshTamaMask(bool key1, bool key2) {
  if (g_combo_latched) {
    g_tama_mask = (key1 && key2) ? g_combo_mask : 0;
    return;
  }

  if (g_tama_pulse_mask == 0 || timeReached(g_tama_pulse_until_ms)) {
    clearTamaPulse();
    g_tama_mask = 0;
    return;
  }

  g_tama_mask = g_tama_pulse_mask;
}

void publishReleasedKey(ButtonEvent click_event, ButtonEvent hold_event, uint8_t tama_mask,
                        uint32_t down_ms, uint32_t pulse_ms) {
  if (millis() - down_ms >= kHoldThresholdMs) {
    publish(hold_event);
  } else {
    publish(click_event);
    emitTamaPulse(tama_mask, pulse_ms);
  }
}
}  // namespace

void buttonsInit() {
  g_last_event = ButtonEvent::None;
  g_last_event_ms = 0;
  g_last_activity_ms = millis();
  g_last_event_seq = 0;
  g_key1_down_ms = 0;
  g_key2_down_ms = 0;
  clearTamaPulse();
  g_tama_mask = 0;
  g_combo_latched = false;
  g_combo_mask = 0;
  g_feedback_enabled = true;
  g_suppress_until_release = false;
  g_last_key1 = M5.BtnA.isPressed();
  g_last_key2 = M5.BtnB.isPressed();
  const uint32_t now = millis();
  if (g_last_key1) {
    g_key1_down_ms = now;
  }
  if (g_last_key2) {
    g_key2_down_ms = now;
  }
}

void buttonsUpdate() {
  const uint32_t now = millis();
  refreshInjectedMask();

  const bool key1 = M5.BtnA.isPressed();
  const bool key2 = M5.BtnB.isPressed();

  if (key1 != g_last_key1 || key2 != g_last_key2) {
    g_last_activity_ms = now;
  }

  if (key1 && !g_last_key1) {
    g_key1_down_ms = now;
  }
  if (key2 && !g_last_key2) {
    g_key2_down_ms = now;
  }

  if (g_suppress_until_release) {
    clearTamaPulse();
    g_tama_mask = 0;
    if (!key1 && !key2) {
      g_suppress_until_release = false;
    }
    g_last_key1 = key1;
    g_last_key2 = key2;
    return;
  }

  if (key1 && key2 && !g_combo_latched) {
    g_combo_latched = true;
    const bool key2_first =
        g_key2_down_ms != 0 && g_key1_down_ms != 0 &&
        static_cast<int32_t>(g_key1_down_ms - g_key2_down_ms) > 0;
    g_combo_mask = key2_first ? (kTamaButtonA | kTamaButtonC) : kTamaButtonC;
    clearTamaPulse();
    publish(key2_first ? ButtonEvent::AC : ButtonEvent::C);
  }

  if (g_combo_latched) {
    refreshTamaMask(key1, key2);
    if (!key1 && !key2) {
      g_combo_latched = false;
      g_combo_mask = 0;
    }
  } else {
    if (!key1 && g_last_key1) {
      publishReleasedKey(ButtonEvent::A, ButtonEvent::Menu, kTamaButtonA, g_key1_down_ms,
                         kTamaNavPulseMs);
    }
    if (!key2 && g_last_key2) {
      publishReleasedKey(ButtonEvent::B, ButtonEvent::AiWake, kTamaButtonB, g_key2_down_ms,
                         kTamaConfirmPulseMs);
    }
    refreshTamaMask(key1, key2);
  }

  g_last_key1 = key1;
  g_last_key2 = key2;
}

void buttonsSetFeedbackEnabled(bool enabled) {
  g_feedback_enabled = enabled;
}

void buttonsSuppressUntilRelease() {
  g_suppress_until_release = true;
  clearTamaPulse();
  g_tama_mask = 0;
}

void buttonsInjectTamaMask(uint8_t mask, uint32_t duration_ms) {
  if (mask == 0 || duration_ms == 0) {
    g_injected_mask = 0;
    g_injected_until_ms = 0;
    return;
  }
  g_injected_mask = mask;
  g_injected_until_ms = millis() + duration_ms;
  g_last_activity_ms = millis();
}

uint8_t buttonsTamaMask() {
  refreshInjectedMask();
  if (g_injected_mask != 0) {
    return g_injected_mask;
  }
  return g_tama_mask;
}

bool buttonsIsAnyPressed() {
  refreshInjectedMask();
  return g_injected_mask != 0 || g_tama_pulse_mask != 0 || M5.BtnA.isPressed() ||
         M5.BtnB.isPressed();
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

uint32_t buttonsLastActivityAgeMs() {
  return millis() - g_last_activity_ms;
}

uint32_t buttonsLastEventSeq() {
  return g_last_event_seq;
}
