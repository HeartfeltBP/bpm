// #include <string>
// #include <utility>

// // Register Map for MAX86150

// #include <Arduino.h>
// #include <utility>
// #include <string>
// // #include <array>
// // #include <bitset>

// // TODO: add include guards

// typedef std::pair<std::string, byte> reg_pair;

// struct bitRange
// {
//     byte from;
//     byte to;

//     bitRange(byte from, byte to)
//         : from{from}, to{to} {}

//     bitRange(byte index)
//         : from{index}, to{index} {}
//     // workaround - TODO: remove redundant variable
// };

// namespace hf
// {

//     class RegSetting;
//     /**
//      * @brief
//      *
//      */
//     class Reg
//     {
//     protected:
//         // get rid of string with defines for efficiency
//         std::string _name;
//         const byte _address;         // address of register
//         byte _localVal = 0b00000000; // current register values
//         RegSetting *_settings;

//     public:
//         Reg(std::string name = "tmp", byte address = 0x00, RegSetting *settings = nullptr, byte initialVal = 0x00)
//             : _address{address}, _localVal{initialVal}, _settings{settings}
//         {
//         }

//         Reg(std::string name, byte address, RegSetting *settings)
//             : _address{address}, _settings{settings}
//         {
//         }

//         void setSettings(RegSetting *settings)
//         {
//             _settings = settings;
//         }

//         byte getLocalVal()
//         {
//             return _localVal;
//         }

//         byte setLocalVal(byte val)
//         {
//             _localVal = val;
//         }

//         byte getAddress()
//         {
//             return _address;
//         }
//     };

//     class RegSetting
//     {
//     protected:
//         Reg _reg;
//         std::string _name;
//         reg_pair *_options; // array of possible options
//         bitRange _range;

//     public:
//         RegSetting();

//         RegSetting(Reg reg, std::string name, reg_pair *options, bitRange range)
//             : _reg{reg}, _name{name}, _options{options}, _range{range} {}

//         RegSetting(Reg reg, char *name, reg_pair *options, bitRange range)
//             : _reg{reg}, _name{std::string(name)}, _options{options}, _range{range} {}

//         std::string getName()
//         {
//             return _name;
//         }

//         reg_pair *getOptions()
//         {
//             return _options;
//         }

//         byte getAddress()
//         {
//             return _reg.getAddress();
//         }

//         bitRange getRange()
//         {
//             return _range;
//         }

//         Reg getReg()
//         {
//             return _reg;
//         }

//         byte setSetting(byte writeVal)
//         {
//             int i, j = 0;
//             // could also find dynamic way to generate masks
//             byte temp = _reg.getLocalVal();
//             for (i = _range.to; i < _range.from; i++, j++)
//             {
//                 bitWrite(temp, i, bitRead(writeVal, j)); // consider finding arduino agnostic function (bitset?)
//             }
//             _reg.setLocalVal(temp);
//             return temp;
//         }
//     };

// }

// reg_pair *bool_opts(byte off, byte on)
// {
//     reg_pair *arr = new reg_pair[2];
//     arr[0].first = "off";
//     arr[0].second = off;
//     arr[1].first = "on";
//     arr[1].second = on;
//     return arr;
// }

// reg_pair *lin_opts(int n)
// {
//     reg_pair *arr = new reg_pair[n];
//     for (int i = 0; i < n; i++)
//     {
//         arr[i].first = std::string((char *)i);
//         arr[i].second = i;
//     }
//     return arr;
// }

// byte off = 0, on = 1;

