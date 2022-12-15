#include <Arduino.h>
#include <Wire.h>

// #include "./include/bpmWiFi.hpp"
#include "./include/bpmSensor.hpp"
#include "./include/stress.hpp"

// max https://datasheets.maximintegrated.com/en/ds/MAX86150.pdf

// hf::BpmSensor bpmSensor = hf::BpmSensor(1);
// // hf::BpmWiFi bpmWiFi;

// void setup()
// {   
//     Serial.begin(115200);
//     Wire.begin();
//     Wire.setClock(400000);
//     delay(10);

//     // bpmWiFi.initWiFi();
//     bpmSensor.init();

//     // loop until serial connection opens - diagnostic
//     while (!Serial)
//         delay(1);
    
//     Serial.println("Setup Complete");
//     delay(100);
// }

// void loop()
// {
//     bpmSensor.sample();
//     // delay(1000);
// }

hf::StressTest test = hf::StressTest();

void setup() 
{
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);

    while (!Serial) delay(1);
}

void loop()
{
    Serial.println("BASIC TEST STARTING");
    test.basic(); // basic calculatoins - reads and sends data

    Serial.println("INTENSE TEST STARTING");
    test.intense();
}