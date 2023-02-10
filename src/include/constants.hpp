#define I2C_ADDRESS 94
#define WINDOW_LENGTH 256
#define SLOT_COUNT 3
#define BUFFER_LENGTH 32

// PPG & ECG SLOTS
// CUR CODE IS BASED OFF THIS SLOT STRUCTURE
// slot 0 and slot 1 can be either red or ir depending on config 
/* (currently 0 = IR, 1 = RED) */
#define PPG_SLOT0 1
#define PPG_SLOT1 2
#define ECG_SLOT  3
