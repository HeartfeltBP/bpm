#include <Arduino.h>

#include <Wire.h>
#include <WiFiNINA.h>
#include <ArduinoJson.hpp>
#include <vector>
#include "./include/bpmSensor.hpp"

// max https://datasheets.maximintegrated.com/en/ds/MAX86150.pdf

hf::BpmSensor bpmSensor = hf::BpmSensor(1);

void setup()
{
    // IMU.begin();
    // Serial.println(IMU.gyroscopeSampleRate());
    // Serial.println(IMU.accelerationSampleRate());
    // set Baud rate
    
    Serial.begin(115200);

    Wire.begin();
    Wire.setClock(400000);
    
    delay(10);

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
    delay(100);
}