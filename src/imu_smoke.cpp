#include "imu_smoke.h"

#include <Arduino.h>
#include <M5Unified.h>

namespace {
ImuSample g_sample;
uint32_t g_last_log_ms = 0;
}  // namespace

void imuSmokeInit() {
  g_sample = {};
}

bool imuSmokeUpdate() {
  const auto imu_updated = M5.Imu.update();
  if (!imu_updated) {
    return false;
  }

  const auto data = M5.Imu.getImuData();
  g_sample.valid = true;
  g_sample.ax = data.accel.x;
  g_sample.ay = data.accel.y;
  g_sample.az = data.accel.z;
  g_sample.gx = data.gyro.x;
  g_sample.gy = data.gyro.y;
  g_sample.gz = data.gyro.z;

  if (millis() - g_last_log_ms >= 1000) {
    g_last_log_ms = millis();
    Serial.printf("imu acc=(%.2f,%.2f,%.2f) gyro=(%.1f,%.1f,%.1f)\n",
                  g_sample.ax,
                  g_sample.ay,
                  g_sample.az,
                  g_sample.gx,
                  g_sample.gy,
                  g_sample.gz);
  }

  return true;
}

const ImuSample& imuSmokeSample() {
  return g_sample;
}