// hf::RegSetting *generateSettingsMap()
// {
//     hf::Reg Int_Status1 = hf::Reg("Int_Status1", 0x00);
//     // std::array<hf::RegSetting, 5> is1_settings <- use array obj in future
//     hf::RegSetting is1_settings[5] = {
//         hf::RegSetting(Int_Status1, std::string("almost_full_flag"), bool_opts(off, on), bitRange(7)),
//         hf::RegSetting(Int_Status1, std::string("new_ppg_data_ready_flag"), bool_opts(off, on), bitRange(6)),
//         hf::RegSetting(Int_Status1, std::string("ambient_light_overflow_flag"), bool_opts(off, on), bitRange(5)),
//         hf::RegSetting(Int_Status1, std::string("proximity_int"), bool_opts(off, on), bitRange(4)),
//         hf::RegSetting(Int_Status1, std::string("power_ready"), bool_opts(off, on), bitRange(0))};
//     Int_Status1.setSettings(is1_settings);

//     hf::Reg Int_Status2 = hf::Reg("Int_Status2", 0x01);
//     hf::RegSetting is2_settings[2] = {
//         hf::RegSetting(Int_Status2, std::string("vdd_out_of_range_flag"), bool_opts(off, on), bitRange(7)),
//         hf::RegSetting(Int_Status2, std::string("new_ecg_data_ready_flag"), bool_opts(off, on), bitRange(2))};
//     Int_Status2.setSettings(is2_settings);

//     hf::Reg Int_Enable1 = hf::Reg("Int_Enable1", 0x02);
//     hf::RegSetting ie1_settings[4] = {
//         hf::RegSetting(Int_Enable1, std::string("almost_full_flag"), bool_opts(off, on), bitRange(7)),
//         hf::RegSetting(Int_Enable1, std::string("new_ppg_data_int"), bool_opts(off, on), bitRange(6)),
//         hf::RegSetting(Int_Enable1, std::string("ambient_light_overflow_int"), bool_opts(off, on), bitRange(5)),
//         hf::RegSetting(Int_Enable1, std::string("proximity_int"), bool_opts(off, on), bitRange(4))};
//     Int_Enable1.setSettings(ie1_settings);

//     hf::Reg Int_Enable2 = hf::Reg("Int_Enable2", 0x03);
//     hf::RegSetting ie2_settings[2] = {
//         hf::RegSetting(Int_Enable2, std::string("proximity_int"), bool_opts(off, on), bitRange(7)),
//         hf::RegSetting(Int_Enable2, std::string("vdd_out_of_range_int"), bool_opts(off, on), bitRange(2))};
//     Int_Enable2.setSettings(ie2_settings);

//     // TODO: need to figure out specifics of what to do with these
//     // bit range for all of these is 7:0

//     // reg_setting fwp_settings[2] = {
//     //     regSetting()
//     // };
//     // reg Fifo_Write_Ptr = reg(0x04, fwp_settings);

//     // reg_setting foc_settings[2] = {
//     //     regSetting()
//     // };
//     // reg Fifo_Overflow_Counter = reg(0x05, foc_settings);

//     // reg_setting frp_settings[2] = {
//     //     regSetting()
//     // };
//     // reg Fifo_Read_Ptr = reg(0x06, frp_settings);

//     // reg_setting fd_settings[2] = {
//     //     reg_setting()
//     // };
//     // reg Fifo_Data = reg(0x07, fd_settings);

//     hf::Reg Fifo_Config = hf::Reg("Fifo_Config", 0x08);
//     hf::RegSetting fc_settings[4] = {
//         hf::RegSetting(Fifo_Config, std::string("almost_full_int_read_clear"), bool_opts(off, on), bitRange(6)),
//         hf::RegSetting(Fifo_Config, std::string("almost_full_flag_persist"), bool_opts(off, on), bitRange(5)),
//         hf::RegSetting(Fifo_Config, std::string("fifo_roll_on_full"), bool_opts(off, on), bitRange(4)),
//         hf::RegSetting(Fifo_Config, std::string("fifo_free_space_before_int"), lin_opts(15), bitRange(3, 0))};
//     Fifo_Config.setSettings(fc_settings);

