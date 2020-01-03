// Native
#include <Particle.h>
#include <APNHelperRK.h>

// Our code
#include "lib/Cloud.h"

PRODUCT_VERSION(200);
PRODUCT_ID(7456);
SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

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

int32_t lv_wave;
int32_t l_lv;
int32_t n_lv;
int32_t hv_wave;
int32_t l_hv;
int32_t n_hv;

void setup() {
  Serial.begin(9600);

  apnHelper.setCredentials();

  Serial.begin(9600);

  Serial.println("Enabling STM Power");
  pinMode(C3, OUTPUT);
  digitalWrite(C3, HIGH);

  if(!Wire.isEnabled()) {
    Wire.begin();
  }

  Particle.connect();
}

void loop() {
  Particle.process();
  
  lv_wave = 0;
  l_lv = 0;
  n_lv = 0;
  hv_wave = 0;
  l_hv = 0;
  n_hv = 0;

  Wire.requestFrom(0x0D, 24, true);
  for(uint8_t i = 0; i < 4; i++) {
    uint8_t b = Wire.read();
    lv_wave |= (b << i*8);
    Serial.printlnf("Byte %d: %02X",i,b);
  }

  for(uint8_t i = 0; i < 4; i++) {
    uint8_t b = Wire.read();
    l_lv |= (b << i*8);
    Serial.printlnf("Byte %d: %02X",i,b);
  }

  for(uint8_t i = 0; i < 4; i++) {
    uint8_t b = Wire.read();
    n_lv |= (b << i*8);
    Serial.printlnf("Byte %d: %02X",i,b);
  }

  for(uint8_t i = 0; i < 4; i++) {
    uint8_t b = Wire.read();
    hv_wave |= (b << i*8);
    Serial.printlnf("Byte %d: %02X",i,b);
  }

  for(uint8_t i = 0; i < 4; i++) {
    uint8_t b = Wire.read();
    l_hv |= (b << i*8);
    Serial.printlnf("Byte %d: %02X",i,b);
  }

  for(uint8_t i = 0; i < 4; i++) {
    uint8_t b = Wire.read();
    n_hv |= (b << i*8);
    Serial.printlnf("Byte %d: %02X",i,b);
  }

  Wire.endTransmission(true);


  Serial.printlnf("LV waveform %ld", lv_wave);
  Serial.printlnf("Max L %ld", l_lv);
  Serial.printlnf("Max N %ld", n_lv);
  Serial.printlnf("HV waveform %ld", hv_wave);
  Serial.printlnf("Max L %ld", l_hv);
  Serial.printlnf("Max N %ld", n_hv);
  Cloud::Publish("voltage",String(lv_wave) + String("|") + String(l_lv) + String("|") + String(n_lv) + String("|") + String(hv_wave) + String("|") + String(l_hv) + String("|") + String(n_hv));
  delay(20000);


}


