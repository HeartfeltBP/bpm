#ifndef HF_BPM_MAX
#define HF_BPM_MAX

#include <Wire.h>
#include <Arduino.h>
#include <array>
#include <utility>
#include <vector>

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
            // sys control - reset, enable, fifo setup
            write(0x0D, 0x01);
            write(0x0D, 0x04);
            write(0x08, 0x7F);

            // // sl1
            // write(9U, 0x01);
            // // sl2
            // write(9U, 0x09);
            // both slots (1001 - 0001)
            write(9U, 0x91);
            // sl3
            write(10U, 0x02);

            // PPG config (protocentral 0xD1 for Config1, maxim 0xD3) 0xD7 us: adc range, sample rate=200/s, led pulse width
            write(14U, 0xD7);
            // PPG sample averging
            write(15U, 0x00);

            // set led pulse amplitudes
            write(17U, 0xFF);
            write(18U, 0xFF);

            // set ECG sampling rate = 200Hz
            write(0x3C, 0x03);
            // set ECG IA gain: 9.5; PGA gain: 8 (idk what this means)
            write(0x3E, 0x0D);

            // // 0x14 = 'led range?' = led current (50 mA = 0x00)
            write(0x14, 0x00);
        }
    };

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
            delay(100);
            clear();
        }

        int range()
        {
            byte readPtr = _reg.read(6U);
            byte writePtr = _reg.read(4U);
            return writePtr - readPtr;
        }

        uint32_t save(int bitMask)
        {
            
            byte longArr[4];
            uint32_t tempLong;

            longArr[3] = 0;
            longArr[2] = Wire.read();
            longArr[1] = Wire.read();
            longArr[0] = Wire.read();

            memcpy(&tempLong, longArr, sizeof(tempLong));
            // std::copy(&tempLong, &tempLong + sizeof(tempLong), &longArr);

            return tempLong &= bitMask;
        }

        uint32_t save()
        {
            
            byte longArr[4];
            uint32_t tempLong;

            longArr[3] = 0;
            longArr[2] = Wire.read();
            longArr[1] = Wire.read();
            longArr[0] = Wire.read();

            memcpy(&tempLong, longArr, sizeof(tempLong));
            // std::copy(&tempLong, &tempLong + sizeof(tempLong), &longArr);

            return tempLong;
        }

        void read()
        {
            uint32_t tempLong;

            // fallthrough switch statement to read up to 3 slots atm
            switch(_numSlots) {
                // EMPTY FOR NOW
                case 4:
                    break;
                // PPG_SLOT1 = 3
                case PPG_SLOT1:
                    tempLong = save(0x7FFFF);
                    _ppgWindow1.push_back(tempLong);
                // ECG_SLOT = 2
                case ECG_SLOT:
                    tempLong = save();
                    _ecgWindow.push_back(tempLong);
                // PPG_SLOT0 = 1
                case PPG_SLOT0:
                    tempLong = save(0x7FFFF);
                    _ppgWindow0.push_back(tempLong);
                    break;
                default:
                    break;
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

                        if (_ppgWindow0.size() > WINDOW_LENGTH)
                        {
                            // data notification state
                            _ppgWindow0.clear();
                        }
                        if (_ppgWindow1.size() > WINDOW_LENGTH)
                        {
                            // data notification state
                            _ppgWindow1.clear();
                        }
                        if (_ecgWindow.size() > WINDOW_LENGTH)
                        {
                            // data notification state
                            for (auto &&i : _ecgWindow)
                            {
                                delay(100);
                                Serial.print(i);
                                Serial.println(", ");
                            }
                            _ecgWindow.clear();
                            
                        }
                    }
                }
            }
        }
    };
}

#endif // HF_BPM_MAX