// Native
#include <Particle.h>

// Our code
#include "lib/Gps.h"
//***********************************
//* GPS
//***********************************
auto gpsSubsystem = Gps();

void setup() {
  // Set up debugging UART
  Serial.begin(9600);

  gpsSubsystem.setup();

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
