#pragma once

struct ImuSample {
  bool valid = false;
  float ax = 0.0f;
  float ay = 0.0f;
  float az = 0.0f;
  float gx = 0.0f;
  float gy = 0.0f;
  float gz = 0.0f;
};

void imuSmokeInit();
bool imuSmokeUpdate();
const ImuSample& imuSmokeSample();

