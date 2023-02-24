#ifndef HF_BPM_MAX
#define HF_BPM_MAX

#include <drivers/i2c.h>
#include "constants.hpp"
#include <zephyr.h>
// #include "bpmWiFi.hpp"
// #include "utils.hpp"

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

            // Wire.beginTransmission(I2C_ADDRESS);
            // Wire.write(reg);
            // Wire.write(value);
            // Wire.endTransmission();
        }

        byte read(byte reg)
        {
            // Wire.beginTransmission(I2C_ADDRESS);
            // Wire.write(reg);
            // Wire.endTransmission(false);

            // Wire.requestFrom(_i2cAddress, 1);

            // int available = Wire.available();
            // if (available > 0)
            // {
            //     int ret = Wire.read();
            //     // this->_localRegi.emplace(reg_pair(reg, temp));
            //     return ret;
            // }

            // return -1;
        }

        // move to sensor to isolate functionality?
        void config()
        {
            // sys control - reset, enable, fifo config
            // write(0x02, 0x80);
            write(0x0D, 0x01);
            write(0x0D, 0x04);
            // write(0x08, 0x7F);
            write(0x08, 0x1F);

            /* slot 1 = ppg(ir), slot 2 = ppg(red), slot 3 = ecg */
            write(9U, 0x21); // slot 1 and 2
            // write(10U, 0x09);

            // // PPG config (protocentral 0xD1 for Config1, maxim 0xD3) 0xD7 us: adc range, sample rate=200/s, led pulse width
            write(14U, 0xD7);
            // write(14U, 0xEB);
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
                write(10U, 0x09);
                // set ECG sampling rate = 200Hz
                write(0x3C, 0x03);
                // // set ECG IA gain: 9.5; PGA gain: 8 (idk what this means)
                write(0x3E, 0x0D);

                // AFE config
                write(0xFF, 0x54);
                write(0xFF, 0x4D);
                write(0xCE, 0x0A);
                write(0xCF, 0x18);
                write(0xFF, 0x00);
                // Serial.println("ECG Configuration: Complete");
            }

            // // 0x14 = 'led range?' = led current (50 mA = 0x00)
            write(0x14, 0x00);
            // Serial.println("PPG Configuration: Complete");
            // Serial.println("FIFO Configuration: Complete");
        }
    };

    // class Chrono
    // {
    //     protected:
    //         // add support for multiple timers?
    //         unsigned long int *_time;
    //         boolean *_setList;
    //         byte _timeSlots;

    //     public:
    //         Chrono(byte timeSlots = SLOT_COUNT)
    //             : _timeSlots{timeSlots}
    //         {
    //             _time = new unsigned long int[timeSlots];
    //             _setList = new boolean[timeSlots];
    //         }

    //     boolean slotUsed(byte slot) {
    //         if(slot > _timeSlots) return false;
    //         return (_time[slot] && _setList[slot]) ? true : false;
    //     }

    //     void mark(byte slot) {
    //         _time[slot] = millis();
    //         _setList[slot] = true;
    //     }

    //     unsigned long int check(byte slot) {
    //         if(slotUsed(slot)) {
    //             unsigned long int curTime = millis();
    //             return curTime - _time[slot];
    //         } else {
    //             return 99999999;
    //         }
    //     }

    //     void reset(byte slot) {
    //         _time[slot] = 0;
    //         _setList[slot] = false;
    //     }

    //     void resetAll() {
    //         for(int i = 0; i < _timeSlots; i++) {
    //             reset(i);
    //         }
    //     }
        
    //     unsigned long int checkReset(byte slot) {
    //         int cur = check(slot);
    //         reset(slot);
    //         return cur;
    //     }

    //     unsigned long int checkResetMark(byte slot) {
    //         int cur = check(slot);
    //         reset(slot);
    //         mark(slot);
    //         return cur;
    //     }

    //     boolean handleTime(byte slot, unsigned long int interval) 
    //     {
    //         Serial.println(check(slot));
    //         return (checkResetMark(slot) < interval) ? true : false;
    //         return false;
    //     }
    // };

    class WindowHandler
    {
    protected:
        unsigned int _windowLength;
        int _numSlots;

        bool _serial;

        ppgInt *_ppgWindow0;
        ppgInt *_ppgWindow1;
        ecgInt *_ecgWindow;

        int _ppg0i = 0;
        int _ppg1i = 0;
        int _ecgi = 0;

        // BpmWiFi *_wifi;
        // Chrono _chrono;

    public:
        WindowHandler(int numSlots, bool serial = false)
            : _numSlots{numSlots}, _serial{serial}
        {
            // slot 1 = ppg, slot 2 = ppg, slot 3 = ecg
            if (numSlots >= 1)
            {
                _ppgWindow0 = new ppgInt[WINDOW_LENGTH];

                if (numSlots >= 2)
                {
                    _ppgWindow1 = new ppgInt[WINDOW_LENGTH];

                    if (numSlots >= 3)
                    {
                        _ecgWindow = new ecgInt[WINDOW_LENGTH];
                    }
                }
            }
            
        }
        

        bool isEnabled(int slot)
        {
            // will matter more once integrated into config
            bool ret = (slot != DISABLED) ? true : false;
            return ret;
        }

        template <typename sampleInt>
        void pushSample(byte slot, sampleInt sample)
        {
            switch (slot)
            {
            case PPG_SLOT0:
                _ppgWindow0[_ppg0i] = sample;
                // if(_serial) {
                //     Serial.print(sample);
                //     (_numSlots > 1) ? Serial.print(",") : Serial.println(",");
                // }
                _ppg0i += 1;
                if(_ppg0i >= WINDOW_LENGTH) {
                    // if(_chrono.handleTime(PPG_SLOT0, 5)) {
                    //     Serial.print("hooray"); Serial.println(1);
                    // }
                    // _wifi->txWindow(_ppgWindow0, PPG_SLOT0);
                    // _ppg0i = 0;
                }
                break;
            case PPG_SLOT1:
                _ppgWindow1[_ppg1i] = sample;
                // if(_serial) {
                //     Serial.print(sample);
                //     (_numSlots > 2) ? Serial.print(",") : Serial.println(",");
                // }
                _ppg1i += 1;
                if(_ppg1i >= WINDOW_LENGTH) {
                    // if(_chrono.handleTime(PPG_SLOT1, 5)) {
                    //     // _wifi->txWindow(_ppgWindow1, PPG_SLOT1);
                    //     Serial.print("hooray"); Serial.println(2);
                    // }
                    // _ppg1i = 0;
                }
                break;
            case ECG_SLOT:
                _ecgWindow[_ecgi] = sample;
                // if(_serial) {
                //     Serial.print(sample);
                //     Serial.println(",");
                // }
                _ecgi += 1;
                if(_ecgi >= WINDOW_LENGTH) {
                    // if(_chrono.handleTime(ECG_SLOT, 5)) {
                    //     // _wifi->txWindow(_ecgWindow, ECG_SLOT);
                    //     Serial.print("hooray"); Serial.println(3);
                    // }

                    // calculateBloodOx();
                    _ecgi = 0; _ppg1i = 0; _ppg0i = 0;
                }
                break;
            default:
                break;
            }
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

        /**
         * Gets number of available FIFO elements
         * @returns range of available values
        */
        int range()
        {
            byte readPtr = _reg->read(6U);
            byte writePtr = _reg->read(4U);
            return writePtr - readPtr;
        }

        template <typename sampleInt>
        sampleInt burstRead(sampleInt slot)
        {
            byte longArr[4];

            bool ecg = (slot > 2) ? true : false;

            // longArr[3] = 0;
            // longArr[2] = Wire.read();
            // longArr[1] = Wire.read();
            // longArr[0] = Wire.read();

            sampleInt temp;
            // memcpy(&temp, longArr, sizeof(temp));


            if (ecg && temp & (1 << 17))
                temp -= (1 << 18);

            return temp = (ecg) ? (int)temp & 0x3FFFF : (unsigned int)temp & 0x7FFFF;
        }

        void read()
        {
            // slot 1 = ppg, slot 2 = ppg, slot 3 = ecg
            if (_numSlots >= 1)
            {
                ppgInt temp = (ppgInt)burstRead(PPG_SLOT0);
                _windowHandler->pushSample(PPG_SLOT0, temp);

                if (_numSlots >= 2)
                {
                    temp = (ppgInt)burstRead(PPG_SLOT1);
                    _windowHandler->pushSample(PPG_SLOT1, temp);

                    if (_numSlots >= 3)
                    {
                        ecgInt temps = (ecgInt)burstRead(ECG_SLOT);
                        _windowHandler->pushSample(ECG_SLOT, temps);
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

                // // reimplement reg map
                // Wire.beginTransmission(I2C_ADDRESS);
                // Wire.write(7U); // queue bytes to fifo data 0x07
                // Wire.endTransmission();

                while (available > 0)
                {
                    int readBytes = available;
                    
                    // i2c buffer len is 32 on Uno, 64 on Nano? - possibly not implemented in Wire lib
                    // so tempRem is limited by buffer len and there may be remaining bytes beyond
                    // bytes * slots
                    if (readBytes > 32)
                        readBytes = (32 - (32 % (3 * _numSlots)));
                    available -= readBytes;

                    // Wire.requestFrom(I2C_ADDRESS, readBytes);

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

        // BpmWiFi *_wifi;
        WindowHandler *_windowHandler;

    public:
        BpmSensor(WindowHandler *windowHandler, byte numSlots)
            : _numSlots{numSlots}, _windowHandler{windowHandler}
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