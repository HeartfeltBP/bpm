
/**
 * Driver for LEDs connected to analog pins on Arduino (A0 -> A7)
 * 
 */
class Led 
{
    private:
        int ledPin;
        // uses onboard LED for testing
        bool diagnosticMode = false;

    public:
        // constructor
        Led(int ledPin) 
        {
            // support for only single LED for now
            if (diagnosticMode) {
                pinMode(LED_BUILTIN, OUTPUT);
            } else {
                this->ledPin = ledPin;
                pinMode(this->ledPin, OUTPUT);
            }
            // printf("%d\n", led);
        }
        // TODO: add overloaded constructor for
        // multiple LEDs and accompanying logic

        void
        setDiagnostic(bool set)
        {
            this->diagnosticMode = set;
        }

        int
        setVal(int ledVoltage) 
        {
            if(!this->ledPin) {
                perror("ERROR: led pin not set");
                return -1;
            }
            analogWrite(this->ledPin, ledVoltage);
            if(analogRead(this->ledPin) != ledVoltage) {
                perror("ERROR: LED voltage was not set");
                return -1;
            }
            return 0;
        }

        int 
        getVal()
        {
            analogRead(this->ledPin);
        }
};