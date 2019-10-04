#include <Particle.h>

PRODUCT_VERSION(112);
PRODUCT_ID(8379);
SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

retained unsigned long reboot_cnt = 0;

void setup() {
  Serial.begin(9600);
  
}

void loop() {
  delay(10000);
  Serial.printlnf("Current reboot count. Should increment ever ~25 minutes: %d", reboot_cnt);
}
