// Native
#include <Particle.h>

#include "lib/Imu.h"
auto imuSubsystem = Imu();

void setup() {
  Serial.begin(9600);

  imuSubsystem.setup();
}

void loop() {
  LoopStatus result = imuSubsystem.loop();

  if(result == FinishedError) {
    Serial.println("IMU Error"); 
  } else if (result == FinishedSuccess){
    Serial.printlnf("IMU returned result %s", imuSubsystem.getResult().c_str());
    delay(10000);
  }
}
