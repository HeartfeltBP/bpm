#ifndef HF_BPM_MAX
#define HF_BPM_MAX

#include "constants.hpp"
#include <zephyr.h>
#include "i2c.hpp"

namespace hf
{

    class MaxReg
    {

    protected:
        int _i2cAddress;
        int _numSlots;



    public:
        MaxReg(uint8_t numSlots = 1, uint8_t i2cAddress = 0x5E)
            : _i2cAddress{i2cAddress}, _numSlots{numSlots} {}

        void write(uint8_t reg, uint8_t value)
        {
            i2c_reg_write_byte_dt(&i2c_dt, reg, value);
        }

        uint8_t read(uint8_t reg)
        {
            uint8_t retVal;
            i2c_reg_read_byte_dt(&i2c_dt, reg, &retVal);
            return retVal;
        }

        // uint8_t burstRead(uint8_t reg, int numBytes)
        // {
        //     void *retVal = malloc(numBytes);
        //     i2c_reg_read_byte_dt(&i2c_dt, reg, &retVal);
        //     return retVal;
        // }

        // move to sensor to isolate functionality?
        void config()
        {
            // sys control - reset, enable, fifo config
            // write(0x02, 0x80);
            write(0x0D, 0x01);

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

                printk("ECG Configuration: Complete\n");
            }

            // // 0x14 = 'led range?' = led current (50 mA = 0x00)
            write(0x14, 0x00);
            // printk("PPG Configuration: Complete\n");
            // printk("FIFO Configuration: Complete\n");
        }
    };

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
        void pushSample(uint8_t slot, sampleInt sample)
        {
            switch (slot)
            {
            case PPG_SLOT0:
                _ppgWindow0[_ppg0i] = sample;
                if(_serial) {
                    printk("%lu", sample);
                    (_numSlots > 1) ? printk(",") : printk(",\n");
                }
                _ppg0i += 1;
                if(_ppg0i >= WINDOW_LENGTH) {
                    // _wifi->txWindow(_ppgWindow0, PPG_SLOT0);
                    _ppg0i = 0;
                }
                break;
            case PPG_SLOT1:
                _ppgWindow1[_ppg1i] = sample;
                if(_serial) {
                    printk("%lu", sample);
                    (_numSlots > 2) ? printk(",") : printk(",\n");
                }
                _ppg1i += 1;
                if(_ppg1i >= WINDOW_LENGTH) {
                    _ppg1i = 0;
                }
                break;
            case ECG_SLOT:
                _ecgWindow[_ecgi] = sample;
                if(_serial) {
                    printk("%lu\n", sample);
                }
                _ecgi += 1;
                if(_ecgi >= WINDOW_LENGTH) {
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
        uint8_t _numSlots;

        MaxReg *_reg;
        WindowHandler *_windowHandler;

    public:
        MaxFifo(MaxReg *reg, WindowHandler *windowHandler, uint8_t numSlots)
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
            uint8_t readPtr = _reg->read(6U);
            uint8_t writePtr = _reg->read(4U);
            return writePtr - readPtr;
        }

        template <typename sampleInt>
        sampleInt burstRead(sampleInt slot)
        {
            bool ecg = (slot > 2) ? true : false;

            // 0x07 is fifo data, 3 slots * 3 bytes = 9 bytes
            sampleInt temp;
            void *buffer = k_malloc(sizeof(sampleInt));
            i2c_burst_read_dt(&i2c_dt, 0x07, (int8_t*)buffer, (_numSlots * 3));

            if (ecg && temp & (1 << 17))
                temp -= (1 << 18);

            temp = (sampleInt)buffer;
            // get this out of the burst read function?
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

                // // called target_driver_register in newer verison (available for nano?)
                // i2c_slave_driver_register(i2c_dt);
                
                // // reimplement reg map
                // // Wire.beginTransmission(I2C_ADDRESS);
                // // Wire.write(7U); // queue bytes to fifo data 0x07
                // // Wire.endTransmission();
                

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
        uint8_t _numSlots;
        MaxReg *_reg;
        MaxFifo *_fifo;

        // BpmWiFi *_wifi;
        WindowHandler *_windowHandler;

    public:
        BpmSensor(WindowHandler *windowHandler, uint8_t numSlots)
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