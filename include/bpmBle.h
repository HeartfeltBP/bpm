// #include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEServer.h>

// #define SERVICE_UUID "ec9cffe9-4e3f-4bc0-9428-2be09da4f4de"
// #define CHAR_UUID "e4c44ae2-05f9-4880-b61d-61f2a463721c"

// namespace hf {
//     class BpmBle {
//         protected:
//             BLEServer *_server;
//             BLEService *_service;
//             BLECharacteristic *_tokenChar;
//             BLEAdvertising *_advertising;
        
//         public:
//             BpmBle() {
//                 BLEDevice::init("hf-BPM");
//                 _server = BLEDevice::createServer();
//                 _service = _server->createService(SERVICE_UUID);
//                 _tokenChar = _service->createCharacteristic(CHAR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
//                 _tokenChar->setValue("unset");
//             }

//             void start() {
//                 _service->start();
//                 _advertising = BLEDevice::getAdvertising();
//                 _advertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue ???
//                 _advertising->setMinPreferred(0x12);
//                 BLEDevice::startAdvertising();

//                 #if (VERBOSE && DEBUG)
//                 Serial.println("BLE Started...");
//                 #endif
//             }

//             const char *readChar() {
//                 return _tokenChar->getValue().c_str();
//             }
//     };    
// }
