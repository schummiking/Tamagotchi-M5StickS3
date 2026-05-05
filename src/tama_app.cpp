#include "tama_app.h"

#include <Arduino.h>
#include <M5Unified.h>
#include <stdarg.h>
#include <stdlib.h>

#include "buttons.h"
#include "audio.h"
#include "display.h"
#include "tama_frame.h"
#include "tama_storage.h"

extern "C" {
#include "hal.h"
#include "hw.h"
#include "tamalib.h"
}

#if __has_include("../data/rom.h")
#include "../data/rom.h"
#define M5STICKS3_HAS_LOCAL_ROM 1
#else
#define M5STICKS3_HAS_LOCAL_ROM 0
#endif

namespace {
constexpr uint8_t kLcdWidth = kTamaFrameWidth;
constexpr uint8_t kLcdHeight = kTamaFrameHeight;
constexpr uint8_t kIconCount = 8;
constexpr uint32_t kFrameIntervalMs = 33;
constexpr uint32_t kStepBudgetUs = 7000;
constexpr uint16_t kMaxStepsPerLoop = 512;
constexpr uint32_t kTamaTickFrequency = 32768;
constexpr uint32_t kMaxCatchupElapsedMs = 10UL * 60UL * 1000UL;
constexpr uint32_t kMaxCatchupWallMs = 35000;

bool g_pixels[kLcdHeight][kLcdWidth] = {};
bool g_icons[kIconCount] = {};
bool g_running = false;
bool g_init_failed = false;
bool g_sound_enabled = false;
bool g_fast_forwarding = false;
uint32_t g_frequency_dhz = 40960;
uint32_t g_last_frame_ms = 0;
uint8_t g_last_button_mask = 0;

void* halMalloc(u32_t size) {
  return malloc(size);
}

void halFree(void* ptr) {
  free(ptr);
}

void halHalt() {
  Serial.println("tamalib: cpu halted");
}

bool_t halIsLogEnabled(log_level_t level) {
  return level == LOG_ERROR;
}

void halLog(log_level_t level, char* format, ...) {
  if (!halIsLogEnabled(level)) {
    return;
  }

  Serial.print("tamalib: ");
  va_list args;
  va_start(args, format);
  char buffer[160];
  vsnprintf(buffer, sizeof(buffer), format, args);
  Serial.print(buffer);
  va_end(args);
}

timestamp_t halGetTimestamp() {
  return static_cast<timestamp_t>(micros());
}

void halSleepUntil(timestamp_t ts) {
  const timestamp_t now = halGetTimestamp();
  const int32_t delta = static_cast<int32_t>(ts - now);
  if (delta <= 0) {
    return;
  }
  if (delta > 2000) {
    delay(static_cast<uint32_t>(delta) / 1000);
  } else {
    delayMicroseconds(static_cast<uint32_t>(delta));
  }
}

void halUpdateScreen() {
}

void halSetLcdMatrix(u8_t x, u8_t y, bool_t val) {
  if (x < kLcdWidth && y < kLcdHeight) {
    g_pixels[y][x] = val != 0;
  }
}

void halSetLcdIcon(u8_t icon, bool_t val) {
  if (icon < kIconCount) {
    g_icons[icon] = val != 0;
  }
}

void halSetFrequency(u32_t freq_dhz) {
  g_frequency_dhz = freq_dhz;
  if (g_sound_enabled && audioVolume() > 0 && !g_fast_forwarding) {
    M5.Speaker.tone(static_cast<float>(g_frequency_dhz) / 10.0f);
  }
}

void halPlayFrequency(bool_t enabled) {
  g_sound_enabled = enabled != 0;
  if (g_sound_enabled && audioVolume() > 0 && !g_fast_forwarding) {
    M5.Speaker.tone(static_cast<float>(g_frequency_dhz) / 10.0f);
  } else {
    M5.Speaker.stop();
  }
}

void setTamaButton(button_t button, bool pressed) {
  tamalib_set_button(button, pressed ? BTN_STATE_PRESSED : BTN_STATE_RELEASED);
}

void syncButtonsToTama() {
  const uint8_t mask = buttonsTamaMask();
  if (mask == g_last_button_mask) {
    return;
  }

  setTamaButton(BTN_LEFT, (mask & kTamaButtonA) != 0);
  setTamaButton(BTN_MIDDLE, (mask & kTamaButtonB) != 0);
  setTamaButton(BTN_RIGHT, (mask & kTamaButtonC) != 0);
  g_last_button_mask = mask;
  if (g_running) {
    tamaStorageMarkDirty();
  }
}

int halHandler() {
  syncButtonsToTama();
  return 0;
}

hal_t g_tama_hal = {
    halMalloc,
    halFree,
    halHalt,
    halIsLogEnabled,
    halLog,
    halSleepUntil,
    halGetTimestamp,
    halUpdateScreen,
    halSetLcdMatrix,
    halSetLcdIcon,
    halSetFrequency,
    halPlayFrequency,
    halHandler,
};

void renderTamaFrame() {
  displayRenderTama(&g_pixels[0][0], g_icons, g_sound_enabled);
}
}  // namespace

