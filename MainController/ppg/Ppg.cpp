
class Ppg {
    private:
        int ppgPin;

    public:
        // constructor
        Ppg(int ppgPin) 
        {
            this->ppgPin = ppgPin;
            pinMode(ppgPin, INPUT);
        }

        int
        getVal() 
        {
            return analogRead(this->ppgPin);
        }
};