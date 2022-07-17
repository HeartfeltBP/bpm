#include <ArduinoBLE.h>
#include "./ppg/PpgController.cpp"

#define LED_PIN A1
#define PPG_PIN A2

BLEService ppgService("180A");
BLEByteCharacteristic switchCharacteristic("2A57", BLERead | BLEWrite);

PpgController controllerPPG(PPG_PIN, LED_PIN, 100);

void 
setup()
{
    // set Baud rate
    Serial.begin(9600);

    while (!Serial);

    if (!BLE.begin())
    {
        Serial.println("* Starting Bluetooth® Low Energy module failed!");
        while (1);
    }

    switchCharacteristic.writeValue(0);

    BLE.setLocalName("ActiveBP: Joe");
    BLE.advertise();

    Serial.println("ActiveBP Active");
}

void
loop()
{
    BLEDevice central = BLE.central();

    if (central) {
        Serial.print("Connected to central: ");
        Serial.println(central.address());
        digitalWrite(LED_BUILTIN, HIGH);

        // while(central.connected()) {
        //     Serial.print("%d", (int)controllerPPG.samplePpg());
        // }
    }
}

