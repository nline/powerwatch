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

void cidDmp() {
  cid_t cid;
  if (!SD.sd.card()->readCID(&cid)) {
    Serial.println("readCID failed");
  }
  Serial.print("\nManufacturer ID: ");
  Serial.printlnf("%X",cid.mid);
  Serial.print("\nOEM ID: ");
  Serial.printlnf("%c%c",cid.oid[0],cid.oid[1]);

  Serial.print("\nProduct: ");
  for (uint8_t i = 0; i < 5; i++) {
    Serial.printf("%X",cid.pnm[i]);
  }
  Serial.println();

  Serial.print("Version: ");
  Serial.printlnf("%d.%d",int(cid.prv_n),int(cid.prv_m));

  Serial.print("Serial number: ");
  Serial.printlnf("%X",cid.psn);

  Serial.print("Manufacturing date:: ");
  Serial.printlnf("%d/%d",int(cid.mdt_month),(2000 + cid.mdt_year_low + 10 * cid.mdt_year_high));
}

void loop() {

  Serial.println("Powering on SD");
  SD.PowerOn();
  delay(5000);


  Serial.println("Powering off SD");
  SD.PowerOff();
  delay(5000);

  Serial.println("Powering on SD");
  SD.PowerOn();
  delay(5000);
  
  Serial.println("Logging to SD Card");
  DataLog.append("This is a data log 1");
  DataLog.append("This is a data log 2");
  cidDmp();

  Serial.println("Powering off SD. Next write should fail.");
  SD.PowerOff();
  delay(1000);

  DataLog.append("This is a data log 3");

  //Writing to the card while it's off 
  SD.PowerOff();
  delay(1000);
  Serial.println("Powering on SD. Read should succeed.");
  SD.PowerOn();
  delay(5000);


  Serial.println("Reading from SD Card");
  Serial.printlnf("%s",DataLog.getLastLine().c_str());
  Serial.printlnf("%s",DataLog.getLastLine().c_str());
}
