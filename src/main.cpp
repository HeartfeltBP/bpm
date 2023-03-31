#include <Arduino.h>
#include "bpmSensor.h"
#include "sutils.h"

static hf::BPM bpm = hf::BPM();

void setup() {
    if (bpm.config(true) < 0) {
      return;
    }

    if (WIFI_ENABLED && bpm.initWiFi() < 0) {
      return;
    }

    #if (DEBUG)
    dotPrinter(8, *"*", 500);
    Serial.println("[!] Setup Complete");
    #endif

    delay(1000);
}

void loop() {
  // delay until bluetooth (BLE) char trigger to get window, get like 10 windows at a time (delayed by a few seconds?)
  while(bpm.enabled()) {
    if(bpm.sampleTx() > 0)
    {
      #if (DEBUG)
      bpm.printWindows();
      #endif
      // Serial.println("TX");
    }
  }
}