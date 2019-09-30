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
  rgbLED.setBrightness(0xFF);
  delay(1000);
  rgbLED.setBrightness(0xF0);
  delay(1000);
  rgbLED.setBrightness(0xE0);
  delay(1000);
  rgbLED.setBrightness(0xD0);
  delay(1000);
  rgbLED.setBrightness(0xC0);
  delay(1000);
  rgbLED.setBrightness(0xB0);
  delay(1000);
  rgbLED.setBrightness(0xA0);
  delay(1000);
  rgbLED.setBrightness(0x90);
  delay(1000);
  rgbLED.setBrightness(0x80);
  delay(1000);
  rgbLED.setBrightness(0x70);
  delay(1000);
  rgbLED.setBrightness(0x60);
  delay(1000);
  rgbLED.setBrightness(0x50);
  delay(1000);
  rgbLED.setBrightness(0x40);
  delay(1000);
  rgbLED.setBrightness(0x30);
  delay(1000);
  rgbLED.setBrightness(0x20);
  delay(1000);
  rgbLED.setBrightness(0x10);
  delay(1000);
  Serial.println("Back to full brightness");
  rgbLED.setBrightness(0xFF);
}
