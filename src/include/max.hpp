#ifndef HF_BPM_MAX
#define HF_BPM_MAX


#include <ArduinoJson.hpp>
#include <Wire.h>

// #include <string>
// #include <map>
#include <array>
#include <utility>
#include <vector>

#define WINDOW_SIZE 256
#define FRAME_SIZE 64

#define SR 200 // Sampling rate remove when reg lib fully implemented
#define ST 5   // Sampling period (ms) ^


#include ".env.h" // WiFi credentials

namespace hf
{

    class MaxSensor 
    {

        protected:
            const byte _i2cAddress = byte(0x5E);
            // arduino::TwoWire _i2c;
            int _windowLength = WINDOW_SIZE;
            byte _numSlots = 1;

            // std::map<std::string, Reg> _rmap;
            // std::map<byte, byte> _localRegi;
            std::vector<uint32_t> _ppgWindow;
            // BpmWiFi _bpmWiFi;


        public:
        
            MaxSensor(byte i2cAddress = 0x5E)
            : _i2cAddress{i2cAddress}
            {}

            void initRegi() 
            {
                // current hardcoded sequence - look at ArduinoMain copy to see more detail

                // sys control - reset, enable, fifo setup
                writeRegister(0x0D, 0x01);
                writeRegister(0x0D, 0x04);
                writeRegister(0x08, 0x7F);

                // sl1
                writeRegister(9U, 0x01);
                // sl2
                writeRegister(10U, 0x00);
                
                // PPG config (protocentral 0xD1 for Config1, maxim 0xD3) 0xD7 us: adc range, sample rate=200/s, led pulse width
                writeRegister(14U, 0xD7);
                // PPG sample averging
                writeRegister(15U, 0x00);

                // set led pulse amplitudes
                writeRegister(17U, 0xFF);
                writeRegister(18U, 0xFF);

                // // set ECG sampling rate = 200Hz (I think PPG sr is based off this)
                // writeRegister(ECG_CONFIG1, byte(0x03));
                // // set ECG IA gain: 9.5; PGA gain: 8 (idk what this means)
                // writeRegister(ECG_CONFIG3, byte(0x0D));

                // // 0x14 = 'led range?' = led current (50 mA = 0x00)
                writeRegister(0x14, 0x00);

                clearFifo();
            }

            void writeRegister(byte reg, byte value)
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

            byte readRegister(byte reg)
            {
                Wire.beginTransmission(this->_i2cAddress);
                Wire.write(reg);
                Wire.endTransmission(false);

                Wire.requestFrom(this->_i2cAddress, byte(1));

                int bytesAvailable = Wire.available();
                if(bytesAvailable > 0) 
                {
                    int temp = Wire.read();
                    // this->_localRegi.emplace(reg_pair(reg, temp));
                    return temp;
                }

                return -1;
            }

            void clearFifo(void) 
            {
                // reset 0x04 = FIFO write ptr, 0x05 = FIFO overflow, 0x06 = FIFO read ptr
                this->writeRegister(byte(0x04), 0);
                this->writeRegister(byte(0x05), 0);
                this->writeRegister(byte(0x06), 0);
            }

            void fifoReadLoop() {
                int sampleRange = 0;
                byte readPtr = this->readRegister(6U);
                byte writePtr = this->readRegister(4U);

                if (readPtr != writePtr)
                {
                    // if we found data get 
                    sampleRange = writePtr - readPtr;
                    if(sampleRange < 0) sampleRange += 32;

                    // = num samples * num devices * bits per sample
                    int remaining = sampleRange * _numSlots * 3;

                    Wire.beginTransmission(_i2cAddress);
                    Wire.write(7U); // queue bytes
                    Wire.endTransmission();

                    while(remaining > 0)
                    {
                        int tempRem = remaining;

                        // buffer len is 32 on Uno, 64 on 
                        if(tempRem > 32)
                        {
                            tempRem = 32 - (32 % 3 * _numSlots); // 3 * 1 = bytes * devices
                        }
                        remaining -= tempRem;

                        Wire.requestFrom(_i2cAddress, byte(tempRem));

                        byte tempArr[4]; // 4 bytes = sizeof(uint32_t) sizeof(unsigned long)
                        uint32_t tempLong;

                        while (tempRem > 0)
                        {
                            for(int i = 0; i < _numSlots; i++){
                                tempArr[3] = 0;
                                tempArr[2] = Wire.read();
                                tempArr[1] = Wire.read();
                                tempArr[0] = Wire.read();

                                // convert 4 bytes into long
                                memcpy(&tempLong, tempArr, sizeof(tempLong));
                                // zero all but 19 bits - why?
                                tempLong &= 0x7FFFF;

                                _ppgWindow.push_back(tempLong);

                                if(_ppgWindow.size() >= WINDOW_SIZE) {
                                    return;
                                }
                                
                                // Serial.println(_ppgWindow.size());

                                Serial.print(_ppgWindow.back());
                                // Serial.println(',');

                                tempRem -= 3;
                                delay(1);
                            }
                        }
                    }
                }
            }

            std::vector<uint32_t> getWindow()
            {
                std::vector<uint32_t> tmp = _ppgWindow;
                clearWindow();
                return tmp;
            }

            void clearWindow()
            {
                _ppgWindow.clear();
            }

            // int sampleSensor()
            // {
            //     unsigned long start = millis();
            // }

            byte getI2CAddress() 
            {
                return this->_i2cAddress;
            }

    };


}

#endif // HF_MAX