#include <Arduino.h>
#include <Wire.h>

// #include "./include/bpmWiFi.hpp"
#include "./include/bpmSensor.hpp"

// max https://datasheets.maximintegrated.com/en/ds/MAX86150.pdf

hf::WindowHandler *windowHandler;
hf::BpmSensor *bpmSensor;
hf::BpmWiFi *bpmWiFi;

void setup()
{
    bpmWiFi = new hf::BpmWiFi(SSID, PASS);
    // bpmWiFi->initWiFi(true);

    windowHandler = new hf::WindowHandler(bpmWiFi, SLOT_COUNT, false);
    bpmSensor = new hf::BpmSensor(bpmWiFi, windowHandler, SLOT_COUNT);

    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);
    // Wire.setClock(100000);
    delay(10);

    bpmSensor->init();

    // loop until serial connection opens - diagnostic
    while (!Serial)
        delay(1);
    
    Serial.println("Setup Complete");
    delay(100);
}

void loop()
{
    bpmSensor->sample();
    
}