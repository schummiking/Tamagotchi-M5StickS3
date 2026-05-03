#pragma once

#include <Arduino.h>

constexpr uint8_t kTamaFrameWidth = 32;
constexpr uint8_t kTamaFrameHeight = 16;
constexpr uint16_t kTamaFramePixels = kTamaFrameWidth * kTamaFrameHeight;
constexpr uint16_t kTamaDarkRoomMinLitPixels = (kTamaFramePixels * 3) / 4;

inline uint16_t tamaFrameCountLit(const bool* pixels) {
  uint16_t lit_pixels = 0;
  for (uint16_t i = 0; i < kTamaFramePixels; ++i) {
    if (pixels[i]) {
      ++lit_pixels;
    }
  }
  return lit_pixels;
}

inline bool tamaFrameLooksLikeDarkRoom(const bool* pixels) {
  return tamaFrameCountLit(pixels) >= kTamaDarkRoomMinLitPixels;
}
