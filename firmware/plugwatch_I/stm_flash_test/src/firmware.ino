// Native
#include <Particle.h>
#include <Serial5/Serial5.h>

#include "blink.h"

SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);


void setup() {
  // Set up debugging UART
  Serial.begin(9600);
  Serial.println("Initial Setup.");

  //set the two I2C lines low
  pinMode(D1, OUTPUT);
  digitalWrite(D1, LOW);
  digitalWrite(D0, LOW);

  //Turn on the STM
  pinMode(C3, OUTPUT);
  digitalWrite(C3, HIGH);

  delay(2000);

  Serial5.begin(9600);
  
  Serial.println("Setup complete.");
}


void loop() {
  //write the size of the firmware to the serial
  uint8_t* size = (uint8_t *)&application_len;
  for(uint8_t i = 0; i < 4; i++) {
    Serial5.write(size[i]);
  }

  Serial.println("Wrote application size. Now writing firmware.");

  delay(1000);

  //now write the application in a loop
  for(uint32_t j = 0; j < application_len; j++) {
    Serial5.write(application[j]);
    delay(10);
  }
}
