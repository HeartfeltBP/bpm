// #ifndef HF_BPM_FIFO
// #define HF_BPM_FIFO

// #include <Wire.h>

// // #include <string>
// // #include <map>
// #include <array>
// #include <utility>
// #include <vector>

// #include "maxReg.hpp"

// namespace hf
// {

//     class MaxFifo 
//     {

//         protected:
//             const byte _i2cAddress;
//             int _windowLength;
//             byte _numSlots;
//             byte _longBytes = 4;

//             std::vector<uint32_t> _ppgWindow;
//             // std::vector<uint32_t> _ppgWindow1;
//             //std::vector<uint32_t> _ecgWindow;

//             MaxReg reg;

//         public:
        
//             MaxFifo(byte numSlots = 1, byte i2cAddress = 0x5E, int windowLength = 256)
//             : _numSlots{numSlots}, _i2cAddress{i2cAddress}, _windowLength{windowLength}
//             { _longBytes = sizeof(uint32_t); }

//             void clear() 
//             {
//                 // reset 0x04 = FIFO write ptr, 0x05 = FIFO overflow, 0x06 = FIFO read ptr
//                 reg.write(byte(0x04), 0);
//                 reg.write(byte(0x05), 0);
//                 reg.write(byte(0x06), 0);
//             }

//             int range() {
//                 byte readPtr = reg.read(6U);
//                 byte writePtr = reg.read(4U);
//                 return writePtr - readPtr;
//             }

//             void read(int numSlots) {
//                 byte longArr[_longBytes];
//                 uint32_t tempLong;

//                 for(int i = 0; i < numSlots; i++) {
//                     longArr[3] = 0;
//                     longArr[2] = Wire.read();
//                     longArr[1] = Wire.read();
//                     longArr[0] = Wire.read();

//                     // convert 4 bytes into long
//                     memcpy(&tempLong, longArr, sizeof(tempLong));

//                     // zero all but 19 bits
//                     tempLong &= 0x7FFFF;
//                     // _ppgWindow.push_back(tempLong);

//                     // Serial.print("PPG: ");
//                     Serial.print(tempLong);
//                     Serial.println(",");
//                 }
//             } 

//             void check() {
//                 int fifoRange = range();
//                 if (fifoRange != 0)
//                 {
//                     // if we found data get
//                     if(fifoRange < 0) fifoRange += 32;

//                     // avail = num samples * num devices * bits per sample
//                     int available = fifoRange * _numSlots * 3;

//                     Wire.beginTransmission(_i2cAddress);
//                     Wire.write(7U); // queue bytes to fifo data 0x07
//                     Wire.endTransmission();

//                     while(available > 0)
//                     {
//                         int readBytes = available;

//                         // i2c buffer len is 32 on Uno, 64 on Nano? - possibly not implemented in Wire lib
//                         // so tempRem is limited by buffer len and there may be remaining bytes beyond
//                         //                                          bytes * slots
//                         if(readBytes > 32) readBytes = 32 - ( 32 % (3 * _numSlots) );

//                         available -= readBytes;

//                         Wire.requestFrom(_i2cAddress, readBytes);

//                         while(readBytes > 0) {
//                             read(_numSlots);
//                             readBytes -= _numSlots;

//                             if(_ppgWindow.size() >= (std::size_t)_windowLength) {
//                                 // _bpmWiFi.getTest();
//                                 // because we might break and skip some samples?
//                                 if(readBytes > 0) available += readBytes;
//                                 break;
//                             }
//                         }
//                     }
//                 }
//             }

//             byte getI2CAddress() 
//             {
//                 return this->_i2cAddress;
//             }

//     };
// }

// #endif // HF_MAX