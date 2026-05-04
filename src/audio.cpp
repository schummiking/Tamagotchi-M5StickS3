#include "audio.h"

#include <Arduino.h>
#include <M5Unified.h>

namespace {
uint8_t g_volume = 96;
}

void audioInit(uint8_t volume) {
  audioSetVolume(volume);
}

void audioSetVolume(uint8_t volume) {
  g_volume = volume;
  M5.Speaker.setVolume(g_volume);
  if (g_volume == 0) {
    M5.Speaker.stop();
  }
}

uint8_t audioVolume() {
  return g_volume;
}

void audioStop() {
  M5.Speaker.stop();
}

void audioPlayBootTone() {
  if (g_volume == 0) {
    return;
  }
  M5.Speaker.tone(440.0f, 120);
}

void audioPlayButtonTone() {
  if (g_volume == 0) {
    return;
  }
  M5.Speaker.tone(880.0f, 45);
}
