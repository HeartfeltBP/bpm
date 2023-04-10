#ifndef HF_BPM_WIFI
#define HF_BPM_WIFI

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HttpClient.h>
#include <string>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include ".env.h"
#include "crets.h"
#include "constants.h"

namespace hf
{

    class BpmWiFi
    {
    protected:
        // WiFiServer _server = WiFiServer(80);
        AsyncWebServer _server;
        WiFiClientSecure _client;
        HttpClient* _http;
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
            if (enterprise) {
                LOG_H_LN("NOT_IMPLEMENTED");
                // _wlStatus = WiFi.begin(SSID, WPA2_AUTH_PEAP, PASS);
                return -1;
            }
            else 
            {
                _wlStatus = WiFi.begin(SSID, PASS);
            }

            delay(500);

            if (_wlStatus != WL_CONNECTED)
            {
                int giveUp = 20;
                while (_wlStatus != WL_CONNECTED)
                {
                    #if (DEBUG && VERBOSE)
                    LOG_H("[*] W{ #r:");
                    LOG(giveUp);
                    LOG_H(" s#:");
                    LOG(_wlStatus);
                    LOG_H_LN(" }");
                    #endif
                    _wlStatus = WiFi.status();

                    giveUp--;
                    if (giveUp <= 0)
                    {
                        #if (DEBUG && VERBOSE)
                        LOG_H("[!] bpmWiFi giving up: wifi status: ");
                        LOG_LN(_wlStatus);
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
                    DBG(delay(10000));
                    LOG_H("[*] c{ #r:");
                    LOG(giveUp);
                    LOG_H(" s#:");
                    LOG(_clStatus);
                    LOG_H_LN(" }");

                    _clStatus = _client.connect(URL, LPORT);

                    giveUp--;
                    if (giveUp <= 0)
                    {
                        LOG_H("[!] bpmWiFi giving up: client status: ");
                        LOG_LN(_clStatus);
                        return -1;
                    }
                }
            }
            return 0;
        }

    public:
        // try constructing the http client
        BpmWiFi(std::string url = URL, std::string ssid = SSID, std::string password = PASS)
            : _server(AsyncWebServer(SERVE_PORT)) {}

        int initWiFi(bool enterprise = false, std::string ssid = SSID, std::string pass = PASS, std::string url = URL)
        {
            _client.setCACert(CERTIFICATE_WEBAPP);
            _http = new HttpClient(_client, URL, LPORT);

            if (connectWiFi(ssid, pass, enterprise) >= 0 && connectClient(url) >= 0)
            {
                _init = 1;
                LOG_H_LN("[*] WIFI CONNECTED!");
                LOG_H("[*] Connected to"); LOG_LN(WiFi.getHostname());
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
            _server.onNotFound([](AsyncWebServerRequest* request) {
                if (request->method() == HTTP_OPTIONS) {
                    request->send(200, HTTP_PLAINTEXT, F("⚙️"));
                    LOG_H_LN("⚙️");
                }
                return;
                });

            // tell external server we can handle CORS request
            _server.on("/", HTTP_OPTIONS, [](AsyncWebServerRequest* request) {
                request->send(200, HTTP_PLAINTEXT, F("⚙️"));
                LOG_H_LN("⚙️🎫");
                });

            // recieve get request when pairing is initiated from link
            _server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
                request->send(200, HTTP_PLAINTEXT, DEVICE_INFO);
                LOG_H_LN("☑️📋");
                });

            _server.on("/", HTTP_POST, [this](AsyncWebServerRequest* request) {
                AsyncWebHeader* header = request->getHeader("Authorization");
                this->_token = header->toString().c_str();
                LOG_LN(this->_token.c_str());
                });

            LOG_H("SERVER RUNNING ON IP: "); 
            LOG(WiFi.localIP()); 
            LOG_H(":"); 
            LOG_LN(SERVE_PORT);

            _server.begin();
        }

        void endWebServer() {
            _server.end();
        }

        bool identityStatus()
        {
            return (_token.length() > 0) ? true : false;
        }

        const char* getIdentityToken()
        {
            return _token.c_str();
        }

        template <typename T, std::size_t n>
        int txWindow(T(&frame)[n], int type)
        {
            LOG_H_LN("[*] Preparing to TX...");
            if (WiFi.status() != WL_CONNECTED || !_client.connected())
            {
                retryWiFi();
            }

            LOG_H("<FOR USE TO TX> <> BEGIN<");
            LOG(this->_token.c_str());
            LOG_H_LN(">END");
            DBG(delay(10000));

            if (_token.length() == 0) {
                return ERROR;
            }

            std::string postData;

            // retreive from csv array index 0 = # of samples, 1 = sampling rate, 2 = type
            // Post Info: 0: num-samples; 1: sampling-rate; 2: metric-type; 3: collections-sent; 4: transmission-count; 5: post-id 
            postData.append("4100,200,"); // 0, 1
            switch (type) {
            case PPG_SLOT0:
                postData.append("PPG0,");
                break;
            case PPG_SLOT1:
                postData.append("PPG1,");
                break;
            case ECG_SLOT0:
                postData.append("ECG0,");
                break;
            default:
                LOG_H_LN("[!] no such slot type exists");
            } // 2

            postData.append(std::to_string(_wcSent) + ","); // 3
            postData.append(std::to_string(_txCount) + ","); // 4
            postData.append(_postId + ","); // 5
            postData.append(_token + ",");
            for (int i = 0; i < FRAME_LENGTH; i++) {
                postData.append(std::to_string(frame[i]));
                if (i != FRAME_LENGTH - 1) postData.append(",");
            }

            _http->beginRequest();
            _http->post(RX_ENDPOINT);
            _http->sendHeader("User-Agent", "HF-BPM/0.1");
            _http->sendHeader("Content-Length", postData.length());
            _http->sendHeader("Content-Type", "text/csv");
            _http->connectionKeepAlive();
            _http->beginBody();

            // LOG_LN(postData.c_str());
            _http->print(postData.c_str());
            _http->endRequest();
            postData.clear();

            _txCount++;
            if (_txCount % 3 == 0)
                _wcSent++;

            int statusCode = _http->responseStatusCode();
            const char *resBody = _http->responseBody().c_str();
            
            if(statusCode == 201) {
                _postId = resBody;
            } else {
                _postId = "INIT";
            }
        

            LOG(statusCode);
            LOG(resBody);
            LOG_LN(_postId.c_str());
            
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
            const char *resBody = _http->responseBody().c_str();

            LOG(statusCode);
            LOG_LN(resBody); 

            _http->responseBody();


            if (statusCode != 200) {
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