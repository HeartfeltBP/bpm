
class Ppg {
    private:
        int ppgPin;

    public:
        // constructor with default PPG pin A0
        Ppg(int ppgPin = A0) 
        {
            this->ppgPin = ppgPin;
            pinMode(ppgPin, INPUT);
        }

        int
        setPin(int pin)
        {
            this->ppgPin = pin;
        }

        int
        getVal() 
        {
            return analogRead(this->ppgPin);
        }
};