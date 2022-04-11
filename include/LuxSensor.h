#ifndef LUXSENSOR_H_
#define LUXSENSOR_H_

#include "Arduino.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

uint16_t readLuxVisible(void);
void initLuxSensor(void);
void configureSensor(void);

// Adafruit_TSL2591 tsl;

#endif