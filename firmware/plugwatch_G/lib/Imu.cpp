#include <Particle.h>
#include <math.h>

// https://build.particle.io/libs/SparkFun_MPU-9250/1.0.0/tab/example/MPU9250BasicAHRS.ino
#include "lib/lis2dh12.h"
#include "lib/Imu.h"
#include "lib/FileLog.h"

void Imu::setup() {
  //set the motion threshold interrupt as an input
  pinMode(IMU_INT, INPUT);

  // This code along with the driver was taken from the permamote
  // repo and slightly modified to fit our use case (I2C, latched interrupt)
  accel.config_for_wake_on_motion(200);

  // Clear the interrupt by reading the interrupt status register
  delay(1000);
  accel.read_status();
}

LoopStatus Imu::loop() {
  super::loop();

  // Sample the wake on Interrupt pin
  if(digitalRead(IMU_INT)) {
    result = "1" + String(MINOR_DLIM) + String(accel.get_temp());

    // Clear the interrupt
    accel.read_status();
  } else {
    result = "0" + String(MINOR_DLIM) + String(accel.get_temp());
  }

  //read the 3axis acceleration and compute the angle
  int16_t x = accel.get_x();
  int16_t y = accel.get_y();
  int16_t z = accel.get_z();

  int pitch = (int)(atan2((-x) , sqrt(y*y + z*z)) * 57.3);

  int sign = 0;
  if(z > 0) {
    sign = 1;
  } else {
    sign = -1;
  }

  //A more stable approximation of roll than the mathematically correct formula
  int roll = (int)(atan2(y , sign * sqrt(z*z + 0.001*x*x)) * 57.3);

  result += String(MINOR_DLIM) + String(pitch) + String(MINOR_DLIM) + String(roll);

  return FinishedSuccess;
}

String Imu::getResult() {
    return result;
}
