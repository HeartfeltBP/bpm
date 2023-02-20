#include <Arduino.h>
#include <Wire.h>

// #include "./include/bpmWiFi.hpp"
#include "./include/bpmSensor.hpp"
// #include <Adafruit_SleepyDog.h>

// max https://datasheets.maximintegrated.com/en/ds/MAX86150.pdf

hf::FrameHandler *frameHandler;
hf::BpmSensor *bpmSensor;
hf::BpmWiFi *bpmWiFi;

void setup()
{
    bpmWiFi = new hf::BpmWiFi(SSID, PASS);
    bpmWiFi->initWiFi();
    Serial.println(getFreeRam());

    Serial.println("WiFi: initialized");
    Serial.println(getFreeRam());

    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);

    // frameHandler = new hf::FrameHandler(bpmWiFi, SLOT_COUNT);
    frameHandler = new PROGMEM hf::FrameHandler(bpmWiFi, SLOT_COUNT);
    Serial.println(getFreeRam());

    bpmSensor = new hf::BpmSensor(frameHandler, SLOT_COUNT);
    Serial.println(getFreeRam());
    Serial.println("Waiting on serial or internet connection to begin...");

    // TODO: start transmitting if serial OR wifi client available
    // loop until serial connection opens - diagnostic
    while (!Serial)
        delay(1);

    bpmSensor->init();
    Serial.println("Sensor: initialized");
    Serial.println("Setup Complete");
    delay(100);
}

void loop()
{
    // TODO: see if we can get window collect -> frame transmission under 5ms
    printFreeRam(true);
    bpmSensor->sample();
    
}