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
            "api/rx/",
            "api/test/"
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
            // add .env or something for pass management (maybe just don't handle on arduino somehow?)
            // maybe encrypt on external device, decrypt on arduino (just probably setup two way encryption in general)

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
                        if(giveUp <= 0) {Serial.print("Giving up: setup failed"); return -1;}
                        giveUp--;
                        delay(2000);
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
                        delay(2000);
                    }
                }
                return 0;
            }

            int initWiFi(std::string ssid, std::string pass, std::string url) {
                if(connectWiFi(ssid, pass) >= 0 && connectClient(url) >= 0 ) {
                    return 0;
                } else { return -1; }
            }

            void txWindow(std::vector<uint32_t> ppgWindow) {
                if (WiFi.status() != WL_CONNECTED || !_client.status()) {retryWiFi();}

                ArduinoJson::StaticJsonDocument<256 * 4> ppgJson;
                int tmpArr[256] = {0};
                memset(tmpArr, 69, 256);

                ArduinoJson::copyArray(tmpArr, ppgJson.to<ArduinoJson::JsonArray>());
                
                std::string tmpUrl = URL;
                std::string header = "POST " + jsonReciever + " HTTP/1.1\n" +
                                        "Host: " + tmpUrl + "\n" +
                                        "User-Agent: Arduino/1.0\n" +
                                        "Accept: */*\n" +
                                        "Cache-Control: no-cache\n" +
                                        "Accept-Encoding: gzip, deflate, br\n" +
                                        "Content-Type: application/json\n" +
                                        "Content-Length: " + std::to_string(ppgJson.size()) +
                                        "\n\n";
                
                delete &tmpUrl;
                _client.println(header.c_str());
                Serial.println(header.c_str());
                ArduinoJson::serializeJson(ppgJson, _client);
                ArduinoJson::serializeJsonPretty(ppgJson, Serial);
                // Serial.println("{}");
                // _client.println("{}");
                Serial.println("Sent");
                Serial.println(_client.readString());

                delete &header;
                // delete &ppgJson;
            }   

            void getTest() {
                if (WiFi.status() != WL_CONNECTED || !_client.status()) {retryWiFi();}
                
                std::string tmpString = "GET " + testEndpoint + " HTTP/1.1";
                Serial.println(tmpString.c_str());
                _client.println(tmpString.c_str());

                std::string tmpUrl = URL;
                tmpString = "Host: " + tmpUrl;

                _client.println(tmpString.c_str());
                
                delete &tmpString;

                _client.println("Connection: close");
                _client.println();

                Serial.println(_client.readString());
                Serial.println("Sent");
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