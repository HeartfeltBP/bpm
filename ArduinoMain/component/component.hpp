#ifndef COMPONENT_H
#define COMPONENT_H

namespace component
{

    class Ppg
    {
    private:
        int ppgPin;
        bool diagnosticMode;

    public:
        Ppg(int ppgPin = 6)
            : ppgPin(ppgPin)
        {}

        int setPin(int pin);

        int getPin();

        int getVal();
    };

    class Led
    {
    private:
        int ledPin;

    public:
        Led(int ledPin = 5)
            : ledPin(ledPin)
        {}

        void setDiagnostic(bool set);

        int setVal(int ledVoltage);

        int setPin(int pin);

        int getPin();
    };
}

#endif