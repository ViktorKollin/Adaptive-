#include <Arduino.h>

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("Hello World, from thomas");
  Serial.println("Detta är en ändring");
  delay(1000);
}