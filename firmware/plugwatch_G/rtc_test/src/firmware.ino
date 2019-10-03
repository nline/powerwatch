// Native
#include <Particle.h>

// Our code
#include "lib/AB1815.h"

PRODUCT_VERSION(112);
PRODUCT_ID(8379);
SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

//***********************************
//* RTC
//***********************************
AB1815 rtc;

void setup() {
  Serial.begin(9600);

  //Timesync
/*  uint32_t time = rtc.getTime();
  Serial.printlnf("Current Time is: %d", time);

  Serial.print("Please set the unix time. Press enter to leave it unchanged:");
  String t;
  char last = '0';
  while(last != '\n') {
    if(Serial.available() > 0) {
      last = Serial.read();
      String l = (const char *)(&last);
      t = t + l;
    }
  }

  //convert the string to an integer
  time = t.toInt();
  if(time == 0) {
    Serial.println("No time to set. Continuing.");
  } else {
    rtc.setTime(time);
    Serial.printlnf("Set time to %d. Continuing.", time);
  }*/
}

void loop() {
  delay(1000);
  Serial.println("loop");
}

