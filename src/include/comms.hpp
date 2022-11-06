#ifndef HF_BPM_COMMS
#define HF_BPM_COMMS

#include <string>
#include <WiFiNINA.h>

namespace hf
{
    class BpmWiFi
    {
        protected:
        WiFiClient _client;
        byte _status = WL_IDLE_STATUS;

        public:
            // add .env or something for pass management (maybe just don't handle on arduino somehow?)
            // maybe encrypt on external device, decrypt on arduino (just probably setup two way encryption in general)

            BpmWiFi(std::string ssid, std::string password)
            {
                _status = WiFi.begin(ssid.c_str(), password.c_str());

                if(_status != WL_CONNECTED) {
                    Serial.println("WiFi not connected, retrying...");
                }

                _status = WiFi.begin(ssid.c_str(), password.c_str());

                if(_status != WL_CONNECTED) {
                    Serial.println("WiFi not connected, exiting");
                    _status = WL_DISCONNECTED;
                }
                
            }

            

    }
};


#endif