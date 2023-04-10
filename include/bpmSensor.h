#ifndef HF_BPM_MAX
#define HF_BPM_MAX

#include <Wire.h>
#include <Arduino.h>

#include "constants.h"
#include "crets.h"

#include "bpmWiFi.h"
#include ".env.h"

namespace hf
{

    class MaxReg
    {

    protected:
        int _i2cAddress;
        int _numSlots;

        #if INT_ENABLE
        struct interruptStatus {
            // Interrupt Status 1 (0x00)
            #if ALMOST_FULL_FLAG_EN
            bool almost_full = 0;
            #endif
            #if NEW_PPG_DATA_RDY_EN
            bool new_ppg_data = 0;
            #endif
            #if AMBIENTLIGHT_OVF_EN
            bool ambient_light_ovf = 0;
            #endif
            #if PROXIMITY_INTERRUPT
            bool proximity_interrupt = 0;
            #endif
            #if POWER_READY_FLAG_EN
            bool power_ready = 0;
            #endif

            // Interrupt Status 2 (0x00)
            #if VDD_OUT_OF_RANGE_EN
            bool vdd_out_of_range = 0;
            #endif
            #if NEW_ECG_DATA_RDY_EN
            bool new_ecg_data = 0;
            #endif
        } _interrupts;
        #endif

    public:
        MaxReg(byte numSlots = 1, byte i2cAddress = 0x5E)
            : _i2cAddress{ i2cAddress }, _numSlots{ numSlots } {}

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

            int ret = Wire.read();

            return ret;
        }

