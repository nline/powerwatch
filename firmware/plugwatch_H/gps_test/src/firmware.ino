// Native
#include <Particle.h>

// Our code
#include "lib/Gps.h"

PRODUCT_VERSION(112);
PRODUCT_ID(8379);
SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

//***********************************
//* GPS
//***********************************
auto gpsSubsystem = Gps();

void pps_func(void) {
  Serial.println("PPS!");
}

void setup() {
  // Set up debugging UART
  Serial.begin(9600);

  delay(10000);

  //setup the PPS line as an input
  pinMode(D4, INPUT);
  attachInterrupt(D4, pps_func, RISING);

  // GPS
  Serial.println("Turning GPS ON");
  pinMode(C0, OUTPUT);
  digitalWrite(C0, HIGH);

  delay(10000);
  Serial.println("Turning GPS OFF");
  digitalWrite(C0, LOW);

  delay(10000);
  Serial.println("Turning GPS ON");
  digitalWrite(C0, HIGH);

  gpsSubsystem.setup();
}

void loop() {

  LoopStatus result = gpsSubsystem.loop();

  //return result or error
  if(result == FinishedError) {
    Serial.println("GPS error");
  } else if(result == FinishedSuccess) {
    Serial.printlnf("GPS Result: %s", gpsSubsystem.getResult().c_str());
    delay(10000);
  }
}
