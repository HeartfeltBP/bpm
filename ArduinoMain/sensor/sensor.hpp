#ifndef COMPONENT
#include "../component/component.hpp"
#endif

#ifndef SENSOR
#define SENSOR

namespace sensor {
    class SensorPpg {
        private:
            int ppgPin, ledPin,
            samplingRate, T;
            bool diagnosticMode = false;
            component::Led LED;
            component::Ppg PPG;
            
        public:
            SensorPpg(int ppgPin, int ledPin, int samplingRate)
                : ppgPin(ppgPin), ledPin(ledPin), samplingRate(samplingRate)
            {}

            int getPpgPin();

            int getLedPin();

            void setDiagnostic(bool set);

            void setLedVal(int val);

            int samplePpg();
    };
}

#endif