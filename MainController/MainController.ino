#include <ArduinoBLE.h>
#include "./ppg/PpgController.cpp"

#include <iostream>
#include <queue>

#define LED_PIN 5
#define PPG_PIN 6

BLEService ppgService("180A");
BLEByteCharacteristic switchCharacteristic("2A57", BLERead | BLENotify);
PpgController controllerPPG(PPG_PIN, LED_PIN, 1000);

int i;
std::queue<int> bleQueue;
int sampleResult;

void 
setup()
{
    // set Baud rate
    Serial.begin(9600);
    i = 1;

    // loop until serial connection opens - diagnostic
    while (!Serial);

    if (!BLE.begin())
    {
        Serial.println("* Starting Bluetooth® Low Energy module failed!");
        while (1);
    }

    switchCharacteristic.writeValue(0);

    BLE.setLocalName("ActiveBP");
    BLE.advertise();

    Serial.println("ActiveBP active");
}

void
loop()
{
    BLEDevice central = BLE.central();

    sampleResult = controllerPPG.samplePpg();
    delay(1000);

    Serial.print("LED PIN: ");
    Serial.print(controllerPPG.getLedPin());
    Serial.print(" | PPG PIN: ");
    Serial.print(controllerPPG.getPpgPin());
    Serial.print(" | PPG output: ");
    Serial.println(sampleResult);

    if (central) {
        Serial.println("bluetooth connected");
        Serial.println(central.address());
        digitalWrite(LED_BUILTIN, HIGH);

        while(central.connected()) {
            sampleResult = controllerPPG.samplePpg();
            Serial.print("OUTPUT | ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(sampleResult);
            bleQueue.push(sampleResult);
            delay(500);

            if(bleQueue.size() >= 20) {
                int buffer[20];
                Serial.println("-----------------------------------------");
                for(int b = 0; b < 20; b++) {
                    buffer[b] = bleQueue.front();
                    bleQueue.pop();

                    Serial.print("Buffer Index | ");
                    Serial.print(b);
                    Serial.print(": ");
                    Serial.println(buffer[b]);
                }
                Serial.println("-----------------------------------------");
                Serial.print("Buffer END - Transmitted Size (bytes): ");
                Serial.println(sizeof(buffer));
                switchCharacteristic.writeValue((int)buffer);
                i = 0;
            } 
            i++;
        }
    }
}

