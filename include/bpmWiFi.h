#ifndef HF_BPM_WIFI
#define HF_BPM_WIFI

#include <Arduino.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <string>
#include <ESPAsyncWebServer.h>

#include ".env.h"
#include "crets.h"

namespace hf
{

    class BpmWiFi
    {
    protected:
        // WiFiServer _server = WiFiServer(80);
        AsyncWebServer _server;
        WiFiClient _client;
        HttpClient *_http;

        byte _wlStatus = WL_IDLE_STATUS;
        byte _clStatus = 0;
        std::string _postId = "INIT";
        std::string _token;

        bool _init = 0;
        
        // txCount iterates every post transmission, wcSent iterates every SLOT_COUNT windows
        int  _txCount = 0;
        int _wcSent = 0; // window collections (a collection has a window from each slot)

        int connectWiFi(std::string ssid, std::string password, boolean enterprise = false)
        {
            if(enterprise) {
                #if (DEBUG && VERBOSE)
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
                    #if (DEBUG && VERBOSE)
                    Serial.print("[*] W{ #r:");
                    Serial.print(giveUp);
                    Serial.print(" s#:");
                    Serial.print(_wlStatus);
                    Serial.println(" }");
                    #endif
                    _wlStatus = WiFi.status();

                    giveUp--;
                    if (giveUp <= 0)
                    {
                        #if (DEBUG && VERBOSE)
                        Serial.print("[!] bpmWiFi giving up: wifi status: ");
                        Serial.println(_wlStatus);
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
                    #if (DEBUG && VERBOSE)
                    Serial.print("[*] c{ #r:");
                    Serial.print(giveUp);
                    Serial.print(" s#:");
                    Serial.print(_clStatus);
                    Serial.println(" }");
                    #endif

                    _clStatus = _client.connect(URL, LPORT);

                    giveUp--;
                    if (giveUp <= 0)
                    {
                        #if (DEBUG && VERBOSE)
                        Serial.print("[!] bpmWiFi giving up: client status: ");
                        Serial.println(_clStatus);
                        #endif
                        return -1;
                    }
                }
            }
            return 0;
        }

    public:
        // try constructing the http client
        BpmWiFi(std::string url = URL, std::string ssid = SSID, std::string password = PASS)
        :  _server(AsyncWebServer(SERVE_PORT)) {}

        int initWiFi(bool enterprise = false, std::string ssid = SSID, std::string pass = PASS, std::string url = URL)
        {
            _http = new HttpClient(_client, URL, LPORT);

            if (connectWiFi(ssid, pass, enterprise) >= 0 && connectClient(url) >= 0)
            {
                _init = 1;

                #if (DEBUG && VERBOSE)
                Serial.println("[*] WIFI CONNECTED!");
                #endif
                
                return getTest();
            }
            else
            {
                _init = 0;
                return -1;
            }
        }

        void initWebServer() 
        {
            if (WiFi.status() != WL_CONNECTED || !_client.connected())
            {
                retryWiFi();
            }
            
            // allow CORS from link
            DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
            DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "authorization");
            // handle CORS prefetch
            _server.onNotFound([](AsyncWebServerRequest *request) {
                if(request->method() == HTTP_OPTIONS) {
                    request->send(200, "text/plain", "⚙️");

                    #if VERBOSE && DEBUG
                    Serial.println("⚙️");
                    #endif
                }
                return;
            });

            _server.on("/", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
                request->send(200, "text/plain", "⚙️");
                #if VERBOSE && DEBUG
                Serial.println("⚙️");
                #endif
            });

            _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
                request->send(200, "text/plain", "☑️ Bungo");
                #if VERBOSE && DEBUG
                Serial.print("Bungo"); Serial.println("☑️");
                #endif
            });

            _server.on("/", HTTP_POST, [this](AsyncWebServerRequest *request) {
                AsyncWebHeader *header = request->getHeader("Authorization");
                this->_token = header->toString().c_str();

                #if VERBOSE && DEBUG
                Serial.println(this->_token.c_str());
                #endif
            });

            #if (DEBUG && VERBOSE)
            Serial.print("SERVER RUNNING ON IP: "); Serial.print(WiFi.localIP()); Serial.print(":"); Serial.println(SERVE_PORT);
            #endif
            
            _server.begin();
        }

        void endWebServer() {
            _server.end();
        }

        bool identityStatus()
        {
            return (_token.length() > 0) ? true : false;
        }

        const char *getIdentityToken() 
        {
            return _token.c_str();
        }

        template <typename T, std::size_t n>
        int txWindow(T (&frame)[n], int type)
        {
            #if VERBOSE && DEBUG
            Serial.println("tx...");
            #endif

            if (WiFi.status() != WL_CONNECTED || !_client.connected())
            {
                retryWiFi();
            }

            #if VERBOSE && DEBUG
            Serial.print(this->_token.c_str()); Serial.println("<FOR USE TO TX>");
            delay(10000);
            #endif
            if(_token.length() == 0) {
                return ERROR;
            }

            std::string postData;
            // paramaterize
            // retreive from csv array index 0 = # of samples, 1 = sampling rate, 2 = type
            
            // Post Info: 0: num-samples; 1: sampling-rate; 2: metric-type; 3: collections-sent; 4: transmission-count; 5: post-id 
            postData.append("4100,200,"); // 0, 1
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
            } // 2

            postData.append(std::to_string(_wcSent) + ","); // 3
            postData.append(std::to_string(_txCount) + ","); // 4
            postData.append(_postId + ","); // 5
            postData.append(_token + ",");

            for(int i = 0; i < FRAME_LENGTH; i++) {
                postData.append(std::to_string(frame[i]));
                if(i != FRAME_LENGTH-1) postData.append(",");
            }

            _http->beginRequest();
            _http->post(RX_ENDPOINT);
            _http->sendHeader("User-Agent", "HF-BPM/0.1");
            _http->sendHeader("Content-Length", postData.length());
            _http->sendHeader("Content-Type", "text/csv");
            _http->connectionKeepAlive();
            _http->beginBody();

            // Serial.println(postData.c_str());
            _http->print(postData.c_str());
            _http->endRequest();

            postData.clear();
            
            _txCount++;

            if(_txCount % 3 == 0)
            {
                _wcSent++;
            }
            
            int statusCode = _http->responseStatusCode();
            _http->responseBody();
            // _postId = _http->responseBody().c_str();

            // CREATE LOGGER CLASS
            #if (DEBUG && VERBOSE)
            Serial.print(statusCode);
            Serial.println(_postId.c_str());
            #endif

            if(statusCode != 200 || statusCode != 100){
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
            _http->get(TEST_ENDPOINT);

            int statusCode = _http->responseStatusCode();
            #if (DEBUG && VERBOSE)
            Serial.print(statusCode);
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
            // if(!_init) initWiFi();
            return (_wlStatus == WL_CONNECTED);
        }

        bool isClientConnected() {
            return (_clStatus == 1);
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
    };
}
#endif