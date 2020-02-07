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
  pinMode(D0, INPUT);
  digitalWrite(D1, LOW);
  pinMode(C3, OUTPUT);

  Serial5.begin(115200);
  Serial.println("Setup complete.");
}


void loop() {

  delay(10000);

  //Turn off STM
  Serial.println("Turning off STM.");
  digitalWrite(C3, LOW);
  delay(3000);

  //Turn on the STM
  Serial.println("Turning on STM.");
  digitalWrite(C3, HIGH);
  delay(2000);

  Serial.println("Writing size bytes");

  //write the size of the firmware to the serial
  uint8_t* size = (uint8_t *)&application_len;
  for(uint8_t i = 0; i < 4; i++) {
    Serial5.write(size[i]);
  }

  Serial.println("Writing firmware.");

  delay(1000);

  //now write the application in a loop
  for(uint32_t j = 0; j < application_len; j++) {
    while(digitalRead(D0) == 1);
    Serial5.write(application[j]);
    delayMicroseconds(100);
  }
  Serial.println("Done writing application. Delaying - application should start");

  delay(60000);

  Serial.println("Power cycling. Application could start again.");
  digitalWrite(D1, HIGH);
  digitalWrite(C3, LOW);
  delay(1000);
  digitalWrite(C3, HIGH);

  delay(60000);

  Serial.println("Flashing again.");
  digitalWrite(D1, LOW);

}
