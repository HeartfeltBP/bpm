#ifndef HF_BPM_UTILS
#define HF_BPM_UTILS

extern "C" char* sbrk(int incr);

int freeRam()
{
    char top;
    return &top - reinterpret_cast<char*>(sbrk(0));
}


#endif