#ifndef HF_BPM_MAX
#define HF_BPM_MAX

#include <Wire.h>
#include <Arduino.h>
#include <array>
#include <utility>
#include <vector>
#include <bitset>

#include <ArduinoJson.hpp>
#include <Arduino_LSM6DS3.h>

#include "constants.hpp"


namespace hf
{

    class MaxReg
    {

    protected:
        int _i2cAddress;

    public:
        MaxReg(int i2cAddress = byte(0x5E))
            : _i2cAddress{i2cAddress} {}

        void write(byte reg, byte value)
        {
            // fix
            // is this uninitialized?
            // if(this->localRegi[reg])
            // this->localRegi[reg].setLocalVal(value);

            // track register value with map
            // this->_localRegi.emplace(reg_pair(reg, value));

            Wire.beginTransmission(I2C_ADDRESS);
            Wire.write(reg);
            Wire.write(value);
            Wire.endTransmission();
        }

        byte read(byte reg)
        {
            Wire.beginTransmission(I2C_ADDRESS);
            Wire.write(reg);
            Wire.endTransmission(false);

            Wire.requestFrom(_i2cAddress, 1);

            int available = Wire.available();
            if (available > 0)
            {
                int ret = Wire.read();
                // this->_localRegi.emplace(reg_pair(reg, temp));
                return ret;
            }

            return -1;
        }

        // move to sensor to isolate functionality?
        void config()
        {
            // sys control - reset, enable, fifo config
            // write(0x02, 0x80);
            write(0x0D, 0x01);
            write(0x0D, 0x04);
            // write(0x08, 0x7F);
            write(0x08, 0x7F);

            // // sl1 (led red)
            // write(9U, 0x01);
            // // sl2 (ecg)
            // write(9U, 0x09);
            // both slots (1001 - 0001)
            // I think this is PPG and ECG in one
            // write(9U, 0x91);
            // sl3(led ir) & 4 (empty)
            
            // write(9U, 0x06);
            /* MAXIM VALS */
            write(9U, 0x21);
            // write(10U, 0x09);

            // PPG config (protocentral 0xD1 for Config1, maxim 0xD3) 0xD7 us: adc range, sample rate=200/s, led pulse width
            write(14U, 0xD7);
            // PPG sample averging
            write(15U, 0x02);
            // write(15U, 0x18);

            // set led pulse amplitudes
            write(17U, 0xFF);
            write(18U, 0xFF);



            // // set ECG sampling rate = 200Hz
            // write(0x3C, 0x03);
            // // set ECG IA gain: 9.5; PGA gain: 8 (idk what this means)
            // write(0x3E, 0x0D);

            // // 0x14 = 'led range?' = led current (50 mA = 0x00)
            write(0x14, 0x00);

            // AFE
            // write(0xFF, 0x54);
            // write(0xFF, 0x4D);
            // write(0xCE, 0x0A);
            // write(0xCF, 0x18);
            // write(0xFF, 0x00);
        }
    };

    // TODO: Add interrupt handling
    class MaxFifo
    {

    protected:
        byte _numSlots;

        std::vector<uint32_t> _ppgWindow0;
        std::vector<uint32_t> _ppgWindow1;
        std::vector<uint32_t> _ecgWindow;

        MaxReg _reg;

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
            // delay(100);
            clear();
        }

        int range()
        {
            byte readPtr = _reg.read(6U);
            byte writePtr = _reg.read(4U);
            return writePtr - readPtr;
        }

        uint32_t savePpg()
        {
            byte longArr[4];
            uint32_t tempLong;

            longArr[3] = 0;
            longArr[2] = Wire.read();
            longArr[1] = Wire.read();
            longArr[0] = Wire.read();

            memcpy(&tempLong, longArr, sizeof(tempLong));
            // std::copy(&tempLong, &tempLong + sizeof(tempLong), &longArr);

            return tempLong &= 0x7FFFF;
        }

        int32_t saveEcg()
        {
            
            byte longArr[4];
            int32_t tempLong; // SIGNED

            longArr[3] = 0;
            longArr[2] = Wire.read();
            longArr[1] = Wire.read();
            longArr[0] = Wire.read();

            memcpy(&tempLong, longArr, sizeof(tempLong));
            // std::copy(&tempLong, &tempLong + sizeof(tempLong), &longArr);

            if(tempLong & (1 << 17)) tempLong -= (1 << 18);

            // return tempLong &= 0x3FFFF;
             return tempLong &= 0x3FFFF;
        }

        void read()
        {
            // slot 1 = ppg, slot 2 = ppg, slot 3 = ecg
            if (_numSlots >= 1) {
                uint32_t tempLong = savePpg();
                Serial.print(tempLong);
                Serial.println(",");
                //_ppgWindow0.push_back(tempLong);

                if (_numSlots >= 2) {
                    tempLong = savePpg();
                    // _ppgWindow1.push_back(tempLong);

                    if (_numSlots >= 3) {
                        int32_t tempLongSigned = saveEcg();
                        // Serial.print(tempLongSigned);
                        // Serial.println(",");

                            // if (_numSlots >= 4) {
                            //     // not implemented
                            // }
                    }
                }
            }

        }

        void sample()
        {

            int fifoRange = range();
            if (fifoRange != 0)
            {
                // wrap fifo range
                if (fifoRange < 0)
                    fifoRange += 32;

                // avail = num samples * num devices * bits per sample
                int available = fifoRange * _numSlots * 3;

                Wire.beginTransmission(I2C_ADDRESS);
                Wire.write(7U); // queue bytes to fifo data 0x07
                Wire.endTransmission();

                while (available > 0)
                {
                    int readBytes = available;
                    // i2c buffer len is 32 on Uno, 64 on Nano? - possibly not implemented in Wire lib
                    // so tempRem is limited by buffer len and there may be remaining bytes beyond
                    // bytes * slots <- add all these kinds of functions to util
                    if (readBytes > 32)
                        readBytes = (32 - (32 % (3 * _numSlots)));
                    available -= readBytes;

                    Wire.requestFrom(I2C_ADDRESS, readBytes);

                    while (readBytes > 0)
                    {
                        read();
                        readBytes -= _numSlots * 3;

                        // if (_ppgWindow0.size() > WINDOW_LENGTH)
                        // {
                        //     // data notification state
                        //     _ppgWindow0.clear();
                        // }
                        // if (_ppgWindow1.size() > WINDOW_LENGTH)
                        // {
                        //     // data notification state
                        //     _ppgWindow1.clear();
                        // }
                        // if (_ecgWindow.size() > WINDOW_LENGTH)
                        // {
                        //     // // data notification state
                        //     // for (auto &&i : _ecgWindow)
                        //     // {
                        //     //     Serial.print(i);
                        //     //     Serial.println(", ");
                        //     // }
                        //     _ecgWindow.clear();
                            
                        // }
                    }
                }
            }
        }
    };

    class BpmSensor 
    {

        protected:
            byte _numSlots;
            MaxFifo _fifo;

        public:
        
            BpmSensor(byte numSlots)
            : _numSlots{numSlots}
            {
                _fifo = MaxFifo(numSlots);
            }

            void init() {
                _fifo.config();
            }

            void sample()
            {
                // unsigned long start = millis();
                _fifo.sample();
            }

    };
}

#endif // HF_BPM_MAX