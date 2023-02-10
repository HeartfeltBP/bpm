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

#include "bpmWiFi.hpp"
#include "utils.hpp"

namespace hf
{

    class MaxReg
    {

    protected:
        int _i2cAddress;
        int _numSlots;

    public:
        MaxReg(byte numSlots = 1, byte i2cAddress = 0x5E)
            : _i2cAddress{i2cAddress}, _numSlots{numSlots} {}

        void write(byte reg, byte value)
        {
            // TODO: reimplement local register tracking
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

            /* slot 1 = ppg(ir), slot 2 = ppg(red), slot 3 = ecg */
            write(9U, 0x21); // slot 1 and 2
            write(10U, 0x09);

            // PPG config (protocentral 0xD1 for Config1, maxim 0xD3) 0xD7 us: adc range, sample rate=200/s, led pulse width
            write(14U, 0xD7);
            // PPG sample averging
            write(15U, 0x00);

            // prox interrupt threshold
            write(16U, 0x18);

            // set led pulse amplitudes
            write(17U, 0xFF);
            write(18U, 0xFF);

            if (_numSlots > 2)
            {
                // ecg specific settings
                // set ECG sampling rate = 200Hz
                write(0x3C, 0x03);
                // set ECG IA gain: 9.5; PGA gain: 8 (idk what this means)
                write(0x3E, 0x0D);

                // AFE config
                write(0xFF, 0x54);
                write(0xFF, 0x4D);
                write(0xCE, 0x0A);
                write(0xCF, 0x18);
                write(0xFF, 0x00);
            }

            // // 0x14 = 'led range?' = led current (50 mA = 0x00)
            write(0x14, 0x00);
        }
    };

    class WindowHandler
    {
    protected:
        unsigned int _windowLength;
        int _ppg0i = 0;
        int _ppg1i = 0;
        int _ecgi  = 0;

        std::array<unsigned int, WINDOW_LENGTH> _ppgWindow0;
        std::array<unsigned int, WINDOW_LENGTH> _ppgWindow1;
        std::array<int, WINDOW_LENGTH> _ecgWindow;

        BpmWiFi *_wifi;

    public:
        WindowHandler(BpmWiFi *wifi, unsigned int windowLength)
            : _windowLength{windowLength}, _wifi{wifi} {}

        template <typename T>
        int updateWindow(std::array<T, 256> window, T sample, int i)
        {
            window[i] = sample;
            if(i+1 > WINDOW_LENGTH) {
                // callback function?
                i = 0;
            }

            return i += 1;
        }

        // PPG_SLOT0(IR) 1
        // PPG_SLOT1(RED) 2
        // ECG_SLOT  3
        void pushSample(byte slot, int sample)
        {
            switch (slot)
            {
            case PPG_SLOT0:
                _ppg0i = updateWindow(_ppgWindow0, (unsigned)sample, _ppg0i);
                break;
            case PPG_SLOT1:
                _ppg1i = updateWindow(_ppgWindow1, (unsigned)sample, _ppg1i);
                break;
            case ECG_SLOT:
                _ecgi = updateWindow(_ecgWindow,  sample, _ecgi);
                break;
            default:
                break;
            }
            // checkVects();
        }
    };

    // TODO: Add interrupt handling
    class MaxFifo
    {

    protected:
        byte _numSlots;

        MaxReg *_reg;
        WindowHandler *_windowHandler;

    public:
        MaxFifo(MaxReg *reg, WindowHandler *windowHandler, byte numSlots)
            : _numSlots{numSlots}, _reg{reg}, _windowHandler{windowHandler}
        {
        }

        void clear()
        {
            // reset 0x04 = FIFO write ptr, 0x05 = FIFO overflow, 0x06 = FIFO read ptr
            _reg->write(0x04, 0);
            _reg->write(0x05, 0);
            _reg->write(0x06, 0);
        }

        void config()
        {
            _reg->config();
            clear();
        }

        int range()
        {
            byte readPtr = _reg->read(6U);
            byte writePtr = _reg->read(4U);
            return writePtr - readPtr;
        }

        int burstRead(bool ecg = false)
        {
            byte longArr[4];
            int temp;

            longArr[3] = 0;
            longArr[2] = Wire.read();
            longArr[1] = Wire.read();
            longArr[0] = Wire.read();

            // uint32_t temp;
            memcpy(&temp, longArr, sizeof(temp));

            if (ecg && temp & (1 << 17))
                temp -= (1 << 18);

            return temp = (ecg) ? temp & 0x3FFFF : temp & 0x7FFFF;
        }

        void read()
        {
            // slot 1 = ppg, slot 2 = ppg, slot 3 = ecg
            if (_numSlots >= 1)
            {
                int temp = burstRead();
                Serial.print(temp);
                Serial.print(",");
                _windowHandler->pushSample(PPG_SLOT0, temp);

                if (_numSlots >= 2)
                {
                    temp = burstRead();
                    Serial.print(temp);
                    Serial.print(",");
                    // Serial.println(",");
                    _windowHandler->pushSample(PPG_SLOT1, temp);

                    if (_numSlots >= 3)
                    {
                        unsigned int tempU = (unsigned)burstRead(true);
                        Serial.print(tempU);
                        Serial.println(",");
                        _windowHandler->pushSample(ECG_SLOT, tempU);
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

                // available = num samples * num devices * bits per sample
                int available = fifoRange * _numSlots * 3;

                // reimplement reg map
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
                    }
                }
            }
        }
    };

    class BpmSensor
    {

    protected:
        byte _numSlots;
        MaxReg *_reg;
        MaxFifo *_fifo;

        BpmWiFi *_wifi;
        WindowHandler *_windowHandler;

    public:
        BpmSensor(BpmWiFi *wifi, WindowHandler *windowHandler, byte numSlots)
            : _numSlots{numSlots}, _wifi{wifi}, _windowHandler{windowHandler}
        {
            _reg = new MaxReg(SLOT_COUNT);
            _fifo = new MaxFifo(_reg, windowHandler, numSlots);
        }

        void init()
        {
            _fifo->config();
        }

        void sample()
        {
            // unsigned long start = millis();
            _fifo->sample();
        }
    };
}

#endif // HF_BPM_MAX