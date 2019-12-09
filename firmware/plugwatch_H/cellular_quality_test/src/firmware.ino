// Native
#include <Particle.h>
#include <APNHelperRK.h>

// Our code
#include "lib/CellStatus.h"
#include "lib/Cloud.h"

PRODUCT_VERSION(200);
PRODUCT_ID(7456);
SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

//***********************************
//* ChargeState Checker
//***********************************

const APNHelperAPN apns[7] = {
  {"8901260", "wireless.twilio.com"},
  {"8923301", "http://mtnplay.com.gh"},
  {"8991101", "airtelgprs.com"},
  {"8958021", "gprsweb.digitel.ve"},
  {"8958021", "internet.digitel.ve"},
  {"8923400", "9mobile"},
  {"8918500", "iot-eu.aer.net"}
};
APNHelper apnHelper(apns, sizeof(apns)/sizeof(apns[0]));

auto cellStatus = CellStatus();


void setup() {
  Serial.begin(9600);

  apnHelper.setCredentials();


  Particle.connect();
}

void loop() {
  Particle.process();

  LoopStatus result = cellStatus.loop();

  //return result or error
  if(result == FinishedError) {
    //Log the error in the error struct
    Serial.println("Error getting cell status");
    delay(5000);
  } else if(result == FinishedSuccess) {
    Serial.printlnf("Got result from cell status: %s",cellStatus.getResult().c_str());
    delay(5000);
  }
}

