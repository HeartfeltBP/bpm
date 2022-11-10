#ifndef HF_BPM_WIFI
#define HF_BPM_WIFI

#include <Arduino.h>

#include <string>
#include <vector>

#include <ArduinoJson.hpp>
#include <WiFiNINA.h>

#include ".env.h" // WiFi credentials

namespace hf
{
    class BpmWiFi
    {
        protected:
        std::string _url;
        WiFiClient _client;
        byte _wlStatus = WL_IDLE_STATUS;
        byte _clStatus;

        public:
            // add .env or something for pass management (maybe just don't handle on arduino somehow?)
            // maybe encrypt on external device, decrypt on arduino (just probably setup two way encryption in general)

            // place within try catch block
            BpmWiFi(std::string url = URL, std::string ssid = SSID, std::string password = PASS)
            : _url{url} {
                int status;
                status = connectWiFi(ssid, password);

                if (status < 0) {
                    // std::__throw_runtime_error("connectWiFi failed");
                    Serial.println("FAILED");
                } else {
                    status = connectClient(url);
                    if (status < 0) {
                        // std::__throw_runtime_error("connectClient failed");
                        Serial.println("FAILED");
                    }
                }
            }

            int connectWiFi(std::string ssid, std::string password)
            {
                if(WiFi.status() == WL_NO_MODULE) {
                    // std::__throw_runtime_error("No WiFi module detected!");
                    return -1;
                }
                _wlStatus = WiFi.begin(ssid.c_str(), password.c_str());

                if(_wlStatus != WL_CONNECTED) {
                    Serial.println("WiFi not connected, retrying...");
                }

                delay(10);
                _wlStatus = WiFi.begin(ssid.c_str(), password.c_str());

                if(_wlStatus != WL_CONNECTED) {
                    _wlStatus = WL_DISCONNECTED;
                    return -1;
                }
                // maybe print to log file or BLE characteristic or something?
                Serial.print("WiFi connected to ");
                Serial.println(ssid.c_str());

                return 0;
            }

            int connectClient(std::string url)
            {
                _clStatus = _client.connect(url.c_str(), 80);

                if(_clStatus) {
                    Serial.print("Client connected to ");
                    Serial.println(url.c_str());
                    return 0;
                } else {
                    Serial.println("Client failed to connect");
                    return -1;
                }
            }

            void txWindow(std::vector<uint32_t> ppgWindow) 
            {
                if(!_clStatus || !_wlStatus) {
                    return;
                }

                int txArr[256];
                
                // find a way to avoid doing this
                // for(int i = 0; i < WINDOW_SIZE; i++) {
                //     txArr[i] = _ppgWindow.data()[i];
                // }

                // std::copy(&_ppgWindow.front(), &_ppgWindow.back(), txArr);
                std::copy(&ppgWindow.front(), &ppgWindow.at(256), txArr);
                
                ArduinoJson::StaticJsonDocument<256 * 4> ppgJson;
                ArduinoJson::copyArray(txArr, ppgJson.to<ArduinoJson::JsonArray>());
                ArduinoJson::serializeJsonPretty(ppgJson, Serial);
                
                std::string tmpString = "POST " + jsonReciever + " HTTP/1.1";
                Serial.println(tmpString.c_str());
                _client.println(tmpString.c_str());

                tmpString = "Host: " + _url;
                _client.println(tmpString.c_str());

                _client.println("Content-Type: application/json");
                _client.println("Accept: */*"); // Set MIME types
                _client.println("Cache-Control: no-cache");
                _client.println("Accept-Encoding: gzip, deflate");
                _client.println("Accept-Language: en-us");

                tmpString = "Conetent-Length: " + ppgWindow.size();
                _client.println(tmpString.c_str());
                
                _client.println("Connection: close");
                _client.println();
                ArduinoJson::serializeJson(ppgJson, _client);
                _client.println();

            }

            

    };
}


#endif