#ifndef HF_BPM_FIFO
#define HF_BPM_FIFO

#include <Wire.h>

// #include <string>
// #include <map>
#include <array>
#include <utility>
#include <vector>

#include "maxReg.hpp"
#include "bpmWiFi.hpp"

#include "constants.hpp"

namespace hf
{

    class MaxFifo 
    {

        protected:
            byte _numSlots;

            std::vector<uint32_t> _ppgWindow;
            // std::vector<uint32_t> _ppgWindow1;
            //std::vector<uint32_t> _ecgWindow;

            MaxReg _reg;
            BpmWiFi _bpmWiFi;

        public:
        
            MaxFifo(byte numSlots = 1)
            : _numSlots{numSlots}
            { 
                _reg = MaxReg();  
            }

            void clear() 
            {
                // reset 0x04 = FIFO write ptr, 0x05 = FIFO overflow, 0x06 = FIFO read ptr
                _reg.write(byte(0x04), 0);
                _reg.write(byte(0x05), 0);
                _reg.write(byte(0x06), 0);
            }

            void config()
            {
                _reg.config();
                delay(100);
                _bpmWiFi.initWiFi();
                clear();
            }

            int range() {
                byte readPtr = _reg.read(6U);
                byte writePtr = _reg.read(4U);
                return writePtr - readPtr;
            }

            void read(int numSlots) {
                byte longArr[4];
                uint32_t tempLong;

                for(int i = 0; i < numSlots; i++) {
                    longArr[3] = 0;
                    longArr[2] = Wire.read();
                    longArr[1] = Wire.read();
                    longArr[0] = Wire.read();

                    // convert 4 bytes into long
                    memcpy(&tempLong, longArr, sizeof(tempLong));
                    // std::copy(&tempLong, &tempLong + 4, longArr);

                    // zero all but 19 bits
                    tempLong &= 0x7FFFF;
                    _ppgWindow.push_back(tempLong);

                    // Serial.print("PPG: ");
                    Serial.print(tempLong);
                    Serial.println(",");
                }
            }

            void check() {

                if(_ppgWindow.size() > WINDOW_LENGTH) {
                    // _bpmWiFi.txWindow(_ppgWindow);
                    _bpmWiFi.getTest();
                    _ppgWindow.clear();
                }

                int fifoRange = range();
                if (fifoRange != 0)
                {
                    // wrap fifo range
                    if(fifoRange < 0) fifoRange += 32;

                    // avail = num samples * num devices * bits per sample
                    int available = fifoRange * _numSlots * 3;

                    Wire.beginTransmission(I2C_ADDRESS);
                    Wire.write(7U); // queue bytes to fifo data 0x07
                    Wire.endTransmission();

                    while(available > 0)
                    {
                        int readBytes = available;
                        // i2c buffer len is 32 on Uno, 64 on Nano? - possibly not implemented in Wire lib
                        // so tempRem is limited by buffer len and there may be remaining bytes beyond
                        //                                          bytes * slots <- add all these kinds of functions to util
                        if(readBytes > 32) readBytes = ( 32 - ( 32 % (3 * _numSlots)) );
                        available -= readBytes;

                        Wire.requestFrom(I2C_ADDRESS, readBytes);

                        while(readBytes > 0) {
                            read(_numSlots);
                            readBytes -= _numSlots * 3;
                        }
                    }
                }
            }

            // change this to return an array
            std::vector<uint32_t> getWindow() 
            {
                // std::array<uint32_t, 256> ret;
                std::vector<uint32_t> ret = _ppgWindow;
                _ppgWindow.clear();
                return ret;
            }


    };
}

#endif // HF_MAX