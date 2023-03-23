#ifndef HF_BPM_MAX
#define HF_BPM_MAX

#include <Wire.h>
#include <Arduino.h>
#include <array>
#include "constants.h"

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

        void config()
        {
            // sys control - reset, enable, fifo config
            // write(0x02, 0x80);
            write(0x0D, 0x01);
            write(0x0D, 0x04);
            // write(0x08, 0x7F);
            write(0x08, 0x1F);

            /* slot 1 = ppg(ir), slot 2 = ppg(red), slot 3 = ecg */
            (SLOT_COUNT >= 1) ? write(9U, 0x21) : write(9U, 0x02); // slot 1 and 2

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

            #if (SLOT_COUNT > 2)
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
            #endif

            // // 0x14 = 'led range?' = led current (50 mA = 0x00)
            write(0x14, 0x00);

            Serial.println("PPG Configuration: Complete");
            Serial.println("FIFO Configuration: Complete");
        }
    };

    class WindowHandler
    {
    protected:
        unsigned int _windowLength;
        int _numSlots;

        ppgInt *_ppgWindow0;
        ppgInt *_ppgWindow1;
        ecgInt *_ecgWindow;

        bool _serial;
        
        // int _slotIterators[SLOT_COUNT] {0};
        int _ppg0i = 0;
        int _ppg1i = 0;
        int _ecgi = 0;

        int _slotStatus[SLOT_COUNT] {-1};

    public:
        // this is pretty fucking gross
        WindowHandler(ppgInt arr0[WINDOW_LENGTH])
        {
            _ppgWindow0 = arr0;
            _slotStatus[0] = 0;
        }

        WindowHandler(ppgInt arr0[WINDOW_LENGTH], ppgInt arr1[WINDOW_LENGTH])
        {
            _ppgWindow0 = arr0;
            _slotStatus[0] = 0;
            
            _ppgWindow1 = arr1;
            _slotStatus[1] = 0;
        }

        WindowHandler(ppgInt arr0[WINDOW_LENGTH], ppgInt arr1[WINDOW_LENGTH], ecgInt arr2[WINDOW_LENGTH])
        {
            _ppgWindow0 = arr0;
            _slotStatus[0] = 0;

            _ppgWindow1 = arr1;
            _slotStatus[1] = 0;

            _ecgWindow = arr2;
            _slotStatus[2] = 0;
        }

        bool isEnabled(int slot)
        {
            // will matter more once integrated into config
            bool ret = (slot != DISABLED) ? true : false;
            return ret;
        }

        int handleIter(int *iter)
        {
            if(*iter+1 >= WINDOW_LENGTH) {
                *iter = -1;
            } else {
                *iter++;
            }
        }
        template <typename sampleInt>
        int pushSample(byte slot, sampleInt sample)
        {
            switch (slot)
            {
            case PPG_SLOT0:
                _ppgWindow0[_ppg0i] = sample;
                handleIter(&_ppg0i);
                break;
            case PPG_SLOT1:
                _ppgWindow1[_ppg1i] = sample;
                handleIter(&_ppg1i);
                break;
            case ECG_SLOT:
                _ecgWindow[_ecgi] = sample;
                handleIter(&_ecgi);
                break;
            default:
                Serial.println("Something went wrong?");
                break;
            }

            if(_ppg0i == -1 && _ppg1i == -1 && _ecgi == -1) {
                _ppg0i = 0; _ppg1i = 0; _ecgi = 0;
                return 1;
            }
            return 0;
        }
    };
    // TODO: Add interrupt handling
    class MaxFifo
    {

    protected:
        byte _numSlots;
        int _available;
        int _readBytes;

        MaxReg *_reg;
        WindowHandler *_windowHandler;

    public:
        MaxFifo(MaxReg *reg, WindowHandler *windowHandler)
            :_reg{reg}, _windowHandler{windowHandler}
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

            return temp = (ecg) ? (ecgInt)temp & 0x3FFFF : (ppgInt)temp & 0x7FFFF;
        }

        int read(int readBytes)
        {
            Wire.requestFrom(I2C_ADDRESS, readBytes);

            for(int rB = readBytes; rB > 0; rB -= (SLOT_COUNT * 3))
            {
                #if (SLOT_COUNT >= 1)
                ppgInt temp = (ppgInt)burstRead(PPG_SLOT0);
                if (_windowHandler->pushSample(PPG_SLOT0, temp)) return 1;
                #endif
                #if (SLOT_COUNT >= 2)
                temp = (ppgInt)burstRead(PPG_SLOT1);
                if (_windowHandler->pushSample(PPG_SLOT1, temp)) return 1;
                #endif
                #if (SLOT_COUNT >= 3)
                ecgInt temps = (ecgInt)burstRead(ECG_SLOT);
                if (_windowHandler->pushSample(ECG_SLOT, temps)) return 1;
                #endif  
            }
            return 0;
            // slot 1 = ppg, slot 2 = ppg, slot 3 = ecg   
        }

        int check() 
        {

            int fifoRange = range();
            if (fifoRange != 0)
            {
                // wrap fifo range
                if (fifoRange < 0)
                    fifoRange += 32;

                // available = num samples * num devices * bits per sample
                _available = fifoRange * SLOT_COUNT * 3;

                if(_available > 0) {
                    if (sample()) return 1;
                }
            }

        }

        int sample()
        {
            // reimplement reg map
            Wire.beginTransmission(I2C_ADDRESS);
            Wire.write(7U); // queue bytes to fifo data 0x07
            Wire.endTransmission();
            
            while (_available > 0)
            {
                _readBytes = _available;
                
                // i2c buffer len is 32 on Uno, 64 on Nano? - possibly not implemented in Wire lib
                // so tempRem is limited by buffer len and there may be remaining bytes beyond
                // bytes * slots
                if (_readBytes > 32)
                    _readBytes = (32 - (32 % (3 * SLOT_COUNT)));
                
                if (read(_readBytes)) return 1;
                _available -= _readBytes;
            }
        }
    };

    class BpmSensor
    {

    protected:
        MaxReg *_reg;
        MaxFifo *_fifo;
        WindowHandler *_windowHandler;

    public:
        BpmSensor(WindowHandler *windowHandler)
            :_windowHandler{windowHandler}
        {
            _reg = new MaxReg(SLOT_COUNT);
            _fifo = new MaxFifo(_reg, windowHandler);
        }

        void init()
        {
            _fifo->config();
        }

        int sample()
        {
            return _fifo->check();
        }
    };
}

#endif // HF_BPM_MAX