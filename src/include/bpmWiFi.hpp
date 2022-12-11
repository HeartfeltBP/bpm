#ifndef HF_BPM_WIFI
#define HF_BPM_WIFI

#include <Arduino.h>

#include <string>
#include <vector>
#include <array>

#include <ArduinoJson.hpp>
#include <WiFiNINA.h>
#include <HttpClient.h>

#include ".env.h" // WiFi credentials

namespace hf
{
    static const std::string apiEndpoints[2] = {
            "/api/rx/",
            "/api/test/"
        };

    static const std::string jsonReciever = apiEndpoints[0];
    static const std::string testEndpoint = apiEndpoints[1];

    class BpmWiFi
    {
        protected:
        std::string _url;
        WiFiClient _client;
        HttpClient _http = HttpClient(_client, URL, LPORT);

        byte _wlStatus = WL_IDLE_STATUS;
        byte _clStatus = 0;

        public:

            // place within try catch block
            BpmWiFi(std::string url = URL, std::string ssid = SSID, std::string password = PASS)
            : _url{url} {}

            int connectWiFi(std::string ssid, std::string password)
            {
                if (WiFi.status() == WL_NO_MODULE)
                {
                    return -1;
                }
                _wlStatus = WiFi.begin(SSID, PASS);
                delay(500);
                
                if(_wlStatus != WL_CONNECTED) {
                    int giveUp = 20;
                    while (_wlStatus != WL_CONNECTED)
                    {
                        Serial.print("WiFi connect attempt failed, trying again ");
                        Serial.print(giveUp); Serial.println(" reconnect attempts left...");
                        Serial.print("WiFi status: "); Serial.println(_wlStatus);
                        _wlStatus = WiFi.begin(SSID, PASS);
                        if(giveUp <= 0) { Serial.print("Giving up: setup failed"); return -1; }
                        giveUp--;
                        delay(1000);
                    }
                }
                return 0;
            }

            int connectClient(std::string url)
            {
                _clStatus = _client.connect(URL, LPORT);

                if(!_clStatus) {
                    int giveUp = 20;
                    while (!_clStatus)
                    {
                        Serial.print("Client connect attempt failed, trying again ");
                        Serial.print(giveUp); Serial.println(" reconnect attempts left...");
                        Serial.print("Client status: "); Serial.println(_clStatus);
                        _clStatus = _client.connect(URL, LPORT);
                        if(giveUp <= 0) {Serial.print("Giving up: setup failed"); return -1;}
                        giveUp--;
                        delay(1000);
                    }
                }
                return 0;
            }

            int initWiFi(std::string ssid, std::string pass, std::string url) {
                if(connectWiFi(ssid, pass) >= 0 && connectClient(url) >= 0 ) {
                    return 0;
                } else { return -1; }
            }

            // make vector->array copying a utility function so it could be run independantly from WiFi or BLE
            void txWindow(std::vector<uint32_t> ppgWindow) {
                if (WiFi.status() != WL_CONNECTED || !_client.status()) {retryWiFi();}

                uint32_t txArr[256];
                std::copy(ppgWindow.begin(), ppgWindow.end(), txArr);
                ArduinoJson::StaticJsonDocument<256 * 16> ppgJson;
                ArduinoJson::copyArray(txArr, ppgJson.to<ArduinoJson::JsonArray>());
                Serial.println((ppgJson.memoryUsage()/4)+800);

                _http.beginRequest();
                _http.post(jsonReciever.c_str());
                _http.sendHeader("User-Agent", "Arduino/1.0");
                _http.sendHeader("Content-Type", "application/json");
                // TODO: figure out how tf Content-length is calculated? <- len of string of transmission i think
                _http.sendHeader("Content-Length", ArduinoJson::measureJson(ppgJson));
                _http.sendHeader("Connection", "keep-alive");
                _http.beginBody();
                ArduinoJson::serializeJson(ppgJson, _http);
                ArduinoJson::serializeJson(ppgJson, Serial);
                Serial.println();
                _http.endRequest();

                ppgJson.clear();
                ppgJson.garbageCollect();
                delete &ppgJson;
                // Serial.print(_http.responseBody());
            }   

            void getTest() {
                if (WiFi.status() != WL_CONNECTED || !_client.status()) {retryWiFi();}
                _http.get(testEndpoint.c_str());
                Serial.print(_http.responseBody());
            }

            void retryWiFi() {
                if (WiFi.status() != WL_CONNECTED) {
                    _client.stop();
                    WiFi.end();
                    initWiFi(SSID, PASS, URL);
                }
                if (!_client.status()) {
                     _client.stop();
                     connectClient(URL);
                }
            }

    };
}


#endif