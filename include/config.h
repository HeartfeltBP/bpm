#ifndef HF_CONFIG
#define HF_CONFIG

#define DEV_XIAOESP32C3 true
// #define DEV_XIAOSAMD21  false
// #define DEV_NANO33IOT   false

// DEBUG: enables serial output 
#define DEBUG           false
#define TB              false

#if DEBUG
#define SERIAL_ENABLED  true
#endif
#define SERIAL_ENABLED false
#define WIFI_ENABLED    true

#define I2C_ADDRESS     94
#define WINDOW_LENGTH   4100
#define SLOT_COUNT      3
#define BUFFER_LENGTH   32
#define SAMPLING_RATE   200

#endif