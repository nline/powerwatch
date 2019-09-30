// Native
#include <Particle.h>

// Our code
#include "lib/led.h"

PRODUCT_VERSION(112);
PRODUCT_ID(8379);
SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

led rgbLED;

void setup() {
  // Set up debugging UART
  Serial.begin(9600);
 
  delay(10000);
  Serial.println("Starting LED test");
  Serial.println("All LEDs should be OFF");

  rgbLED.setRed(LOW);
  rgbLED.setGreen(LOW);
  rgbLED.setBlue(LOW);
}

void loop() {
  
  Serial.println("Turning on Red");
  rgbLED.setRed(HIGH);
  delay(2000);
  Serial.println("Turning off Red");
  rgbLED.setRed(LOW);
  delay(2000);

  Serial.println("Turning on Green");
  rgbLED.setGreen(HIGH);
  delay(2000);
  Serial.println("Turning off Green");
  rgbLED.setGreen(LOW);
  delay(2000);

  Serial.println("Turning on Blue");
  rgbLED.setBlue(HIGH);
  delay(2000);
  Serial.println("Turning off Blue");
  rgbLED.setBlue(LOW);
  delay(2000);

  Serial.println("Turning on All");
  rgbLED.setRed(HIGH);
  rgbLED.setGreen(HIGH);
  rgbLED.setBlue(HIGH);
  delay(2000);
  Serial.println("Turning off All");
  rgbLED.setRed(LOW);
  rgbLED.setGreen(LOW);
  rgbLED.setBlue(LOW);
  delay(2000);
  Serial.println("Turning on All");
  rgbLED.setRed(HIGH);
  rgbLED.setGreen(HIGH);
  rgbLED.setBlue(HIGH);
  delay(2000);

  Serial.println("Slowly decreasing brightness");
  for(uint8_t i = 0xF; i > 0x0; i--) {
    uint8_t b = (i << 4 | i);
    rgbLED.setBrightness(b);
    delay(500);
    b = (i << 4 | (i-1));
    rgbLED.setBrightness(b);
    delay(500);
  }

  Serial.println("Back to full brightness");
  rgbLED.setBrightness(0xFF);

  Serial.println("Turning off All");
  rgbLED.setRed(LOW);
  rgbLED.setGreen(LOW);
  rgbLED.setBlue(LOW);
  delay(2000);
}
