#ifndef HF_CONST
#define HF_CONST

typedef unsigned long int ppgInt;
typedef long int ecgInt;

#define I2C_ADDRESS 94
#define WINDOW_LENGTH 4100
#define SLOT_COUNT 3
#define BUFFER_LENGTH 32

// PPG & ECG SLOTS
// CUR CODE IS BASED OFF THIS SLOT STRUCTURE
// slot 0 and slot 1 can be either red or ir depending on config 
/* (currently 0 = IR, 1 = RED) */
#define PPG_SLOT0 0
#define PPG_SLOT1 1
#define ECG_SLOT  2

#endif