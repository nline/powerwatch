/**
 * Cloud Interface.
 */

#pragma once

#include <Particle.h>

#define ERROR_EVENT "!"

#define CHARGE_STATE_EVENT "c"
#define HEARTBEAT_EVENT "h"
#define IMU_EVENT "i"
#define IMU_ERROR_EVENT "!i"
#define LIGHT_EVENT "l"
#define SD_ERROR_EVENT "!s"
#define SD_READ_EVENT "r"

class Cloud {
public:
  static void setup();
  static void Publish(String tag, String message);
};
