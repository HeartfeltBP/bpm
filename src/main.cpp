#include <Arduino.h>
#include "bpmSensor.h"
#include "bpmWifi.h"

static ppgInt ppgArr0[WINDOW_LENGTH];
static ppgInt ppgArr1[WINDOW_LENGTH];
static ecgInt  ecgArr[WINDOW_LENGTH];


static hf::WindowHandler windowHandler(ppgArr0, ppgArr1, ecgArr);
static hf::MaxReg maxReg;
static hf::MaxFifo maxFifo(&maxReg, &windowHandler);
static hf::BpmSensor bpmSensor(&maxFifo, &windowHandler);
static hf::BpmWiFi bpmWifi(URL, SSID, PASS);

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);
    delay(10);
    // loop until serial connection opens - diagnostic
    while (!Serial)
        delay(1);
    delay(1000);
    bpmSensor.init();
    // if(bpmWifi.initWiFi() == -1) Serial.println("ERROR"); return;
    bpmWifi.initWiFi();
    delay(10000);
    Serial.println("Setup Complete");
    delay(1000);
}

void loop() {
  // delay until bluetooth (BLE) char trigger to get window, get like 10 windows at a time (delayed by a few seconds?)
  int sampleRet;
  if(sampleRet = bpmSensor.sample() > 0) {
    bpmWifi.getTest();
    bpmWifi.txWindow(ppgArr0, PPG_SLOT0);
    delay(1000);
    bpmWifi.txWindow(ppgArr1, PPG_SLOT1);
    delay(1000);
    bpmWifi.txWindow(ecgArr, ECG_SLOT);
    // Serial.println("WINDOW PRINTING...");
    // for(int i = 0; i < WINDOW_LENGTH; i++) 
    // {
    //   Serial.print(ppgArr0[i]); Serial.print(",");
    //   delay(10);
    //   Serial.print(ppgArr1[i]); Serial.print(",");
    //   delay(10);
    //   Serial.print(ecgArr[i]);  Serial.println("");
    //   delay(1000);
    // }
    delay(1000);
  } else {
    // Serial.println(sampleRet);
  }
  // bpmWifi.getTest();
  
}