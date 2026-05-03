#include "system_led.h"

#include <Arduino.h>
#include <M5Unified.h>

namespace {
constexpr uint8_t kPi4io2I2cAddr = 0x44;
constexpr uint8_t kPi4ioOutputReg = 0x05;
constexpr uint8_t kStickS3SystemLedBit = 1 << 7;
constexpr uint32_t kI2cFreq = 400000;

bool g_sleep_override = false;
bool g_restore_high = true;

bool setLedOutputHigh(bool high) {
  if (high) {
    return M5.In_I2C.bitOn(kPi4io2I2cAddr, kPi4ioOutputReg, kStickS3SystemLedBit,
                           kI2cFreq);
  }
  return M5.In_I2C.bitOff(kPi4io2I2cAddr, kPi4ioOutputReg, kStickS3SystemLedBit,
                          kI2cFreq);
}

bool readLedOutputHigh() {
  return (M5.In_I2C.readRegister8(kPi4io2I2cAddr, kPi4ioOutputReg, kI2cFreq) &
          kStickS3SystemLedBit) != 0;
}
}  // namespace

void systemLedInit() {
  g_sleep_override = false;
  g_restore_high = readLedOutputHigh();
}

void systemLedEnterDisplaySleep() {
  if (g_sleep_override) {
    return;
  }

  g_restore_high = readLedOutputHigh();

  // Stick S3 SYS_LEDG is active-low on PI4IO E1 P7, so HIGH means LED off.
  if (!setLedOutputHigh(true)) {
    Serial.println("system_led: sleep off failed");
    return;
  }
  g_sleep_override = true;
  Serial.println("system_led: sleep off");
}

void systemLedLeaveDisplaySleep() {
  if (!g_sleep_override) {
    return;
  }

  g_sleep_override = false;
  if (!setLedOutputHigh(g_restore_high)) {
    Serial.println("system_led: restore failed");
    return;
  }
  Serial.println("system_led: restored");
}
