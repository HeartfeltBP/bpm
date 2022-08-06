#include <ArduinoJson.hpp>
#include <ArduinoBLE.h>

class CommsBle {
    private:  
        BLEDevice central; 
    public:
        CommsBle(BLELocalDevice BLE, String name)
        {
            if (!BLE.begin()) {
                Serial.println("* Starting Bluetooth® Low Energy module failed!");
                BLE.end();
            }

            BLE.setLocalName(name.c_str());
            BLE.advertise();

            Serial.println("ActiveBP active");
        }

        void setCentral(BLEDevice central) 
        {
            this->central = central;
        }

        bool checkCentral() {
            return (this->central) ? true : false;
        }


};