#include <Particle.h>

PRODUCT_VERSION(112);
PRODUCT_ID(8379);
SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

void setup() {
  Serial.begin(9600);

  Serial.println("Enabling STM Power");
  pinMode(C3, OUTPUT);
  digitalWrite(C3, HIGH);
}

void loop() {
  delay(1000);
}
