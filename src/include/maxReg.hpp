// #ifndef HF_MAX_REG
// #define HF_MAX_REG

// #include <Wire.h>
// #include <Arduino.h>

// class MaxReg {

//     protected:
//         // add local reg map
//         int _i2cAddress; // default for now, add config

//     public:
//         MaxReg(int i2cAddress = byte(0x5E))
//         :_i2cAddress{i2cAddress} {}

//         void write(byte reg, byte value)
//         {  
//             // fix
//             // is this uninitialized?
//             // if(this->localRegi[reg])
//             // this->localRegi[reg].setLocalVal(value);

//             // track register value with map
//             // this->_localRegi.emplace(reg_pair(reg, value));

//             Wire.beginTransmission(this->_i2cAddress);
//             Wire.write(reg);
//             Wire.write(value);
//             Wire.endTransmission();
//         }

//         byte read(byte reg)
//         {
//             Wire.beginTransmission(this->_i2cAddress);
//             Wire.write(reg);
//             Wire.endTransmission(false);

//             Wire.requestFrom(this->_i2cAddress, 1);

//             int available = Wire.available();
//             if(available > 0) 
//             {
//                 int ret = Wire.read();
//                 // this->_localRegi.emplace(reg_pair(reg, temp));
//                 return ret;
//             }

//             return -1;
//         }

//         void config() {

//             // sys control - reset, enable, fifo setup
//             write(0x0D, 0x01);
//             write(0x0D, 0x04);
//             write(0x08, 0x7F);

//             // sl1
//             write(9U, 0x01);
//             // sl2
//             write(10U, 0x09);
            
//             // PPG config (protocentral 0xD1 for Config1, maxim 0xD3) 0xD7 us: adc range, sample rate=200/s, led pulse width
//             write(14U, 0xD7);
//             // PPG sample averging
//             write(15U, 0x00);

//             // set led pulse amplitudes
//             write(17U, 0xFF);
//             write(18U, 0xFF);

//             // // set ECG sampling rate = 200Hz
//             // writeRegister(ECG_CONFIG1, byte(0x03));
//             // // set ECG IA gain: 9.5; PGA gain: 8 (idk what this means)
//             // writeRegister(ECG_CONFIG3, byte(0x0D));

//             // // 0x14 = 'led range?' = led current (50 mA = 0x00)
//             write(0x14, 0x00);

//         }


// };

// #endif