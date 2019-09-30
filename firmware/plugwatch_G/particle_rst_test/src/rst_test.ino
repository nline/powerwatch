#include <Particle.h>

void setup() {
  Serial.begin(9600);

  pinMode(D3, OUTPUT);
  digitalWrite(D3, LOW);
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
  digitalWrite(D3, HIGH); 
}
