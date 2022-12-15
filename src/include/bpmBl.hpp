#ifndef HF_BPM_BL
#define HF_BPM_BL

#include <CommandParser.h>
#include <ArduinoBLE.h>
#include <ArduinoJson.hpp>
#include <HardwareBLESerial.h>

#include <string>
#include <vector>

namespace hf 
{

    class BpmBleSerial {

        protected:
            HardwareBLESerial &_bleSerial = HardwareBLESerial::getInstance();
            std::string _bleName;

        public:
            BpmBleSerial(std::string bleName = "bpm") 
            : _bleName{bleName} {
                if(!_bleSerial.beginAndSetupBLE(_bleName.c_str())) {
                    Serial.println("ERROR: failed to initialize Serial BLE");
                }
            }

        
            void txWindow(std::vector<uint32_t> ppgWindow) {
                uint32_t txArr[256];
                std::copy(ppgWindow.begin(), ppgWindow.end(), txArr);
                ArduinoJson::StaticJsonDocument<256 * 16> ppgJson;
                ArduinoJson::copyArray(txArr, ppgJson.to<ArduinoJson::JsonArray>());
                Serial.println(ppgJson.data().size());
            }
    };
}

#endif