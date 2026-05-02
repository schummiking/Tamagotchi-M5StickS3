#include "audio.h"

#include <M5Unified.h>

void audioInit() {
  M5.Speaker.setVolume(96);
}

void audioPlayBootTone() {
  M5.Speaker.tone(440.0f, 120);
}

void audioPlayButtonTone() {
  M5.Speaker.tone(880.0f, 45);
}

