#include <Particle.h>

PRODUCT_VERSION(112);
PRODUCT_ID(8379);
SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

void setup() {
  Serial.begin(9600);

  pinMode(D2, OUTPUT);
  digitalWrite(D2, LOW);

  delay(10000);
  Serial.println("Exiting Setup");
}

void loop() {
  delay(10000);
  Serial.println("Resetting for 4 seconds in...");
  Serial.println("5");
  delay(1000);
  Serial.println("4");
  delay(1000);
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);
  digitalWrite(D2, HIGH); 
}