        void config()
        {
            // sys control - reset, enable, fifo config
            // write(0x02, 0x80);
            write(0x02, 0xFF);
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
            write(16U, 0xFF);

            // set led pulse amplitudes
            write(17U, 0xFF);
            write(18U, 0xFF);

            #if (SLOT_COUNT > 2)
            // ecg specific settings
            // set ECG in slot 3
            write(10U, 0x09);
            // set ECG sampling rate = 200Hz
            write(0x3C, 0x03);
            // set ECG IA gain: 9.5; PGA gain: 8
            write(0x3E, 0x0D);
            // ECG AFE config
            write(0xFF, 0x54);
            write(0xFF, 0x4D);
            write(0xCE, 0x0A);
            write(0xCF, 0x18);
            write(0xFF, 0x00);
            #if(DEBUG && VERBOSE) 
            Serial.println("ECG Configuration: Complete");
            #endif 

            #endif

            // // 0x14 = 'led range?' = led current (50 mA = 0x00)
            write(0x14, 0x00);

            #if(DEBUG && VERBOSE) 
            Serial.println("[>] PPG Configuration: Complete");
            Serial.println("[>] FIFO Configuration: Complete");
            #endif
        }

        
    };

    class WindowHandler
    {
    protected:
        unsigned int _windowLength;
        int _numSlots;

        ppgInt* _ppgWindow0;
        ppgInt* _ppgWindow1;
        ecgInt* _ecgWindow;

        bool _serial;
        bool _dataFull;

        int _slotIter[SLOT_COUNT]{ 0 };
        int _slotStatus[SLOT_COUNT]{ -1 };

    public:
        // this is pretty fucking gross - preprocessor grosser?
        WindowHandler(ppgInt arr0[FRAME_LENGTH])
        {
            _ppgWindow0 = arr0;
            _slotStatus[0] = 0;
        }

        WindowHandler(ppgInt arr0[FRAME_LENGTH], ppgInt arr1[FRAME_LENGTH])
        {
            _ppgWindow0 = arr0;
            _slotStatus[0] = 0;

            _ppgWindow1 = arr1;
            _slotStatus[1] = 0;
        }

        WindowHandler(ppgInt arr0[FRAME_LENGTH], ppgInt arr1[FRAME_LENGTH], ecgInt arr2[FRAME_LENGTH])
        {
            _ppgWindow0 = arr0;
            _slotStatus[0] = 0;

            _ppgWindow1 = arr1;
            _slotStatus[1] = 0;

            _ecgWindow = arr2;
            _slotStatus[2] = 0;
        }

        bool enabled(int slot)
        {
            return (slot != DISABLED_SLOT);
        }

        /**
         * Handles the iterator that loops through each sample array.
         * If an array is filled we set the iterator to -1 to stop storing
         * new data.
        */

        int handleIter(int slot)
        {
            if (_slotIter[slot] + 1 >= FRAME_LENGTH)
            {
                #if(DEBUG && VERBOSE) 
                Serial.println("[*] DISABLED!");
                #endif
                _slotIter[slot] = -1;

                return DATA_DISABLED;
            }
            else if (_slotIter[slot] == -1)
            {
                return DATA_FULL;
            }
            else
            {
                _slotIter[slot]++;
            }

            return 0;
        }

        template <typename sampleInt>
        int pushSample(byte slot, sampleInt sample)
        {

            switch (slot)
            {
            case PPG_SLOT0:
                _ppgWindow0[_slotIter[0]] = sample;
                _slotStatus[0] = (handleIter(PPG_SLOT0) > 0) ? 1 : 0;
                break;
            case PPG_SLOT1:
                _ppgWindow1[_slotIter[1]] = sample;
                _slotStatus[1] = (handleIter(PPG_SLOT1) > 0) ? 1 : 0;
                break;
            case ECG_SLOT0:
                _ecgWindow[_slotIter[2]] = sample;
                _slotStatus[2] = (handleIter(ECG_SLOT0) > 0) ? 1 : 0;
                break;
            default:
                #if (DEBUG && VERBOSE)
                Serial.println("Something went wrong?");
                #endif
                return ERROR;
                break;
            }
            bool check = _slotStatus[0] && _slotStatus[1] && _slotStatus[2];

            #if(DEBUG && VERBOSE)
            Serial.print("[>] ");
            Serial.print(_slotIter[0]);
            Serial.print("/");
            Serial.print(_slotIter[1]);
            Serial.print("/");
            Serial.print(_slotIter[2]);
            Serial.print("$: ");
            Serial.print(_slotStatus[0]);
            Serial.print(_slotStatus[1]);
            Serial.print(_slotStatus[2]);
            Serial.print("=");
            Serial.println(check);
            #endif

            if (check)
            {
                _dataFull = true;
                return DATA_FULL;
            }

            return check;
        }

        bool dataFull()
        {
            return _dataFull;
        }

        int restartSampling()
        {
            if ((_slotIter[0] == -1) && (_slotIter[1] == -1) && (_slotIter[2] == -1))
            {
                _slotIter[0] = 0;
                _slotIter[1] = 0;
                _slotIter[2] = 0;

                _dataFull = 0;
                #if (DEBUG && VERBOSE)
                Serial.println("[!] SAMPLING ITERATORS RESET");
                #endif
                return 1;
            }
            #if (DEBUG && VERBOSE)
            Serial.println("[!] NOT ALL SAMPLE ITERATORS DISABLED - ITER STATUS UNCHANGED");
            #endif
            return -1;
        }
    };

    class MaxFifo
    {

    protected:
        byte _numSlots;
        int _available;
        int _readBytes;

        MaxReg* _reg;
        WindowHandler* _windowHandler;

    public:
        MaxFifo(MaxReg* reg, WindowHandler* windowHandler)
            : _reg{ reg }, _windowHandler{ windowHandler }
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

        template <typename sampleInt>
        sampleInt readSample(sampleInt temp, int slot)
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
            
            // Serial.println(temp);

            return temp = (ecg) ? (ecgInt)temp & 0x3FFFF : (ppgInt)temp & 0x7FFFF;
        }

        /**
         * Get a sample from each slot and push to window handler
         * returns DATA_FULL (5) if full
        */

        int collectSample(int readBytes)
        {
            if (_windowHandler->dataFull()) {
                return DATA_FULL;
            }

            int check;
            Wire.beginTransmission(I2C_ADDRESS);
            Wire.write(7U); // queue bytes to fifo data 0x07
            Wire.endTransmission();
            Wire.requestFrom(I2C_ADDRESS, readBytes);

            for (int rB = readBytes; rB > 0; rB -= (SLOT_COUNT * 3))
            {
                #if (SLOT_COUNT >= 1)
                ppgInt temp = (ppgInt)readSample(temp, PPG_SLOT0);
                check = _windowHandler->pushSample(PPG_SLOT0, temp);
                #endif
                #if (SLOT_COUNT >= 2)
                temp = (ppgInt)readSample(temp, PPG_SLOT1);
                check = _windowHandler->pushSample(PPG_SLOT1, temp);
                #endif
                #if (SLOT_COUNT >= 3)
                ecgInt temps = (ecgInt)readSample(temps, ECG_SLOT0);
                check = _windowHandler->pushSample(ECG_SLOT0, temps);
                #endif
            }

            return check;
        }

        /**
         * For available samples read in chunks of 32 bytes.
         * For available bytes, burst read from Fifo.
         * @returns 0 on successful read, 1 on read returning DATA_FULL
        */

        int sample()
        {
            // change to for loop
            while (_available > 0)
            {
                _readBytes = _available;

                if (_readBytes > 32)
                    _readBytes = (32 - (32 % (3 * SLOT_COUNT)));

                _available -= _readBytes;

                while (_readBytes > 0)
                {
                    int ret;
                    if (ret = collectSample(_readBytes) > 0)
                    {
                        return ret;
                        // _windowHandler->restartSampling(); 
                    }
                    _readBytes -= SLOT_COUNT * 3;
                }
            }
            return 0;
        }

        /**
         *  Check to see if samples are available in the 86150's FIFO.
         *  If so we sample the data. Available samples are tracked as class member.
        */

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

                if (_available > 0)
                {
                    if (sample() > 0) {
                        // if (ret == 5)
                        // {
                        //     // if data is full, send 0 to not transmit - make a more explicit condition
                        //     // why? to prevent transmission stick to flags
                        //     return 0;
                        // }
                        return 1;
                    }
                }
            }
            return 0;
        }
    };

    class BpmSensor
    {

    protected:
        MaxReg* _reg;
        MaxFifo* _fifo;
        WindowHandler* _windowHandler;

    public:
        BpmSensor(MaxFifo* fifo, WindowHandler* windowHandler)
            : _fifo{ fifo }, _windowHandler{ windowHandler }
        {
        }

        void init()
        {
            _fifo->config();
        }

        int sample()
        {
            return _fifo->check();
        }

        int restartSampling()
        {
            return _windowHandler->restartSampling();
        }
    };
}

#endif // HF_BPM_MAX