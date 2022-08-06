
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
    Led(int ledPin = 5)
        : ledPin (ledPin)
    {
        if (diagnosticMode)
        {
            pinMode(LED_BUILTIN, OUTPUT);
        }
        else
        {
            pinMode(this->ledPin, OUTPUT);
        }
        // printf("%d\n", led);
    }
    // TODO: add overloaded constructor for
    // multiple LEDs and accompanying logic

    void
    setDiagnostic(bool set) { this->diagnosticMode = set; }

    int
    setVal(int ledVoltage)
    {
        if (!this->ledPin)
        {
            perror("ERROR: led pin not initialized");
            return -1;
        }
        analogWrite(this->ledPin, ledVoltage);
        return 0;
    }

    int
    setPin(int pin) { this->ledPin = pin; }

    int
    getPin() { return this->ledPin; }
};