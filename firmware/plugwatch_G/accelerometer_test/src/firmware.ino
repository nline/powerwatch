// Native
#include <Particle.h>

#include "lib/Imu.h"

PRODUCT_VERSION(112);
PRODUCT_ID(8379);
SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

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