//     reg_pair slot_options[5] = {
//         {"ppg_led1_ir", 0b0101},
//         {"ppg_led2_red", 0b0110},
//         {"pilot_led1_ir", 0b1001},
//         {"pilot_led_red", 0b1010},
//         {"ecg", 0b1101}};

//     hf::Reg Fifo_Data_Control1 = hf::Reg("Fifo_Data_Control1", 0x09);
//     hf::RegSetting fdc1_settings[2] = {
//         hf::RegSetting(Fifo_Data_Control1, std::string("fifo_data1_time_slot2"), slot_options, bitRange(7, 4)),
//         hf::RegSetting(Fifo_Data_Control1, std::string("fifo_data1_time_slot1"), slot_options, bitRange(3, 0))};
//     Fifo_Data_Control1.setSettings(fdc1_settings);

//     hf::Reg Fifo_Data_Control2 = hf::Reg("Fifo_Data_Control2", 0x0A);
//     hf::RegSetting fdc2_settings[2] = {
//         hf::RegSetting(Fifo_Data_Control2, std::string("fifo_data2_time_slot2"), slot_options, bitRange(7, 4)),
//         hf::RegSetting(Fifo_Data_Control2, std::string("fifo_data2_time_slot1"), slot_options, bitRange(3, 0))};
//     Fifo_Data_Control2.setSettings(fdc2_settings);

//     hf::Reg Sys_Control = hf::Reg("Sys_Control", 0x0D);
//     hf::RegSetting sysc_settings[3] = {
//         hf::RegSetting(Fifo_Config, std::string("fifo_enable"), bool_opts(off, on), bitRange(2)),
//         hf::RegSetting(Fifo_Config, std::string("shutdown_control"), bool_opts(off, on), bitRange(1)),
//         hf::RegSetting(Fifo_Config, std::string("reset_control"), bool_opts(off, on), bitRange(0)),
//     };
//     Sys_Control.setSettings(sysc_settings);

//     // spo2 specific?
//     reg_pair adc_range_options[4] = {
//         {"4096nA", 0b00},
//         {"8192nA", 0b01},
//         {"16384nA", 0b10},
//         {"32768nA", 0b11}};

//     reg_pair sample_rate_options[16] = {
//         {"sr:10_pulses:1", 0b0000},
//         {"sr:20_pulses:1", 0b0001},
//         {"sr:50_pulses:1", 0b0010},
//         {"sr:84_pulses:1", 0b0011},
//         {"sr:100_pulses:1", 0b0100},
//         {"sr:200_pulses:1", 0b0101},
//         {"sr:400_pulses:1", 0b0110},
//         {"sr:800_pulses:1", 0b0111},
//         {"sr:1000_pulses:1", 0b1000},
//         {"sr:1600_pulses:1", 0b1001},
//         {"sr:3200_pulses:1", 0b1010},
//         {"sr:10_pulses:2", 0b1011},
//         {"sr:20_pulses:2", 0b1100},
//         {"sr:50_pulses:2", 0b1101},
//         {"sr:84_pulses:2", 0b1110},
//         {"sr:100_pulses:2", 0b1111},
//     };

//     reg_pair pulse_width_options[4] = {
//         {"50us", 0b00},
//         {"100us", 0b01},
//         {"200us", 0b10},
//         {"400us", 0b11}};

//     hf::Reg Ppg_Config1 = hf::Reg("Ppg_Config1", 0x0E);
//     hf::RegSetting ppgc1_settings[3] = {
//         hf::RegSetting(Ppg_Config1, std::string("adc_range"), adc_range_options, bitRange(7, 6)),
//         hf::RegSetting(Ppg_Config1, std::string("sample_rate"), sample_rate_options, bitRange(5, 2)),
//         hf::RegSetting(Ppg_Config1, std::string("led_pulse_width"), pulse_width_options, bitRange(1, 0)),
//     };
//     Ppg_Config1.setSettings(ppgc1_settings);

