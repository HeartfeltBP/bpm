#include <Arduino.h>
#include <Wire.h>

// #include "./include/bpmWiFi.hpp"
#include "./include/bpmSensor.hpp"

// max https://datasheets.maximintegrated.com/en/ds/MAX86150.pdf

// bunga bunga
static hf::BpmWiFi bpmWiFi(SSID, PASS);
static hf::MaxReg maxReg(SLOT_COUNT, I2C_ADDRESS);
static hf::WindowHandler windowHandler(&bpmWiFi, SLOT_COUNT, true);
static hf::MaxFifo maxFifo(&reg, &windowHandler);
static hf::BpmSensor bpmSensor(&bpmWiFi, &windowHandler, SLOT_COUNT);

void setup()
{
    // bpmWiFi->initWiFi(true);

    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);
    // Wire.setClock(100000);
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
    
}