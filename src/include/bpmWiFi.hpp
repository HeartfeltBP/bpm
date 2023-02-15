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

// TODO: for some reason reading http response bodies causes program to hang indefinetly - fix

namespace hf
{
    static const std::string apiEndpoints[2] = {
        "/api/rx/",
        "/api/test/"};

    static const std::string jsonReciever = apiEndpoints[0];
    static const std::string testEndpoint = apiEndpoints[1];

    class BpmWiFi
    {
    protected:
        WiFiClient _client;
        HttpClient *_http;

        byte _wlStatus = WL_IDLE_STATUS;
        byte _clStatus = 0;

        int connectWiFi(std::string ssid, std::string password, boolean enterprise = false)
        {
            if (WiFi.status() == WL_NO_MODULE)
            {
                return -1;
            }
            if(enterprise) {
                _wlStatus = WiFi.beginEnterprise(SSID, USER, PASS);
            } else {
                _wlStatus = WiFi.begin(SSID, PASS);
            }
            // delay(500);

            if (_wlStatus != WL_CONNECTED)
            {
                int giveUp = 20;
                while (_wlStatus != WL_CONNECTED)
                {
                    Serial.print("WiFi connect attempt failed, trying again ");
                    Serial.print(giveUp);
                    Serial.println(" reconnect attempts left...");
                    Serial.print("WiFi status: ");
                    Serial.println(_wlStatus);
                    _wlStatus = WiFi.begin(SSID, PASS);
                    delay(1000);

                    giveUp--;
                    if (giveUp <= 0)
                    {
                        Serial.print("Giving up: setup failed");
                        return -1;
                    }
                }
            }
            return 0;
        }

        // int connectWiFiEnterprise(std::string ssid, std::string password)
        // {
        // }

        int connectClient(std::string url)
        {
            _clStatus = _client.connect(URL, LPORT);

            if (!_clStatus)
            {
                int giveUp = 20;
                while (!_clStatus)
                {
                    Serial.print("Client connect attempt failed, trying again ");
                    Serial.print(giveUp);
                    Serial.println(" reconnect attempts left...");
                    Serial.print("Client status: ");
                    Serial.println(_clStatus);
                    _clStatus = _client.connect(URL, LPORT);
                    delay(1000);

                    giveUp--;
                    if (giveUp <= 0)
                    {
                        Serial.print("Giving up: setup failed");
                        return -1;
                    }
                }
            }
            return 0;
        }

    public:
        // place within try catch block
        BpmWiFi(std::string url = URL, std::string ssid = SSID, std::string password = PASS)
        {
        }

        int initWiFi(std::string ssid = SSID, std::string pass = PASS, std::string url = URL)
        {
            _http = new HttpClient(_client, URL, LPORT);

            if (connectWiFi(ssid, pass) >= 0 && connectClient(url) >= 0)
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }

        template <typename T>
        void txWindow(std::array<T, WINDOW_LENGTH> window, std::string type)
        {
            if (WiFi.status() != WL_CONNECTED || !_client.status())
            {
                retryWiFi();
            }

            uint32_t txArr[256];
            std::copy(window.begin(), window.end(), txArr); // try using arr pointer instrad of copying
            ArduinoJson::StaticJsonDocument<256 * 16> ppgJson;
            ArduinoJson::copyArray(txArr, ppgJson.to<ArduinoJson::JsonArray>());
            // Serial.println((ppgJson.memoryUsage()/4)+800);

            _http->beginRequest();
            // _http.post(jsonReciever.c_str());
            _http->post("/api/rx/");
            _http->sendHeader("User-Agent", "Arduino/1.0");
            _http->sendHeader("Content-Length", ArduinoJson::measureJson(ppgJson));
            _http->sendHeader("Content-Type", "application/json");
            _http->connectionKeepAlive();
            _http->beginBody();

            ArduinoJson::JsonObject nested = ppgJson.createNestedObject();
            nested["type"] = type;
            ArduinoJson::serializeJson(ppgJson, *_http);
            // output JSON to serial as well - diagnostic
            ArduinoJson::serializeJson(ppgJson, Serial);

            Serial.println();
            _http->endRequest();

            ppgJson.clear();
            ppgJson.garbageCollect();
            delete &ppgJson;

            // device freezes waiting for response for some reason
            // Serial.print(_http->responseStatusCode());
            // Serial.print(" ");
            // Serial.println(_http->responseBody());
        }

        void getTest()
        {
            if (WiFi.status() != WL_CONNECTED || !_client.status())
            {
                retryWiFi();
            }
            _http->get(testEndpoint.c_str());
            // Serial.print(_http->responseStatusCode());
            // Serial.print(" ");
            // Serial.println(_http->responseBody());
        }

        void retryWiFi()
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                _client.stop();
                WiFi.end();
                initWiFi(SSID, PASS, URL);
            }
            if (!_client.status())
            {
                _client.stop();
                connectClient(URL);
            }
        }
    };
}

#endif