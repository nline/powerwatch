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
  float vbuf[6];
  Wire.requestFrom(0x0D, 24, true);
  uint32_t temp = 0;
  for(uint8_t i = 0; i < 24; i++) {
    temp |= (Wire.read() << (i % 4));

    if(i % 4 == 3) {
      vbuf[i/4] = (float)(temp);
      temp = 0;
    }
  }
  Wire.endTransmission(true);
  Serial.printlnf("LV waveform %0.2f", vbuf[0]);
  Serial.printlnf("HV waveform %0.2f", vbuf[1]);
  Serial.printlnf("LV second %0.2f", vbuf[2]);
  Serial.printlnf("HV second %0.2f", vbuf[3]);
  Serial.printlnf("LV minute %0.2f", vbuf[4]);
  Serial.printlnf("HV minute %0.2f", vbuf[5]);
  delay(20000);
}
