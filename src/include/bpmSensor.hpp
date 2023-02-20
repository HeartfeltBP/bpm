#ifndef HF_BPM_MAX
#define HF_BPM_MAX

#include <Wire.h>
#include <Arduino.h>
#include <array>

#include "constants.hpp"

#include "bpmWiFi.hpp"
#include "utils.hpp"

namespace hf
{

    class MaxReg
    {

    protected:
        byte _i2cAddress;
        byte _numSlots;

    public:
        MaxReg(byte numSlots = 1, byte i2cAddress = 0x5E)
            : _i2cAddress{i2cAddress}, _numSlots{numSlots} 
        {
                Serial.println(getFreeRam());   
        }

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
            write(0x08, 0x1F);

            /* slot 1 = ppg(ir), slot 2 = ppg(red), slot 3 = ecg */
            // write(9U, 0x21); // slot 1 and 2
            write(9U, 0x02); // slot 1 and 2
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
                Serial.println("ECG Configuration: Complete");
            }

            // // 0x14 = 'led range?' = led current (50 mA = 0x00)
            write(0x14, 0x00);
            Serial.println("PPG Configuration: Complete");
            Serial.println("FIFO Configuration: Complete");
        }
    };

   
    class FrameHandler
    {
    protected:

        byte _numSlots;

        int _ppg0i = 0; // always enabled
        int _ppg1i = 0;
        int _ecgi  = 0;

        // std::array<ppgInt, FRAME_LENGTH> PROGMEM *_ppgWindow0;
        // std::array<ppgInt, FRAME_LENGTH> PROGMEM *_ppgWindow1;
        // std::array<ecgInt, FRAME_LENGTH> PROGMEM *_ecgWindow;
        ppgInt PROGMEM *_ppgWindow0;
        ppgInt PROGMEM *_ppgWindow1;
        ecgInt PROGMEM *_ecgWindow;

        BpmWiFi *_wifi;

    public:
        FrameHandler(BpmWiFi *wifi, byte numSlots)
            : _numSlots{numSlots}, _wifi{wifi} 
        {
            
            //  if (_numSlots >= 1)
            // {
            //     _ppgWindow0 = new PROGMEM std::array<ppgInt, FRAME_LENGTH>;

            //     if (_numSlots >= 2)
            //     {
            //         _ppgWindow1 = new PROGMEM std::array<ppgInt, FRAME_LENGTH>;

            //         if (_numSlots >= 3)
            //         {
            //             _ecgWindow = new PROGMEM std::array<ecgInt, FRAME_LENGTH>;
            //         }
            //     }
            // }

             if (_numSlots >= 1)
            {
                _ppgWindow0 = new PROGMEM ppgInt[FRAME_LENGTH];

                if (_numSlots >= 2)
                {
                    _ppgWindow1 = new PROGMEM ppgInt[FRAME_LENGTH];

                    if (_numSlots >= 3)
                    {
                        _ecgWindow = new PROGMEM ecgInt[FRAME_LENGTH];
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

        void handleWindow(int slot, bool verbose)
        {
            
        }
    

        template <typename sampleInt>
        void pushSample(byte slot, sampleInt sample)
        {
            // sample = ~sample;
            switch (slot)
            {
            case PPG_SLOT0:
                // _ppgWindow0->data()[_ppg0i] = sample;
                _ppgWindow0[_ppg0i] = sample;
                _ppg0i += 1;
                if(_ppg0i >= FRAME_LENGTH) {
                    if(_wifi) _wifi->txWindow(_ppgWindow0, PPG_SLOT0);
                    // _wifi->getTest();
                    // _ppgWindow0->fill(0);
                    _ppg0i = 0;
                }
                break;
            case PPG_SLOT1:
                // _ppgWindow1->data()[_ppg1i] = sample;
                _ppgWindow1[_ppg0i] = sample;
                _ppg1i += 1;
                if(_ppg1i >= FRAME_LENGTH) {
                    if(_wifi) _wifi->txWindow(_ppgWindow1, PPG_SLOT1);
                    // _wifi->getTest();
                    // _ppgWindow1->fill(0);
                    _ppg1i = 0;
                }
                break;
            case ECG_SLOT:
                // _ecgWindow->data()[_ecgi] = sample;
                _ecgWindow[_ecgi] = sample;
                _ecgi += 1;
                if(_ecgi >= FRAME_LENGTH) {
                    if(_wifi) _wifi->txWindow(_ecgWindow, ECG_SLOT);
                    // _wifi->getTest();
                    // _ecgWindow->fill(0);
                    _ecgi = 0;
                }
                break;
            default:
                Serial.println("What the fuck?");
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
        FrameHandler *_windowHandler;

    public:
        MaxFifo(MaxReg *reg, FrameHandler *frameHandler, byte numSlots)
            : _numSlots{numSlots}, _reg{reg}, _windowHandler{frameHandler}
        {
            Serial.println(getFreeRam());
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

        template<typename sampleInt>
        sampleInt burstRead(sampleInt slot)
        {
            byte longArr[4];

            bool ecg = (slot > 2) ? true : false;

            longArr[3] = 0;
            longArr[2] = Wire.read();
            longArr[1] = Wire.read();
            longArr[0] = Wire.read();

            sampleInt temp;
            memcpy(&temp, longArr, sizeof(temp));

            if (ecg && temp & (1 << 17))
                temp -= (1 << 18);

            printFreeRam(true);

            return temp = (ecg) ? (int)temp & 0x3FFFF : (unsigned int)temp & 0x7FFFF;
        }

        void read()
        {
            printFreeRam(true);
            // slot 1 = ppg, slot 2 = ppg, slot 3 = ecg
            if (_numSlots >= 1)
            {
                unsigned long int temp = (unsigned long int)burstRead(PPG_SLOT0);
                _windowHandler->pushSample(PPG_SLOT0, temp);

                if (_numSlots >= 2)
                {
                    temp = (unsigned long int)burstRead(PPG_SLOT1);
                    _windowHandler->pushSample(PPG_SLOT1, temp);

                    if (_numSlots >= 3)
                    {
                        int temps = (long int)burstRead(ECG_SLOT);
                        _windowHandler->pushSample(ECG_SLOT, temps);
                    }
                }
            }
        }

        void sample()
        {
            printFreeRam(true);
            byte fifoRange = range();
            if (fifoRange != 0)
            {
                // wrap fifo range
                if (fifoRange < 0)
                    fifoRange += 32;

                // available = num samples * num devices * bits per sample
                byte available = fifoRange * _numSlots * 3;
                printFreeRam(true);
                // reimplement reg map
                Wire.beginTransmission(I2C_ADDRESS);
                Wire.write(7U); // queue bytes to fifo data 0x07
                Wire.endTransmission();

                while (available > 0)
                {
                    byte readBytes = available;
                    
                    // i2c buffer len is 32 on Uno, 64 on Nano? - possibly not implemented in Wire lib
                    // so tempRem is limited by buffer len and there may be remaining bytes beyond
                    // bytes * slots
                    if (readBytes > 32)
                        readBytes = (32 - (32 % (3 * _numSlots)));
                    available -= readBytes;

                    Wire.requestFrom(I2C_ADDRESS, readBytes);

                    while (readBytes > 0)
                    {
                        printFreeRam(true);
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

        FrameHandler *_windowHandler;

    public:
        BpmSensor(FrameHandler *frameHandler, byte numSlots)
            : _numSlots{numSlots}, _windowHandler{frameHandler}
        {
            _reg = new MaxReg(SLOT_COUNT);
            _fifo = new MaxFifo(_reg, frameHandler, numSlots);
        }

        void init()
        {
            _fifo->config();
        }

        void sample()
        {
            _fifo->sample();
        }
    };
}

#endif // HF_BPM_MAX