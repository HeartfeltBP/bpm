#ifndef HF_BPM_STRESS
#define HF_BPM_STRESS

#include <Arduino.h>

namespace hf
{
    class StressTest
    {
        protected:
            // this is dumb
            bool _simple,
             _moderate,
             _intense,
             _fp_simple,
             _fp_intense;

        public:
            StressTest(bool simple, bool moderate, bool intense, bool fp_simple, bool fp_intense)
            : _simple{simple}, _intense{intense}, _fp_simple{fp_simple}, _fp_intense{fp_intense}{}

            void begin()
            {
                // case
            }

            /* ## basic math ## */

            // add param for array of certain vals
            void add_i(int iter, int delay)
            {
                for(int i = 0; i < iter; i++)
                {
                    int temp = i + iter;
                }
            }

            void mult_i(int iter, int delay)
            {
                for(int i = 0; i < iter; i++)
                {
                    int temp = i * i;
                }
            }

            void div_i(int iter, int delay)
            {
                for(int i = 0; i < iter; i++)
                {
                    int temp = i / 2;
                }
            }

            int basic()
            {
                add_i(10000, 2);
                delay(20);
                // mult_i()
            }

            /* ## bpm tests ## */

            void read_sensor() {

            }

            void transmit_vect() {

            }



    

    }
};


#endif