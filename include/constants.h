#ifndef HF_CONST
#define HF_CONST

#include "config.h"

typedef unsigned long int ppgInt;
typedef long int ecgInt;

#if DEBUG
    #define DBG(...) __VA_ARGS__
#else
    #define DBG(...)
#endif

#if VERBOSE
    #define LOG(...) Serial.print(__VA_ARGS__)
    #define LOG_LN(...) Serial.println(__VA_ARGS__)

    // log value that will be stored in heap, only use with string literals?
    #define LOG_H(...) Serial.print(F(__VA_ARGS__))
    #define LOG_H_LN(...) Serial.println(F(__VA_ARGS__))
#else
    #define LOG(...)
    #define LOG_LN(...)

    // only use with string literals?
    #define LOG_H(...)
    #define LOG_H_LN(...)
#endif

// PPG & ECG SLOTS
// CUR CODE IS BASED OFF THIS SLOT STRUCTURE
// slot 0 and slot 1 can be either red or ir depending on config 
/* (currently 0 = IR, 1 = RED) */
#define PPG_SLOT0 0
#define PPG_SLOT1 1
#define ECG_SLOT0 2

#define DISABLED_SLOT -1

#define TEST_ENDPOINT   F("/api/test")
#define RX_ENDPOINT     F("/api/rx")
#define TOKEN_ENDPOINT  F("/api/token")
#define SERVE_PORT 80

#define HTTP_PLAINTEXT F("text/plain")

#endif