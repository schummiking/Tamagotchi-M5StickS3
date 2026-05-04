#include "system_led.h"

#include <Arduino.h>
#include <M5Unified.h>

namespace {
constexpr uint8_t kM5pm1I2cAddr = 0x6E;
constexpr uint8_t kM5pm1PowerCfgReg = 0x06;
constexpr uint8_t kM5pm1StatusLedBit = 1 << 4;
constexpr uint32_t kM5pm1I2cFreq = 100000;
constexpr uint8_t kPi4io2I2cAddr = 0x44;
constexpr uint8_t kPi4ioOutputReg = 0x05;
constexpr uint8_t kStickS3SystemLedBit = 1 << 7;
constexpr uint32_t kI2cFreq = 400000;

enum class LedBackend {
  None,
  M5pm1StatusLed,
  Pi4ioSystemLed,
};

bool g_sleep_override = false;
bool g_restore_visible = true;
LedBackend g_backend = LedBackend::None;

bool readRegister8Checked(uint8_t address, uint8_t reg, uint8_t& value,
                          uint32_t freq) {
  for (uint8_t attempt = 0; attempt < 2; ++attempt) {
    if (M5.In_I2C.readRegister(address, reg, &value, 1, freq)) {
      return true;
    }
    delay(2);
  }
  return false;
}

const char* backendName() {
  switch (g_backend) {
    case LedBackend::M5pm1StatusLed:
      return "pm1";
    case LedBackend::Pi4ioSystemLed:
      return "pi4io";
    case LedBackend::None:
    default:
      return "none";
  }
}

bool readPm1StatusLedVisible() {
  uint8_t power_cfg = 0;
  if (!readRegister8Checked(kM5pm1I2cAddr, kM5pm1PowerCfgReg, power_cfg,
                            kM5pm1I2cFreq)) {
    return false;
  }
  return (power_cfg & kM5pm1StatusLedBit) != 0;
}

bool setPm1StatusLedVisible(bool visible) {
  uint8_t power_cfg = 0;
  if (!readRegister8Checked(kM5pm1I2cAddr, kM5pm1PowerCfgReg, power_cfg,
                            kM5pm1I2cFreq)) {
    return false;
  }
  if (visible) {
    power_cfg |= kM5pm1StatusLedBit;
  } else {
    power_cfg &= ~kM5pm1StatusLedBit;
  }
  for (uint8_t attempt = 0; attempt < 2; ++attempt) {
    if (M5.In_I2C.writeRegister8(kM5pm1I2cAddr, kM5pm1PowerCfgReg, power_cfg,
                                  kM5pm1I2cFreq)) {
      return true;
    }
    delay(2);
  }
  return false;
}

bool setLedOutputHigh(bool high) {
  if (high) {
    return M5.In_I2C.bitOn(kPi4io2I2cAddr, kPi4ioOutputReg, kStickS3SystemLedBit,
                           kI2cFreq);
  }
  return M5.In_I2C.bitOff(kPi4io2I2cAddr, kPi4ioOutputReg, kStickS3SystemLedBit,
                          kI2cFreq);
}

bool readLedOutputHigh() {
  uint8_t output = 0;
  if (!readRegister8Checked(kPi4io2I2cAddr, kPi4ioOutputReg, output, kI2cFreq)) {
    return true;
  }
  return (output & kStickS3SystemLedBit) != 0;
}

bool selectBackend() {
  if (M5.getBoard() == m5::board_t::board_M5StickS3) {
    g_backend = LedBackend::M5pm1StatusLed;
    return true;
  }
  if (M5.In_I2C.scanID(kPi4io2I2cAddr, kI2cFreq)) {
    g_backend = LedBackend::Pi4ioSystemLed;
    return true;
  }
  g_backend = LedBackend::None;
  return false;
}

bool readLedVisible() {
  switch (g_backend) {
    case LedBackend::M5pm1StatusLed:
      return readPm1StatusLedVisible();
    case LedBackend::Pi4ioSystemLed:
      return !readLedOutputHigh();
    case LedBackend::None:
    default:
      return false;
  }
}

bool setLedVisible(bool visible) {
  switch (g_backend) {
    case LedBackend::M5pm1StatusLed:
      return setPm1StatusLedVisible(visible);
    case LedBackend::Pi4ioSystemLed:
      return setLedOutputHigh(!visible);
    case LedBackend::None:
    default:
      return false;
  }
}
}  // namespace

void systemLedInit() {
  g_sleep_override = false;
  if (!selectBackend()) {
    Serial.println("system_led: no controllable led");
    return;
  }
  setLedVisible(true);
  g_restore_visible = true;
  Serial.printf("system_led: backend %s\n", backendName());
}

void systemLedEnterDisplaySleep() {
  // Display standby is intentionally visible: the LED reminds us this is not
  // a real low-power state.
  Serial.printf("system_led: display standby keep on (%s)\n", backendName());
}

void systemLedLeaveDisplaySleep() {
}

void systemLedEnterLowPowerSleep() {
  if (g_sleep_override) {
    return;
  }

  if (g_backend == LedBackend::None && !selectBackend()) {
    Serial.println("system_led: sleep off failed");
    return;
  }

  g_restore_visible = readLedVisible();
  if (!setLedVisible(false)) {
    Serial.println("system_led: sleep off failed");
    return;
  }
  g_sleep_override = true;
  Serial.printf("system_led: low power off (%s)\n", backendName());
}

void systemLedLeaveLowPowerSleep() {
  if (!g_sleep_override) {
    return;
  }

  g_sleep_override = false;
  if (!setLedVisible(g_restore_visible)) {
    Serial.println("system_led: restore failed");
    return;
  }
  Serial.printf("system_led: restored (%s)\n", backendName());
}
