#include <Arduino.h>

#include <Wire.h>
#include <WiFiNINA.h>
#include <ArduinoJson.hpp>

#include <vector>
#include "./include/max.hpp"

/* template options */


// https://datasheets.maximintegrated.com/en/ds/MAX86150.pdf

// creates sensor object
hf::MaxSensor maxSensor = hf::MaxSensor();
WiFiClient cl;

void setup()
{
    Wire.begin();
    Wire.setClock(400000);
    // set Baud rate
    Serial.begin(115200);

    maxSensor.maxInit();
    maxSensor.clearFifo();

    // loop until serial connection opens - diagnostic
    while (!Serial)
        delay(1);

    
    Serial.println("Setup Complete");
    delay(100);
}

void loop()
{
    maxSensor.fifoReadLoop();

    // if(!Serial) {
    //     delay(100);
    //     return;
    // }
}