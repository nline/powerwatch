#include <Particle.h>
#include <Serial5/Serial5.h>

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

  Serial5.begin(9600);
}

void loop() {

  if(Serial5.available()) {
    Serial.write(Serial5.read());
  }
}
