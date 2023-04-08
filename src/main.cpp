#include <Arduino.h>

#include "bpm.h"
#include "sutils.h"

static hf::BPM bpm = hf::BPM();

void setup() {
    if (bpm.config() < 0) {
      return;
    }

    if (WIFI_ENABLED && bpm.initWiFi() < 0) {
      return;
    }

    #if (DEBUG && VERBOSE)
    dotPrinter(8, *"*", 10);
    Serial.println("[!] Setup Complete");
    #endif

    delay(400);

    LOG_LN("BALLSBALLSBALLS");
    LOG_LN("BALLSBALLSBALLS");
    LOG_LN("BALLSBALLSBALLS");
    LOG_LN("BALLSBALLSBALLS");
    LOG_LN("BALLSBALLSBALLS");
}

void loop() {
  // delay until bluetooth (BLE) char trigger to get window, get like 10 windows at a time (delayed by a few seconds?)
  
  while(bpm.enabled()) {
    if(bpm.sampleTx() > 0)
    {
      #if (DEBUG && PRINT)
      bpm.printWindows(1);
      #endif
      // Serial.println("TX");
    }
  }
}