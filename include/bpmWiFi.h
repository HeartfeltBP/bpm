#ifndef HF_BPM_WIFI
#define HF_BPM_WIFI

#include <Arduino.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <string>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "esp_wpa2.h"

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
        HttpClient* _http;

        byte _wlStatus = WL_IDLE_STATUS;
        byte _clStatus = 0;
        std::string _postId = "INIT";
        std::string _token;
        std::string _serveIp;
        int _servePort;

        bool _init = 0;

        // txCount iterates every post transmission, wcSent iterates every SLOT_COUNT windows
        int  _txCount = 0;
        int _wcSent = 0; // window collections (a collection has a window from each slot)

        int connectWiFi(std::string ssid, std::string password, boolean enterprise = false)
        {
            if (enterprise) {
                _wlStatus = WiFi.begin(SSID, PASS);
                esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)USER, strlen(USER));
                esp_wifi_sta_wpa2_ent_set_username((uint8_t *)USER, strlen(USER));
                esp_wifi_sta_wpa2_ent_set_password((uint8_t *)PASS, strlen(PASS));
                esp_wifi_sta_wpa2_ent_enable();
            }
            else 
            {
                _wlStatus = WiFi.begin(SSID, PASS);
            }

            delay(500);

            if (_wlStatus != WL_CONNECTED)
            {
                int giveUp = 80;
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

        int connectClient()
        {
            if(!ipStatus()) {
                LOG_H_LN("[!] NOT LINKED W. LINK");
                return -1;
            }
            _clStatus = _client.connect(_serveIp.c_str(), _servePort);

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

                    _clStatus = _client.connect(_serveIp.c_str(), _servePort);

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
        BpmWiFi()
            : _server(AsyncWebServer(SERVE_PORT)) {}


        int initWiFi()
        {

            if (connectWiFi(SSID, PASS, ENTERPRISE) >= 0)
            {
                _init = 1;
                LOG_H_LN("[*] WIFI CONNECTED!");
                // LOG_H("[*] Connected to"); LOG_LN(WiFi.getHostname());
                // return getTest();
                return 0;
            }
            else
            {
                _init = 0;
                return -1;
            }
        }

        int initClient() 
        {
            if(!_init || !ipStatus()) {
                LOG_H_LN("[!] WiFi or IP not initialized, aborting Link");
                return 0;
            }

            _http = new HttpClient(_client, _serveIp.c_str(), _servePort);

            if (connectClient() >= 0)
            {
                _init = 1;
                LOG_H_LN("[*] CLIENT CONNECTED!");
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
                AsyncWebHeader* auth = request->getHeader("Authorization");
                AsyncWebHeader* info = request->getHeader("Origin"); // http://host:port

                std::string hostDelimter = "//";
                std::string colonDelimter = ":";
                

                if(auth != nullptr && auth->toString().length() > 0) 
                {
                    LOG_H_LN("got auth");
                    this->_token = auth->toString().c_str();
                    LOG_LN(this->_token.c_str());
                }
                else 
                {
                    LOG_H_LN("[!] Failed to retrieve auth token");
                }

                if(info != nullptr && info->toString().length() > 0) 
                {
                    std::string origin = info->toString().c_str();

                    std::string hostAndPort = origin.substr(origin.find("//")+2, origin.length());
                    std::string host = hostAndPort.substr(0, hostAndPort.find(":"));
                    std::string port = hostAndPort.substr(hostAndPort.find(":")+1, hostAndPort.length());
                    _servePort = stoi(port);

                    LOG_LN(hostAndPort.c_str());
                    LOG_LN(host.c_str());
                    LOG_LN(_servePort);

                    LOG_H_LN("got ip address");
                    this->_serveIp = host;
                    LOG_LN(this->_serveIp.c_str());
                }
                else
                {
                    LOG_H_LN("[!] Failed to retrieve ip address");
                }

                request->send(200);
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

        bool ipStatus()
        {
            return (_serveIp.length() > 0) ? true : false;
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
                WiFi.disconnect();
                initWiFi();
            }
            if (!_client.connected() && ipStatus())
            {
                _client.stop();
                initClient();
            }
        }
    };
}
#endif