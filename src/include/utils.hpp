#ifndef HF_BPM_UTILS
#define HF_BPM_UTILS

extern "C" char* sbrk(int incr);

int getFreeRam()
{
    char top;
    return &top - reinterpret_cast<char*>(sbrk(0));
}

namespace hf 
{
    void wait() {

    }
}


#endif