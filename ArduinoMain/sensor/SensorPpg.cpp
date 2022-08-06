#include "component.hpp"

class SensorPpg
{
private:
    int ppgPin, ledPin,
        samplingRate, T;
    bool diagnosticMode = false;
    component::Led LED;
    component::Ppg PPG;

public:
    // constructor
    SensorPpg(int ppgPin, int ledPin, int samplingRate = 200)
        : ppgPin(ppgPin), ledPin(ledPin), samplingRate(samplingRate)
    {
        this->T   = ( 1 / samplingRate );
        this->PPG = component::Ppg(ppgPin);
        this->LED = component::Led(ledPin);
    }

    int
    getPpgPin() { return this->PPG.getPin(); }

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
        return this->PPG.getVal();
    }
};