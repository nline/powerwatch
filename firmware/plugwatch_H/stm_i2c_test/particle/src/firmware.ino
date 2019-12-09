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

  if(!Wire.isEnabled()) {
    Wire.begin();
  }
}

void loop() {
  //assign STM to 1A for now
  Wire.beginTransmission(0x0D);
  Wire.write(0x10);
  Wire.endTransmission(false);
  Wire.requestFrom(0x0D, 1, true);
  uint8_t val = Wire.read();
  Wire.endTransmission(true);
  Serial.printlnf("Expected output 0x20. Received output 0x%02x", val);

  Wire.beginTransmission(0x0D);
  Wire.write(0x20);
  Wire.endTransmission(false);
  Wire.requestFrom(0x0D, 1, true);
  val = Wire.read();
  Wire.endTransmission(true);
  Serial.printlnf("Expected output 0x30. Received output 0x%02x", val);

  Serial.println("Powering off. Should not receive response.");
  digitalWrite(C3, LOW);
  delay(2000);
  Wire.beginTransmission(0x0D);
  int ret = Wire.write(0x20);
  Serial.printlnf("Got write return value %d", ret);
  Wire.endTransmission(false);
  Wire.requestFrom(0x0D, 1, true);
  val = Wire.read();
  Wire.endTransmission(true);
  Serial.printlnf("Expected output 0xFF. Received output 0x%02x", val);
  delay(2000);
  digitalWrite(C3, HIGH);

  delay(5000);
}