void tamaAppInit() {
  buttonsSetFeedbackEnabled(false);
  tamaStorageInit();

#if M5STICKS3_HAS_LOCAL_ROM
  static_assert((sizeof(kTamaRom) / sizeof(kTamaRom[0])) >= 8192,
                "data/rom.h must define kTamaRom with at least 8192 12-bit words");

  tamalib_register_hal(&g_tama_hal);
  tamalib_set_framerate(30);

  if (tamalib_init(kTamaRom, nullptr, 1000000) == 0) {
    g_running = true;
    g_init_failed = false;
    Serial.println("tamalib: initialized with local ROM");
    if (!tamaStorageRestore()) {
      tamaStorageMarkDirty();
    }
    renderTamaFrame();
  } else {
    g_running = false;
    g_init_failed = true;
    buttonsSetFeedbackEnabled(true);
    Serial.println("tamalib: init failed");
  }
#else
  g_running = false;
  g_init_failed = false;
  buttonsSetFeedbackEnabled(true);
  Serial.println("tamalib: data/rom.h not found; showing ROM setup screen");
#endif
}

void tamaAppUpdate(const ImuSample& imu) {
  if (!g_running) {
    const uint32_t now_ms = millis();
    if (g_last_frame_ms == 0 || now_ms - g_last_frame_ms >= 250 || buttonsLastEventAgeMs() < 300) {
      g_last_frame_ms = now_ms;
      displayRenderMissingRom(imu, g_init_failed);
    }
    delay(10);
    return;
  }

  syncButtonsToTama();

  const uint32_t step_start_us = micros();
  uint16_t steps = 0;
  do {
    tamalib_step();
    ++steps;
  } while ((micros() - step_start_us) < kStepBudgetUs && steps < kMaxStepsPerLoop);

  const uint32_t now_ms = millis();
  if (now_ms - g_last_frame_ms >= kFrameIntervalMs) {
    g_last_frame_ms = now_ms;
    renderTamaFrame();
  }

  tamaStorageUpdate(g_running, buttonsIsAnyPressed(), buttonsLastActivityAgeMs());
}

bool tamaAppHasRom() {
  return M5STICKS3_HAS_LOCAL_ROM != 0;
}

bool tamaAppIsRunning() {
  return g_running;
}

bool tamaAppIsScreenDark() {
  if (!g_running) {
    return false;
  }

  const uint16_t active_pixels = tamaFrameCountLit(&g_pixels[0][0]);
  return active_pixels <= 2 || active_pixels >= kTamaDarkRoomMinLitPixels;
}

uint32_t tamaAppFastForward(uint32_t elapsed_ms) {
  if (!g_running || elapsed_ms == 0) {
    return 0;
  }

  state_t* state = tamalib_get_state();
  if (state == nullptr || state->tick_counter == nullptr) {
    Serial.println("tamalib: catchup skipped, no state");
    return 0;
  }

  const uint32_t requested_ms = elapsed_ms;
  if (elapsed_ms > kMaxCatchupElapsedMs) {
    elapsed_ms = kMaxCatchupElapsedMs;
  }

  const uint32_t start_ticks = *state->tick_counter;
  const uint32_t target_ticks =
      start_ticks + static_cast<uint32_t>((static_cast<uint64_t>(elapsed_ms) *
                                           kTamaTickFrequency) /
                                          1000ULL);
  const uint32_t wall_start_ms = millis();
  uint32_t steps = 0;

  g_fast_forwarding = true;
  tamalib_set_speed(0);
  while (static_cast<int32_t>(*state->tick_counter - target_ticks) < 0 &&
         millis() - wall_start_ms < kMaxCatchupWallMs) {
    tamalib_step();
    ++steps;
    if ((steps & 0x3FF) == 0) {
      yield();
    }
  }
  tamalib_set_speed(1);
  g_fast_forwarding = false;
  M5.Speaker.stop();

  const uint32_t advanced_ticks = *state->tick_counter - start_ticks;
  const uint32_t advanced_ms =
      static_cast<uint32_t>((static_cast<uint64_t>(advanced_ticks) * 1000ULL) /
                            kTamaTickFrequency);
  Serial.printf("tamalib: catchup requested=%lums capped=%lums advanced=%lums steps=%lu wall=%lums\n",
                static_cast<unsigned long>(requested_ms),
                static_cast<unsigned long>(elapsed_ms),
                static_cast<unsigned long>(advanced_ms),
                static_cast<unsigned long>(steps),
                static_cast<unsigned long>(millis() - wall_start_ms));
  tamaStorageMarkDirty();
  renderTamaFrame();
  return advanced_ms;
}

void tamaAppPrintDebugFrame() {
  Serial.println("frame: begin");
  for (uint8_t row = 0; row < kLcdHeight; ++row) {
    for (uint8_t col = 0; col < kLcdWidth; ++col) {
      Serial.print(g_pixels[row][col] ? '#' : '.');
    }
    Serial.println();
  }
  Serial.print("icons:");
  for (uint8_t icon = 0; icon < kIconCount; ++icon) {
    Serial.print(g_icons[icon] ? '1' : '0');
  }
  Serial.println();
  Serial.println("frame: end");
}
