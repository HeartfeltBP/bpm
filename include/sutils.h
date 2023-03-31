
#ifndef HF_SERIAL_UTILS
#define HF_SERIAL_UTILS

#include <Arduino.h>

void dotPrinter(int dotCount, char symbol, int printDelay) 
{
    for(int i = 0; i < dotCount; i++) {
        Serial.print(symbol);
        delay(printDelay);
    }
    Serial.println("");
}

#endif