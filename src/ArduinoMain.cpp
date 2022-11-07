#include <Arduino.h>

#include <Wire.h>
#include <WiFiNINA.h>
#include <ArduinoJson.hpp>

#include <vector>

#include "./include/.env.h"
#include "./include/max.hpp"

/* template options */


// https://datasheets.maximintegrated.com/en/ds/MAX86150.pdf

// creates sensor object

hf::MaxSensor maxSensor = hf::MaxSensor();
WiFiClient cl;

static const std::string apiEndpoints[2] = {
    "api/rx/",
    "api/test/"
};

static const std::string jsonReciever = apiEndpoints[0];
static const std::string testEndpoint = apiEndpoints[1];

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

    int status = WiFi.begin(SSID, PASS);
    delay(1000);

    // loop until serial connection opens - diagnostic
    while (!Serial)
        delay(1);

    if(status) {
        Serial.println(status);
    }
}

void loop()
{
    maxSensor.fifoReadLoop();
}