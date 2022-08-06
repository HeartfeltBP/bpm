
class Ppg
{
private:
    int ppgPin;

public:
    // constructor
    Ppg(int ppgPin = 6)
        : ppgPin(ppgPin)
    {
        pinMode(ppgPin, INPUT);
    }

    int
    setPin(int pin) { this->ppgPin = pin; }

    int
    getPin() { return this->ppgPin; }

    int
    getVal() { return analogRead(this->ppgPin); }
};