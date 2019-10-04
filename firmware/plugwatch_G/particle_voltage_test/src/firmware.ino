// Native
#include <Particle.h>

// Our code
#include "lib/ChargeState.h"

PRODUCT_VERSION(112);
PRODUCT_ID(8379);
SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

//***********************************
//* ChargeState Checker
//***********************************
auto chargeStateSubsystem = ChargeState();

void setup() {
  Serial.begin(9600);

  chargeStateSubsystem.setup();
}

void loop() {
  LoopStatus result = chargeStateSubsystem.loop();

  //return result or error
  if(result == FinishedError) {
    //Log the error in the error struct
    Serial.println("Error sampling charge state");
    delay(5000);
  } else if(result == FinishedSuccess) {
    Serial.printlnf("Got result from charge state: %s",chargeStateSubsystem.getResult().c_str());
    delay(5000);
  }
}