//     hf::Reg Ppg_Config2 = hf::Reg("Ppg_Config2", 0x0F);
//     hf::RegSetting ppgc2_settings[1] = {
//         hf::RegSetting(Ppg_Config2, std::string("sample_averaging_value"), lin_opts(8), bitRange(2, 0)),
//     };
//     Ppg_Config2.setSettings(ppgc2_settings);

//     // hf::Reg Prox_Interrupt;

//     hf::Reg Led1_Ir_Amp = hf::Reg("Led1_Ir_Amp", 0x11);
//     hf::RegSetting led1_amp[1] = {
//         hf::RegSetting(Led1_Ir_Amp, std::string("led1_ir_pulse_amp"), lin_opts(256), bitRange(7, 0)),
//     };
//     Led1_Ir_Amp.setSettings(led1_amp);

//     hf::Reg Led2_Red_Amp = hf::Reg("Led2_Red_Amp", 0x12);
//     hf::RegSetting led2_amp[1] = {
//         hf::RegSetting(Led2_Red_Amp, std::string("led2_red_pulse_amp"), lin_opts(256), bitRange(7, 0)),
//     };
//     Led2_Red_Amp.setSettings(led2_amp);

//     // I think maxim skips the unlucky #

//     reg_pair led_range_options[2] = {
//         {"50mA", 0b00},
//         {"100mA", 0b01}};

//     hf::Reg Led_Range = hf::Reg("Led_Range", 0x14);
//     hf::RegSetting led_range[2] = {
//         hf::RegSetting(Led_Range, std::string("led1_ir_range"), led_range_options, bitRange(7, 0)),
//         hf::RegSetting(Led_Range, std::string("led2_red_range"), led_range_options, bitRange(7, 0)),
//     };
//     Led_Range.setSettings(led_range);

//     hf::Reg Led_Pilot_Amp = hf::Reg("Led_Pilot_Amp", 0x15);
//     hf::RegSetting led_pilot_amp[1] = {
//         hf::RegSetting(Led_Pilot_Amp, std::string("led_pilot_amp"), lin_opts(256), bitRange(7, 0)),
//     };
//     Led_Pilot_Amp.setSettings(led_pilot_amp);

//     // TODO: Replace all of this with hashmap

//     // hf::Reg regArr[14] = {
//     //     Int_Status1,
//     //     Int_Enable2,
//     //     Int_Enable1,
//     //     Int_Enable2,
//     //     Fifo_Config,
//     //     Fifo_Data_Control1,
//     //     Fifo_Data_Control2,
//     //     Sys_Control,
//     //     Ppg_Config1,
//     //     Ppg_Config2,
//     //     Led1_Ir_Amp,
//     //     Led2_Red_Amp,
//     //     Led_Range,
//     //     Led_Pilot_Amp,
//     // };

//     // ensure static to actually return value
//     static hf::RegSetting settingsArr[33] = {
//         is1_settings[0],
//         is1_settings[1],
//         is1_settings[2],
//         is1_settings[3],
//         is1_settings[4],

//         is2_settings[0],
//         is2_settings[1],

//         ie1_settings[0],
//         ie1_settings[1],
//         ie1_settings[2],
//         ie1_settings[3],
//         ie2_settings[0],
//         ie2_settings[1],

//         fc_settings[0],
//         fc_settings[1],
//         fc_settings[2],
//         fc_settings[3],

//         fdc1_settings[0],
//         fdc1_settings[1],
//         fdc2_settings[0],
//         fdc2_settings[1],

//         sysc_settings[0],
//         sysc_settings[1],
//         sysc_settings[2],

//         ppgc1_settings[0],
//         ppgc1_settings[1],
//         ppgc1_settings[2],
//         ppgc2_settings[0],

//         led1_amp[0],
//         led2_amp[0],

//         led_range[0],
//         led_range[1],

//         led_pilot_amp[0]};

//     return settingsArr;
// }