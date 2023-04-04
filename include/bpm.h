#ifndef HF_BPM
#define HF_BPM

#include <Wire.h>
#include <Arduino.h>

#include "constants.h"
#include "config.h"
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
            _bpmWiFi.initWiFi();
            if (_bpmWiFi.isWiFiConnected() && _bpmWiFi.isClientConnected())
            {
                _opFlags.wiFiInit = 1;
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
            }

            _opFlags.configured = _opFlags.i2cInit && _opFlags.sensorInit;

            return 0;
        }

        int txWindows(bool debug = false)
        {
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
            if (!_opFlags.sensorInit || !_opFlags.configured || _opFlags.error)
            {
                return -1;
            }
            if (_windowHandler.dataFull())
            {
                return 0;
            }

            if (_bpmSensor.sample() > 0)
            {
                _opFlags.dataFull = 1;
                if (_opFlags.wiFiInit && _opFlags.configured && !_opFlags.error)
                {
                    _opFlags.txReady = 1;
                }
            }

            if (_opFlags.txReady && _opFlags.wiFiInit)
            {
                txWindows();
                _opFlags.txReady = 0;

                if (!_bpmSensor.restartSampling() > 0) {
                    return -1;
                }

                _opFlags.dataFull = 0;

                return 1;
            }
            return 0;
        }

        void printWindows(int printDelay = 10)
        {
            #if (!PRINT || !DEBUG)
            return;
            #endif

            Serial.println("BEGIN");
            #if (SLOT_COUNT >= 1)
            #if(VERBOSE)
            Serial.println("[>] PRINTING WINDOWS...");
            #endif
            for (int i = 0; i < FRAME_LENGTH; i++) {
                Serial.print(_ppgArr0[i]);
                if (SLOT_COUNT == 1) Serial.println();
                #if (SLOT_COUNT >= 2)
                Serial.print(",");
                Serial.print(_ppgArr1[i]);
                #endif
                #if (SLOT_COUNT >= 3)
                Serial.print(",");
                Serial.println(_ecgArr0[i]);
                #endif
                delay(printDelay);
            }
            Serial.println("END");
            #endif
        }
    };
}

#endif // HF_BPM_MAX