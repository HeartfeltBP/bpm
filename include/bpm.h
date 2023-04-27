#ifndef HF_BPM
#define HF_BPM

#include <Wire.h>
#include <Arduino.h>

#include "constants.h"
#include "crets.h"

#include "bpmSensor.h"
#include "bpmWiFi.h"
#include ".env.h"

namespace hf
{

    class BPM
    {
    protected:
        struct opFlags
        {
            bool enabled = 1;
            bool txReady = 0;
            bool configured = 0;
            bool wiFiInit = 0;
            bool linkInit = 0;
            bool sensorInit = 0;
            bool i2cInit = 0;
            bool serialInit = 0;
            bool proximity = 0;
            bool identity = 0;
            bool dataFull = 0;
            bool error = 0;
        } _opFlags;

        ppgInt _ppgArr0[FRAME_LENGTH];
        ppgInt _ppgArr1[FRAME_LENGTH];
        ecgInt _ecgArr0[FRAME_LENGTH];

        WindowHandler _windowHandler;
        MaxReg _maxReg;
        MaxFifo _maxFifo;
        BpmSensor _bpmSensor;
        BpmWiFi _bpmWiFi;
        // BpmBle _bpmBle;

        // const char* deviceId;

    public:
        BPM(): _maxReg(MaxReg()),
            _windowHandler(WindowHandler(_ppgArr0, _ppgArr1, _ecgArr0)),
            _maxFifo(MaxFifo(&_maxReg, &_windowHandler)),
            _bpmSensor(BpmSensor(&_maxFifo, &_windowHandler)),
            _bpmWiFi(BpmWiFi(URL, SSID, PASS))
        {
            _opFlags.enabled = 1;
        }

        bool enabled()
        {
            return _opFlags.enabled;
        }

        int serialInit()
        {
            Serial.begin(115200);
            _opFlags.serialInit = 1;

            return 0;
        }

        // debug
        void waitUntilSerial()
        {
            while (!Serial)
                delay(1);
        }

        int serialInitWait()
        {
            serialInit();
            waitUntilSerial();

            return 0;
        }

        int i2cInit()
        {
            Wire.begin();
            Wire.setClock(400000);
            _opFlags.i2cInit = 1;

            return 0;
        }

        int sensorInit()
        {
            _bpmSensor.init();
            _opFlags.sensorInit = 1;

            return 0;
        }

        int initWiFi()
        {
            if (!WIFI_ENABLED) return 0;

            _bpmWiFi.initWiFi();

            if (_bpmWiFi.isWiFiConnected())
            {
                _opFlags.wiFiInit = 1;
            }
            else
            {
                int giveUp = 20;

                _opFlags.wiFiInit = 0;

                LOG_H_LN("[!] WiFi not connected: ");

                while (!_bpmWiFi.isWiFiConnected() || !giveUp > 0) {
                    LOG_H("|W|");
                    delay(600);
                    _bpmWiFi.initWiFi();
                    giveUp--;
                }


                if (!_bpmWiFi.isWiFiConnected()) {
                    LOG_H_LN("[!] BPM WiFi connection attempts failed");
                    LOG_H_LN("[*] Starting data collection without WiFi - stops after single frame");
                    return -1;
                }
                else
                {
                    _opFlags.wiFiInit = 1;
                }
            }

            return 0;
        }

        /**
         * Connects to link client
        */
        int initLink()
        {
            if (!WIFI_ENABLED) return 0;

            int giveUp = 20;

            if (!_opFlags.wiFiInit) {
                return -1;
            }

            _bpmWiFi.initWebServer();

            int giveUp = 1000;

            while (!_bpmWiFi.ipStatus() && giveUp > 0) {
                giveUp % 100 == 0 ? Serial.print(giveUp) : Serial.print(".");
                delay(200);
                giveUp--;
            }

            if (!_bpmWiFi.isWiFiConnected()) {
                LOG_H_LN("[!] Failed to recieve ip address for transmission.");
                return -1;
            }
            else
            {
                _opFlags.wiFiInit = 1;
            }

            _bpmWiFi.initClient();

            if (_bpmWiFi.isClientConnected())
            {
                _opFlags.linkInit = 1;
            }
            else
            {
                while (!_bpmWiFi.isClientConnected() || !giveUp > 0) {
                    LOG_H("|c|");
                    delay(800);
                    _bpmWiFi.initClient();
                    giveUp--;
                }

                if (!_bpmWiFi.isWiFiConnected()) {
                    LOG_H_LN("[!] BPM WiFi connection attempts failed");
                    LOG_H_LN("[*] Starting data collection without WiFi - stops after single frame");
                    return -1;
                }
                else
                {
                    _opFlags.wiFiInit = 1;
                }

            }
            return 0;

        }

        void invalidateIdentity() {
            _opFlags.identity = 0;
        }

