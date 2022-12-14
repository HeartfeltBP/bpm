#ifndef HF_BPM_SENSOR
#define HF_BPM_SENSOR


#include <ArduinoJson.hpp>
#include <Arduino_LSM6DS3.h>

// #include <string>
// #include <map>
#include <array>
#include <utility>
#include <vector>

#include "bpmWiFi.hpp"
// #include "bpmBl.hpp"
#include "maxFifo.hpp"
// #include "maxReg.hpp"

#define WINDOW_SIZE 256
#define FRAME_SIZE 64

#define SR 200 // Sampling rate remove when reg lib fully implemented
#define ST 5   // Sampling period (ms) ^

#include ".env.h" // WiFi credentials

namespace hf
{

    class BpmSensor 
    {

        protected:
            // arduino::TwoWire _i2c;
            byte _numSlots;

            // probably use global wifi ble and reg
            BpmWiFi _bpmWiFi;
            // BpmBleSerial ble;

            MaxFifo _fifo;

        public:
        
            BpmSensor(byte numSlots)
            : _numSlots{numSlots}
            {
                _fifo = MaxFifo(numSlots);
            }

            int init() {
                _fifo.config();

                // _bpmWiFi.initWiFi(SSID, PASS, URL);
            }

            void sample()
            {
                // unsigned long start = millis();
                _fifo.check();
            }

            std::vector<uint32_t> getWindow() {
                return _fifo.getWindow();
            }

    };
}

#endif