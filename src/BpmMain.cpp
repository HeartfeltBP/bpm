#include <Arduino.h>
#include <Wire.h>
#include <iostream>

// #include "./include/bpmWiFi.hpp"
#include "./include/bpmSensor.hpp"

// max https://datasheets.maximintegrated.com/en/ds/MAX86150.pdf

hf::WindowHandler *windowHandler;
hf::BpmSensor *bpmSensor;
hf::BpmWiFi *bpmWiFi;

// hf::BpmWiFi bpmWiFi;

void setup()
{
    bpmWiFi = new hf::BpmWiFi(SSID, PASS);
    bpmWiFi->initWiFi();
    
    windowHandler = new hf::WindowHandler(bpmWiFi, WINDOW_LENGTH);
    bpmSensor = new hf::BpmSensor(bpmWiFi, windowHandler, SLOT_COUNT);

    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);
    delay(10);

    // bpmWiFi.initWiFi();
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
    // delay(1);
    // Serial.print("FREE RAM: ");Serial.println(getFreeRam());
    // if (millis() > curTime + 1000) {
    //     bpmSensor.sample();
    //     curTime += 1000;
    // }
    // delay(1000);
}