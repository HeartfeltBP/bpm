//TODO: make headerfile for these
#include "Led.cpp"
#include "Ppg.cpp"


class PpgController 
{
    private:
        int ppgPin, ledPin,
            samplingRate;
        bool diagnosticMode = false;
        Led LED;
        Ppg PPG;

    public:
        // constructor
        PpgController(int ppgPin, int ledPin, int samplingRate) 
        {
            this->ppgPin         = ppgPin;
            this->ledPin         = ledPin;
            this->samplingRate   = samplingRate;

            this->PPG = Ppg(this->ppgPin);
            this->LED = Led(this->ledPin);
        }

        int
        getPpgVal() 
        {
            this->PPG.getVal();
        }

        int
        getLedVal()
        {
            this->LED.getVal();
        }

        void
        setDiagnostic(bool set)
        {
            this->diagnosticMode = set;
            this->LED.setDiagnostic(set);
        }

        void
        setLedVal(int val)
        {
            this->LED.setVal(val);
        }
        
        int
        samplePpg()
        {
            this->setDiagnostic(true);

            int val = 32;
            // test value, doesn't actually work
            // to get sampling rate
            delay(this->samplingRate);

            // toggle the LED between small and big value
            val = (val == 32) ? 255 : 32;
            this->LED.setVal(val);

            return this->PPG.getVal();
        }

};