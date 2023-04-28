#ifndef HF_CONFIG
#define HF_CONFIG

#define FIRMWARE_VERSION F("0.1.0")
#define DEVICE_ID F("BUNGUSbGljayBteSBudXRzIGFzbGFu69aQ")
#define DEVICE_INFO F("BUNGUSbGljayBteSBudXRzIGFzbGFu69aQ,0.1.0")

// NOT IMPLEMENTED
    #define DEV_XIAOESP32C3 true
    // #define DEV_XIAOSAMD21  false
    // #define DEV_NANO33IOT   false
// NOT IMPLEMNTED

// DEBUG: enables serial output 
#define DEBUG           true
#define VERBOSE         true
#define PRINT           false
#define TB              false
#define ENTERPRISE      false

#if DEBUG
#define SERIAL_ENABLED  true
#endif


#if !DEV_XIAOESP32C3
#define WIFI_ENABLED    false
#endif
#define WIFI_ENABLED    true

#define I2C_ADDRESS     94
#define FRAME_LENGTH   4100
#define SLOT_COUNT      2
#define BUFFER_LENGTH   32
#define SAMPLING_RATE   200

// Interrupts
#define INT_ENABLE false
// 0x00
#if INT_ENABLE
    #define ALMOST_FULL_FLAG_EN true
    #define NEW_PPG_DATA_RDY_EN false
    #define AMBIENTLIGHT_OVF_EN false
    #define PROXIMITY_INTERRUPT false
    #define POWER_READY_FLAG_EN false
    // 0x01
    #define VDD_OUT_OF_RANGE_EN false
    #define NEW_ECG_DATA_RDY_EN false
#endif

#endif