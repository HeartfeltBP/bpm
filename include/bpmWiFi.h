#include <Arduino.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <string>

#include ".env.h"

namespace hf
{
    static const std::string apiEndpoints[2] = {
        "/api/rx",
        "/api/test"
    };

    static const std::string jsonReciever = apiEndpoints[0];
    static const std::string testEndpoint = apiEndpoints[1];

    class BpmWiFi
    {
    protected:
        WiFiClient _client;
        HttpClient *_http;

        byte _wlStatus = WL_IDLE_STATUS;
        byte _clStatus = 0;

        bool _init = 0;

        int connectWiFi(std::string ssid, std::string password, boolean enterprise = false)
        {
            if(enterprise) {
                #if (DEBUG)
                Serial.println("NOT IMPLEMENTED");
                #endif
                // _wlStatus = WiFi.begin(SSID, WPA2_AUTH_PEAP, PASS);
                return -1;
            } else {
                _wlStatus = WiFi.begin(SSID, PASS);
            }
            delay(500);
            if (_wlStatus != WL_CONNECTED)
            {
                int giveUp = 20;
                while (_wlStatus != WL_CONNECTED)
                {
                    #if (DEBUG)
                    Serial.print("WiFi connect attempt failed, trying again ");
                    Serial.print(giveUp);
                    Serial.println(" reconnect attempts left...");
                    Serial.print("WiFi status: ");
                    Serial.println(_wlStatus);
                    #endif
                    _wlStatus = WiFi.status();

                    giveUp--;
                    if (giveUp <= 0)
                    {
                        #if (DEBUG)
                        Serial.println("Giving up: setup failed");
                        #endif
                        return -1;
                    }
                    delay(500);
                }
            }
            return 0;
        }

        int connectClient(std::string url)
        {
            _clStatus = _client.connect(URL, LPORT);

            if (!_clStatus)
            {
                int giveUp = 20;
                while (!_clStatus)
                {
                    #if (DEBUG)
                    Serial.print("Client connect attempt failed, trying again ");
                    Serial.print(giveUp);
                    Serial.println(" reconnect attempts left...");
                    Serial.print("Client status: ");
                    Serial.println(_clStatus);
                    #endif
                    _clStatus = _client.connect(URL, LPORT);

                    giveUp--;
                    if (giveUp <= 0)
                    {
                        #if (DEBUG)
                        Serial.println("Giving up: setup failed");
                        #endif
                        return -1;
                    }
                }
            }
            return 0;
        }

    public:
        // place within try catch block
        BpmWiFi(std::string url = URL, std::string ssid = SSID, std::string password = PASS)
        {}

        int initWiFi(bool enterprise = false, std::string ssid = SSID, std::string pass = PASS, std::string url = URL)
        {
            _http = new HttpClient(_client, URL, LPORT);

            if (connectWiFi(ssid, pass, enterprise) >= 0 && connectClient(url) >= 0)
            {
                _init = 1;

                #if (DEBUG)
                Serial.println("WIFI CONNECTED");
                #endif
                
                return getTest();
            }
            else
            {
                _init = 0;
                return -1;
            }
        }

        template <typename T>
        int txWindow(T frame[], int type)
        {
            if (WiFi.status() != WL_CONNECTED || !_client.connected())
            {
                retryWiFi();
            }

            std::string postData;
            // paramaterize
            // retreive from csv array index 0 = # of samples, 1 = sampling rate, 2 = type
            postData.append("4100,200,");
            switch(type) {
                case PPG_SLOT0:
                    postData.append("PPG0,");
                    break;
                case PPG_SLOT1:
                    postData.append("PPG1,");
                    break;
                case ECG_SLOT0:
                    postData.append("ECG0,");
                    break;
            }


            // prob best to calculate data structure length but for some reason
            // sizeof(frame) / sizeof(T) do not work
            for(int i = 0; i < WINDOW_LENGTH; i++) {
                postData.append(std::to_string(frame[i]));
                if(i != WINDOW_LENGTH-1) postData.append(",");
            }

            _http->beginRequest();
            _http->post(jsonReciever.c_str());
            _http->sendHeader("User-Agent", "HF-BPM/0.1");
            _http->sendHeader("Content-Length", postData.length());
            _http->sendHeader("Content-Type", "text/csv");
            _http->connectionKeepAlive();
            _http->beginBody();

            // Serial.println(postData.c_str());
            _http->print(postData.c_str());
            _http->endRequest();

            postData.clear();
            
            int statusCode = _http->responseStatusCode();
            #if (DEBUG)
            Serial.print(statusCode));
            Serial.println(_http->responseBody());
            #else
            _http->responseBody();
            #endif

            if(statusCode != 200){
                return ERROR;
            };

            delay(10);

            return 0;
        }

        int getTest()
        {
            if (WiFi.status() != WL_CONNECTED || !_client.connected())
            {
                retryWiFi();
            }
            _http->get(testEndpoint.c_str());

            int statusCode = _http->responseStatusCode();
            #if (DEBUG)
            Serial.print(statusCode));
            Serial.println(_http->responseBody());
            #else
            _http->responseBody();
            #endif

            if(statusCode != 200){
                return ERROR;
            };

            delay(10);

            return 0;
        }

        bool isWiFiConnected() {
            if(!_init) initWiFi();
            return (_wlStatus == WL_CONNECTED) ? true : false;
        }

        bool isClientConnected() {
            return (_wlStatus == WL_CONNECTED) ? true : false;
        }

        void retryWiFi()
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                _client.stop();
                WiFi.disconnect();
                initWiFi(SSID, PASS, URL);
            }
            if (!_client.connected())
            {
                _client.stop();
                connectClient(URL);
            }
        }

        // TODO: IMPLEMENT IDENTITY CONFIRMATION
        // int sendDeviceId()
        // {

        // }

        // int identityConfig()
        // {

        // }
    };
}