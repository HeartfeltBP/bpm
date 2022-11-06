#include <Arduino.h>
#include <Wire.h>
#include <vector>

#include "./include/max.hpp"

/* template options */


// https://datasheets.maximintegrated.com/en/ds/MAX86150.pdf

// creates sensor object
hf::MaxSensor maxSensor = hf::MaxSensor();
int i;

void setup()
{
    i = 0;
    Wire.begin();
    Wire.setClock(400000);
    // set Baud rate
    Serial.begin(115200);

    maxSensor.initRegi();
    maxSensor.clearFifo();

    // loop until serial connection opens - diagnostic
    while (!Serial)
        delay(1);
}

void loop()
{
    maxSensor.fifoReadLoop();
}