#include <Arduino.h>
#include "bpmSensor.h"
#include "bpmWifi.h"

static ppgInt ppgArr0[WINDOW_LENGTH];
static ppgInt ppgArr1[WINDOW_LENGTH];
static ecgInt  ecgArr[WINDOW_LENGTH];


static hf::WindowHandler windowHandler(ppgArr0, ppgArr1, ecgArr);
static hf::MaxReg maxReg;
static hf::MaxFifo maxFifo(&maxReg, &windowHandler);
static hf::BpmSensor bpmSensor(&windowHandler);
static hf::BpmWiFi bpmWifi(URL, SSID, PASS);

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);
    // Wire.setClock(100000);
    delay(10);


    // loop until serial connection opens - diagnostic
    while (!Serial)
        delay(1);

    bpmSensor.init();
    if(bpmWifi.initWiFi() == -1) Serial.println("ERROR"); return;
    delay(1000);
    Serial.println("Setup Complete");
    delay(10000);
}

void loop() {
  if(bpmSensor.sample()) {
    // bpmWifi.txWindow();
    for(int i = 0; i < WINDOW_LENGTH; i++) 
    {
      Serial.print(ppgArr0[i]); Serial.print(",");
      Serial.print(ppgArr1[i]); Serial.print(",");
      Serial.print(ecgArr[i]);  Serial.println("");
      delay(10);
    }
  } else {
    delay(100);
  }
  // bpmWifi.getTest();
  
}