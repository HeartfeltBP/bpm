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
int status;

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

    if (WiFi.status() == WL_NO_MODULE)
    {
        std::__throw_runtime_error("No WiFi module detected!");
        return;
    }

    status = WiFi.begin(SSID, PASS);

    // if (status != WL_CONNECTED)
    // {
    //     Serial.println("WiFi not connected, retrying...");
    // }

    // delay(10);
    // status = WiFi.begin(SSID, PASS);
    cl.connect(URL, 80);
}

void loop()
{
    maxSensor.fifoReadLoop();
}



void txWindow(std::vector<uint32_t> ppgWindow) {
    int txArr[256];
                
    std::copy(&ppgWindow.front(), &ppgWindow.at(256), txArr);
    
    ArduinoJson::StaticJsonDocument<256 * 4> ppgJson;
    ArduinoJson::copyArray(txArr, ppgJson.to<ArduinoJson::JsonArray>());
    ArduinoJson::serializeJsonPretty(ppgJson, Serial);
    
    std::string tmpString = "POST " + jsonReciever + " HTTP/1.1";
    Serial.println(tmpString.c_str());
    cl.println(tmpString.c_str());

    std::string tmpUrl = URL;
    tmpString = "Host: " + tmpUrl;

    cl.println(tmpString.c_str());

    cl.println("Content-Type: application/json");
    cl.println("Accept: */*"); // Set MIME types
    cl.println("Cache-Control: no-cache");
    cl.println("Accept-Encoding: gzip, deflate");
    cl.println("Accept-Language: en-us");

    tmpString = "Content-Length: " + ppgWindow.size();
    cl.println(tmpString.c_str());
    
    cl.println("Connection: close");
    cl.println();

    ArduinoJson::serializeJson(ppgJson, cl);
    cl.println();
}