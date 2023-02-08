#include <Arduino.h>
#include <Wire.h>
#include <iostream>

// #include "./include/bpmWiFi.hpp"
#include "./include/bpmSensor.hpp"
#include "./include/stress.hpp"

// max https://datasheets.maximintegrated.com/en/ds/MAX86150.pdf

hf::BpmSensor bpmSensor = hf::BpmSensor(3);

// hf::BpmWiFi bpmWiFi;

void setup()
{   
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);
    delay(10);

    // bpmWiFi.initWiFi();
    bpmSensor.init();

    // loop until serial connection opens - diagnostic
    while (!Serial)
        delay(1);
    
    Serial.println("Setup Complete");
    delay(100);
}

void loop()
{
    bpmSensor.sample();
    // if (millis() > curTime + 1000) {
    //     bpmSensor.sample();
    //     curTime += 1000;
    // }
    // delay(1000);
}