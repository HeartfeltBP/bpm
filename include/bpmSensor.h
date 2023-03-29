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
        
        int _slotIter[SLOT_COUNT] {0};
        bool _checks[3];
        // int _slotIter[0] = 0;
        // int _slotIter[1 = 0;
        // int _ecgi = 0;

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

        int handleIter(int slot)
        {
            if(_slotIter[slot]+1 >= WINDOW_LENGTH || _slotIter[slot] == -1) 
            {
                Serial.println("DISABLED!");
                _slotIter[slot] = -1;
                return 1;
            }
            else 
            {
                _slotIter[slot]++;
            }

            return 0;
        }

        template <typename sampleInt>
        bool pushSample(byte slot, sampleInt sample)
        {

            switch (slot)
            {
            case PPG_SLOT0:
                _ppgWindow0[_slotIter[0]] = sample;
                _checks[0] = (handleIter(PPG_SLOT0) > 0) ? 1 : 0;
                break;
            case PPG_SLOT1:
                _ppgWindow1[_slotIter[1]] = sample;
                _checks[1] = (handleIter(PPG_SLOT1) > 0) ? 1 : 0;
                break;
            case ECG_SLOT:
                _ecgWindow[_slotIter[2]] = sample;
                _checks[2] = (handleIter(ECG_SLOT) > 0) ? 1 : 0;
                break;
            default:
                Serial.println("Something went wrong?");
                break;
            }


            Serial.print(_slotIter[0]); Serial.print("/"); 
            Serial.print(_slotIter[1]); Serial.print("/"); 
            Serial.print(_slotIter[2]); Serial.print("$:");
            Serial.print(_checks[0]); Serial.print(_checks[1]); Serial.print(_checks[2]);
            bool check = _checks[0] && _checks[1] && _checks[2];
            Serial.print("="); Serial.println(check);
            if(check) {delay(4000);}

            return check;
        }

        int handleWindows()
        {
            Serial.println("WINDOW HANDLER INVOKED");
            if( (_slotIter[0] == -1) && (_slotIter[1] == -1) && (_slotIter[2] == -1)) {
                _slotIter[0] = 0; _slotIter[1] = 0; _slotIter[2] = 0;
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
        sampleInt sampleRead(sampleInt temp, int slot)
        {
            byte buffer[4];
            bool ecg = (slot > 2) ? true : false;

            buffer[3] = 0;
            buffer[2] = Wire.read();
            buffer[1] = Wire.read();
            buffer[0] = Wire.read();

            memcpy(&temp, buffer, sizeof(temp));

            if (ecg && temp & (1 << 17))
                temp -= (1 << 18);

            return temp = (ecg) ? (ecgInt)temp & 0x3FFFF : (ppgInt)temp & 0x7FFFF;
        }

        int read(int readBytes)
        {
            bool check;
            Wire.beginTransmission(I2C_ADDRESS);
            Wire.write(7U); // queue bytes to fifo data 0x07
            Wire.endTransmission();
            Wire.requestFrom(I2C_ADDRESS, readBytes);

            for(int rB = readBytes; rB > 0; rB -= (SLOT_COUNT * 3))
            {
                #if (SLOT_COUNT >= 1)
                ppgInt temp = (ppgInt)sampleRead(temp, PPG_SLOT0);
                check = _windowHandler->pushSample(PPG_SLOT0, temp);
                #endif
                #if (SLOT_COUNT >= 2)
                temp = (ppgInt)sampleRead(temp, PPG_SLOT1);
                check = _windowHandler->pushSample(PPG_SLOT1, temp);
                #endif
                #if (SLOT_COUNT >= 3)
                ecgInt temps = (ecgInt)sampleRead(temps, ECG_SLOT);
                check = _windowHandler->pushSample(ECG_SLOT, temps);
                #endif  
            }

            return check;
            // slot 1 = ppg, slot 2 = ppg, slot 3 = ecg   
        }

        int sample()
        {
            
            while (_available > 0)
            {
                _readBytes = _available;
                
                // i2c buffer len is 32 on Uno, 64 on Nano? - possibly not implemented in Wire lib
                // so tempRem is limited by buffer len and there may be remaining bytes beyond
                // bytes * slots
                if (_readBytes > 32)
                    _readBytes = (32 - (32 % (3 * SLOT_COUNT)));

                _available -= _readBytes;            

                while(_readBytes > 0) 
                {
                    if (read(_readBytes) > 0) {
                        return _windowHandler->handleWindows();
                    }
                    _readBytes -= SLOT_COUNT * 3;
                }

            }
            return 0;
        }

        int check() 
        {

            int fifoRange = range();

            if (fifoRange != 0)
            {
                // wrap fifo range
                if (fifoRange < 0)
                    fifoRange += 32;

                // available = num samples * num devices * bytes per sample
                _available = fifoRange * SLOT_COUNT * 3;

                if(_available > 0) {
                    if (sample() > 0) return 1;
                }
            }
            return 0;

        }
    };

    class BpmSensor
    {

    protected:
        MaxReg *_reg;
        MaxFifo *_fifo;
        WindowHandler *_windowHandler;

    public:
        BpmSensor(MaxFifo *fifo, WindowHandler *windowHandler)
            :_fifo{fifo}, _windowHandler{windowHandler}
        {}

        void init()
        {
            _fifo->config();
        }

        int sample()
        {
            return _fifo->check();
        }
    };

    class BPM
    {
        protected:
            struct opFlags {
                bool txReady    = 0;
                bool wiFiInit   = 0;
                bool fifoInit   = 0;
                bool i2cInit    = 0;
                bool proximity  = 0;
                bool identity   = 0;
                bool dataFull   = 0;
                bool error      = 0;
            };
    };
}

#endif // HF_BPM_MAX