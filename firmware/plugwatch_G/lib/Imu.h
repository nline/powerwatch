#pragma once

#include <Particle.h>

#include "lib/lis2dh12.h"
#include "lib/Subsystem.h"

#define IMU_INT C4

class Imu: public Subsystem {
  typedef Subsystem super;

  String self_test_str;

  String result;

public:
  lis2dh12 accel;
  void setup();
  LoopStatus loop();
  String getResult();
};
