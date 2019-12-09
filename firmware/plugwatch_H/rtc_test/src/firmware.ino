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

  delay(15000);

  //Timesync
  uint32_t time = rtc.getTime();
  Serial.printlnf("Current Time is: %d", time);

  Serial.print("Please set the unix time. Press enter to leave it unchanged: ");
  char last = '0';
  char t[20] = {0};
  uint8_t i = 0;
  while(last != '\n') {
    if(Serial.available() > 0) {
      last = Serial.read();
      Serial.printf("%c",last);
      t[i] = last;
      i++;
    }
  }

  Serial.printlnf("%s",t);

  //convert the string to an integer
  uint32_t p_time = atoi(t);
  if(p_time == 0) {
    Serial.println("No time to set. Continuing.");
  } else {
    rtc.setTime(p_time);
    time = p_time;
    Serial.printlnf("Set time to %d. Continuing.", p_time);
  }

  delay(5000);
  Serial.println("Setting wakeup timer for 30 seconds in future. Unplug and run particle off battery power to check WKP Function.");
  Serial.println("If the particle wakes up and 30s the test passes.");
  Serial.println("If the particle wakes up and 60s the test failed and the RTC did not wake up the particle.");
  pinMode(WKP, OUTPUT);
  digitalWrite(WKP, LOW);
  delay(5000);
  rtc.setTimerFuture(30);
  System.sleep(SLEEP_MODE_DEEP, 60);
}

void loop() {
  delay(1000);
  Serial.println("loop");
}

