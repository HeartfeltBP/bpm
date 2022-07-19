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

            this->PPG = Ppg(ppgPin);
            this->LED = Led(ledPin);
        }

        int
        getPpgPin() 
        {
            return this->PPG.getPin();
        }

        int
        getLedPin() { return this->LED.getPin(); }

        void
        setDiagnostic(bool set)
        {
            this->diagnosticMode = set;
            this->LED.setDiagnostic(set);
        }

        void
        setLedVal(int val) { this->LED.setVal(val); }
        
        int
        samplePpg()
        {
            this->setDiagnostic(true);

            int val = HIGH;
            
            // toggle the LED between small and big value
            val = (val == HIGH) ? LOW : HIGH;
            this->LED.setVal(val);
            pinMode(LED_BUILTIN, INPUT);
            digitalWrite(LED_BUILTIN, val);

            return this->PPG.getVal();
        }

};