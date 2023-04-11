#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include "constants.h"

#define SERVICE_UUID "ec9cffe9-4e3f-4bc0-9428-2be09da4f4de"
#define CHAR_UUID "e4c44ae2-05f9-4880-b61d-61f2a463721c"

namespace hf {
    class BpmBle {
        protected:
            BLEServer           *_server;
            BLEService          *_service;
            BLECharacteristic   *_char;
            BLEAdvertising      *_advertising;
        
        public:
            BpmBle(const char *charVal = "not set")
            {}

            void start() {
                BLEDevice::init("hf-BPM");
                _server = BLEDevice::createServer();
                _service = _server->createService(SERVICE_UUID);
                _char = _service->createCharacteristic(CHAR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
                _char->setValue("not set");
                LOG_H_LN("[*] BLE Starting...");
                _service->start();
                _advertising = _server->getAdvertising();
                _advertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue ???
                _advertising->setMinPreferred(0x12);
                _advertising->start();
                LOG_H_LN("[*] BLE Started...");
            }

            int setVal(std::string value) {
                _char->setValue(value.c_str());
                return 0;
            }

            const char *getVal()
            {
                const char *data = _char->getValue().c_str();
                if(strlen(data) > 0) {
                    return data;
                }
                return NULL;
            }

            const char *readChar() {
                return _char->getValue().c_str();
            }
    };    
}
