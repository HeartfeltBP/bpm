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
#include "maxReg.hpp"

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
            const byte _i2cAddress = byte(0x5E);
            // arduino::TwoWire _i2c;
            byte _longBytes = 4;
            byte _numSlots;
            // std::vector<uint32_t>

            // probably use global wifi ble and reg
            BpmWiFi _bpmWiFi;
            // BpmBleSerial ble;

            // MaxReg _reg = MaxReg(_i2cAddress);
            // MaxFifo _fifo = MaxFifo(_numSlots, _i2cAddress, WINDOW_SIZE);

        public:
        
            BpmSensor(byte numSlots, byte i2cAddress = 0x5E)
            : _numSlots{numSlots}
            {

            }

        void reg_write(byte reg, byte value)
        {  
            // fix
            // is this uninitialized?
            // if(this->localRegi[reg])
            // this->localRegi[reg].setLocalVal(value);

            // track register value with map
            // this->_localRegi.emplace(reg_pair(reg, value));

            Wire.beginTransmission(this->_i2cAddress);
            Wire.write(reg);
            Wire.write(value);
            Wire.endTransmission();
        }

        byte reg_read(byte reg)
        {
            Wire.beginTransmission(this->_i2cAddress);
            Wire.write(reg);
            Wire.endTransmission(false);

            Wire.requestFrom(this->_i2cAddress, 1);

            int available = Wire.available();
            if(available > 0) 
            {
                int ret = Wire.read();
                // this->_localRegi.emplace(reg_pair(reg, temp));
                return ret;
            }

            return -1;
        }

        void config() {

            // sys control - reset, enable, fifo setup
            reg_write(0x0D, 0x01);
            reg_write(0x0D, 0x04);
            reg_write(0x08, 0x7F);

            // sl1
            reg_write(9U, 0x01);
            // sl2
            reg_write(10U, 0x09);
            
            // PPG config (protocentral 0xD1 for Config1, maxim 0xD3) 0xD7 us: adc range, sample rate=200/s, led pulse width
            reg_write(14U, 0xD7);
            // PPG sample averging
            reg_write(15U, 0x00);

            // set led pulse amplitudes
            reg_write(17U, 0xFF);
            reg_write(18U, 0xFF);

            // // set ECG sampling rate = 200Hz
            // writeRegister(ECG_CONFIG1, byte(0x03));
            // // set ECG IA gain: 9.5; PGA gain: 8 (idk what this means)
            // writeRegister(ECG_CONFIG3, byte(0x0D));

            // // 0x14 = 'led range?' = led current (50 mA = 0x00)
            reg_write(0x14, 0x00);

        }

            void init() {

                config();
                fifo_clear();
                // _bpmWiFi.initWiFi(SSID, PASS, URL);
            }

            void sample()
            {
                // unsigned long start = millis();
                fifo_check();
            }

            void fifo_clear() 
            {
                // reset 0x04 = FIFO write ptr, 0x05 = FIFO overflow, 0x06 = FIFO read ptr
                reg_write(byte(0x04), 0);
                reg_write(byte(0x05), 0);
                reg_write(byte(0x06), 0);
            }

            int fifo_range() {
                byte readPtr = reg_read(6U);
                byte writePtr = reg_read(4U);
                return writePtr - readPtr;
            }

            void fifo_read(int numSlots) {
                byte longArr[_longBytes];
                uint32_t tempLong;

                for(int i = 0; i < numSlots; i++) {
                    longArr[3] = 0;
                    longArr[2] = Wire.read();
                    longArr[1] = Wire.read();
                    longArr[0] = Wire.read();

                    // convert 4 bytes into long
                    memcpy(&tempLong, longArr, sizeof(tempLong));

                    // zero all but 19 bits
                    tempLong &= 0x7FFFF;
                    // _ppgWindow.push_back(tempLong);

                    // Serial.print("PPG: ");
                    Serial.print(tempLong);
                    Serial.println(",");
                }
            } 

            void fifo_check() {
                int fifoRange = fifo_range();
                Serial.println("");
                if (fifoRange != 0) {
                    // if we found data get
                    if(fifoRange < 0) fifoRange += 32;

                    // avail = num samples * num devices * bits per sample
                    int available = fifoRange * _numSlots * 3;

                    
                    Serial.print(available); Serial.print(" = "); Serial.print(fifoRange);
                    Serial.print(" * "); Serial.println(_numSlots);

                    Wire.beginTransmission(_i2cAddress);
                    Wire.write(7U); // queue bytes to fifo data 0x07
                    Wire.endTransmission();

                    while(available > 0)
                    {

                        Serial.println("BUNGA1");
                        Serial.println(fifoRange);
                        Serial.println(available);
                        Serial.println("BUNGA4");
                        int readBytes = available;
                        // i2c buffer len is 32 on Uno, 64 on Nano? - possibly not implemented in Wire lib
                        // so tempRem is limited by buffer len and there may be remaining bytes beyond
                        //                                          bytes * slots
                        if(readBytes > 32) readBytes = 32 - ( 32 % (3 * _numSlots) );
                        available -= readBytes;

                        Wire.requestFrom(_i2cAddress, readBytes);

                        while(readBytes > 0) {
                            fifo_read(_numSlots);
                            readBytes -= _numSlots;

                            // if(_ppgWindow.size() >= (std::size_t)_windowLength) {
                            //     // _bpmWiFi.getTest();
                            //     // because we might break and skip some samples?
                            //     if(readBytes > 0) available += readBytes;
                            //     break;
                            // }
                        }
                    }
                }
            }

            byte getI2CAddress() 
            {
                return this->_i2cAddress;
            }

    };
}

#endif // HF_MAX