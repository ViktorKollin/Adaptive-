#include <Arduino.h>

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("Hello World, from thomas. I'm going my way");
  Serial.println("Hello World, from thomas");
  Serial.println("Detta är en ändring");
  //Kommenterar lite bara
  delay(1000);
}