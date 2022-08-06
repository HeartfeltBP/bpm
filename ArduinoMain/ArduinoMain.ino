#include <iostream>
#include "./component/component.hpp"
#include "./sensor/sensor.hpp"
#include <ArduinoBLE.h>
#include <RTCZero.h>
#include <ArduinoLowPower.h>
#include <list>

#define LED_PIN 5
#define PPG_PIN 6

RTCZero rtc;

BLEService ppgService("180A");
BLEByteCharacteristic switchCharacteristic("2A57", BLERead | BLEWrite);
sensor::SensorPpg controllerPPG(PPG_PIN, LED_PIN, 1000);

int i;
std::list<int> bleList;
int sampleResult;

void setup()
{
    // set Baud rate
    Serial.begin(115200);
    
    // initialize the internal real-time-clock
    rtc.begin();

    i = 0;

    // loop until serial connection opens - diagnostic
    while (!Serial)
        ;

    if (!BLE.begin())
    {
    }

    switchCharacteristic.writeValue(0);

    BLE.setLocalName("ActiveBP");
    BLE.advertise();

    Serial.println("ActiveBP active");
}

void loop()
{

    sampleResult = controllerPPG.samplePpg();

    if (central)
    {
        Serial.println("bluetooth connected");
        Serial.println(central.address());
        digitalWrite(LED_BUILTIN, HIGH);

        while (central.connected())
        {
            sampleResult = controllerPPG.samplePpg();
            Serial.print("OUTPUT | ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(sampleResult);
            bleList.push_front(sampleResult);
            delay(200);

            if (bleList.size() >= 20)
            {
                int buffer[20];
                Serial.println();
                Serial.println("-----------------------------------------");
                for (int b = 0; b < 20; b++)
                {
                    buffer[b] = bleList.front();
                    bleList.pop_front();

                    Serial.print("Buffer Index | ");
                    Serial.print(b);
                    Serial.print(": ");
                    Serial.println(buffer[b]);
                }
                Serial.println("Buffer Created");
                Serial.println("-----------------------------------------");
                Serial.print("Buffer END - Transmitted Size (bytes): ");
                Serial.println(sizeof(buffer));

                switchCharacteristic.writeValue((int)buffer);
                Serial.println("Buffer Sent");
                Serial.println();
                i = 0;
            }
            i++;
        }
    } else {
        // delay before attempting to connect again
        delay(200);
        /* TODO:    sleep after certain amount of cycles/time - design state 
                    machine to make the search - connect - connected loop
                    clearer */

    }
}