        int initPairing() {
            if (!_opFlags.wiFiInit || _opFlags.identity) return -1;

            _bpmWiFi.initWebServer();

            int giveUp = 20000;

            while (!_bpmWiFi.identityStatus() && giveUp > 0) {
                giveUp % 100 == 0 ? Serial.print(giveUp) : Serial.print(".");
                delay(100);

                giveUp--;
            }

            _bpmWiFi.endWebServer();

            if (_bpmWiFi.identityStatus()) {
                LOG_H_LN("Wowee");
                _opFlags.identity = 1;
            }
            else {
                return ERROR;
            }

            return 0;
        }

        int config()
        {
            // add identity config (bool identity), run func if true

            if (_opFlags.configured)
            {
                return 2;
            }

            if (!_opFlags.serialInit && DEBUG)
            {
                serialInitWait();
            }

            if (!_opFlags.i2cInit)
            {
                i2cInit();
                delay(100);
            }

            if (!_opFlags.sensorInit)
            {
                sensorInit();
                delay(100);
            }

            if (WIFI_ENABLED && !_opFlags.wiFiInit)
            {
                initWiFi();
                delay(100);
            }

            if (_opFlags.wiFiInit && !_opFlags.linkInit)
            {

                // review
                initLink();
            }

            if (_opFlags.wiFiInit && !_opFlags.identity)
            {
                initPairing();
            }

            _opFlags.configured = _opFlags.i2cInit && _opFlags.sensorInit;

            return 0;
        }

        int txWindows(bool debug = false)
        {
            if (!_opFlags.wiFiInit) {
                return -1;
            }

            _bpmWiFi.getTest();
            #if (SLOT_COUNT >= 1)
            _bpmWiFi.txWindow(_ppgArr0, PPG_SLOT0);
            delay(100);
            #endif
            #if (SLOT_COUNT >= 2)
            _bpmWiFi.txWindow(_ppgArr1, PPG_SLOT1);
            delay(100);
            #endif
            #if (SLOT_COUNT >= 3)
            _bpmWiFi.txWindow(_ecgArr0, ECG_SLOT0);
            delay(100);
            #endif

            return 0;
        }

        int sampleTx()
        {
            if (!_opFlags.enabled)
            {
                LOG_H_LN("[*] Not enabled!");
                return 0;
            }

            if (!_opFlags.sensorInit || !_opFlags.configured || _opFlags.error)
            {
                LOG_H_LN("[!] Cannot sample, not configured or error has occured");
                return -1;
            }

            if (_windowHandler.dataFull())
            {

                LOG_LN("[!] DATA FULL");
                LOG_H("0:"); LOG(_opFlags.wiFiInit); LOG(_opFlags.configured); LOG_LN(_opFlags.identity);
                DBG(delay(1000));

                // wifi init again?
                return 0;
            }

            // _bpmSensor.checkInterrupts();

            if (_bpmSensor.sample() > 0)
            {
                _opFlags.dataFull = 1;
                if (_opFlags.wiFiInit && _opFlags.configured && _opFlags.identity && !_opFlags.error)
                {
                    LOG_H_LN("[*] Ready to transmit...");
                    _opFlags.txReady = 1;
                }
            }

            if (_opFlags.txReady && _opFlags.wiFiInit && _opFlags.identity)
            {
                LOG_H("0:");
                LOG(_opFlags.txReady);
                LOG(_opFlags.configured);
                LOG_LN(_opFlags.identity);

                if (txWindows() >= 0) {
                    _opFlags.txReady = 0;

                    if (_bpmSensor.restartSampling() < 0) {
                        return -1;
                    }

                    _opFlags.dataFull = 0;

                    return 1;
                }
                else {
                    return -1;
                }
            }
            return 0;
        }

        void logDeviceInfo()
        {
            #if (!PRINT || !DEBUG || !VERBOSE)
            return;
            #endif

            LOG_H("[♥] Heartfelt hfBPM Firmware v"); LOG_H_LN(FIRMWARE_VERSION);

        }

        void printWindows(int printDelay = 10)
        {
            // wtf
            #if (!PRINT || !DEBUG || !VERBOSE)
            return;
            #endif

            LOG_H_LN("BEGIN");
            #if (SLOT_COUNT >= 1)
            LOG_H_LN("[>] PRINTING WINDOWS...");
            for (int i = 0; i < FRAME_LENGTH; i++) {
                LOG(_ppgArr0[i]);
                if (SLOT_COUNT == 1) Serial.println();
                #if (SLOT_COUNT >= 2)
                LOG_H(","); LOG(_ppgArr1[i]);
                #endif
                #if (SLOT_COUNT >= 3)
                LOG_H(","); LOG_LN(_ecgArr0[i]);
                #endif
                delay(printDelay);
            }
            LOG_H_LN("END");
            #endif
        }
    };
}

#endif // HF_BPM_MAX