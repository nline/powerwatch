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

int32_t m = 0;
int32_t temp = 0;
int32_t l = 0;
int32_t n = 0;

void loop() {
  temp = 0;
  l = 0;
  n = 0;

  Wire.requestFrom(0x0D, 12, true);
  for(uint8_t i = 0; i < 4; i++) {
    uint8_t b = Wire.read();
    temp |= (b << i*8);
    Serial.printlnf("Byte %d: %02X",i,b);
  }
  if(abs(temp) > m) {
    m = temp;
  }

  for(uint8_t i = 0; i < 4; i++) {
    uint8_t b = Wire.read();
    l |= (b << i*8);
    Serial.printlnf("Byte %d: %02X",i,b);
  }

  for(uint8_t i = 0; i < 4; i++) {
    uint8_t b = Wire.read();
    n |= (b << i*8);
    Serial.printlnf("Byte %d: %02X",i,b);
  }
  Wire.endTransmission(true);


  Serial.printlnf("LV waveform %ld", temp);
  Serial.printlnf("Max LV waveform %ld", m);
  Serial.printlnf("Max L %ld", l);
  Serial.printlnf("Max N %ld", n);
  delay(5000);
}
