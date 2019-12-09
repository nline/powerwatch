// Native
#include <Particle.h>

// Our code
#include "lib/FileLog.h"
#include "lib/SDCard.h"

PRODUCT_VERSION(112);
PRODUCT_ID(8379);
SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

SDCard SD;
auto DataLog = FileLog(SD, "data_log.txt");

void setup() {
  Serial.begin(9600);
  Serial.println("Initial Setup.");

  // Setup SD card first so that other setups can log
  SD.setup();
}

void loop() {

  Serial.println("Powering on SD");
  SD.PowerOn();
  delay(15000);

  Serial.println("Powering off SD");
  SD.PowerOff();
  delay(15000);

  Serial.println("Powering on SD");
  SD.PowerOn();
  delay(15000);
  
  Serial.println("Logging to SD Card");
  DataLog.append("This is a data log 1");
  DataLog.append("This is a data log 2");
  Serial.println("Reading from SD Card");
  Serial.printlnf("%s",DataLog.getLastLine().c_str());
}
