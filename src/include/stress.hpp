#ifndef HF_BPM_STRESS
#define HF_BPM_STRESS

#include <Arduino.h>
#include "bpmSensor.hpp"
#include "utils.hpp"

namespace hf
{
    class StressTest
    {
        protected:

        public:
            StressTest()
            {}

            /* ## basic math ## */

            // add param for array of certain vals
            void add_i(int iter, int delayVal)
            {
                for(int i = 0; i < iter; i++)
                {
                    int temp = i + iter;
                }
            }

            void mult_i(int iter, int delayVal)
            {
                for(int i = 0; i < iter; i++)
                {
                    int temp = i * i;
                }
            }

            void div_i(int iter, int delayVal)
            {
                for(int i = 0; i < iter; i++)
                {
                    int temp = i / 2;
                }
            }

            /* ## bpm tests ## */

            void basic() 
            {
                Serial.println("Creating Sensor Object");
                hf::BpmSensor sensorObject = hf::BpmSensor(1);
                Serial.println("Delaying for 10 seconds");
                delay(10000);

                Serial.println("Adding (iter sim)");
                add_i(12000, 0);
                Serial.println("Multiplying");
                mult_i(256, 0);
                Serial.println("Adding w. 1ms delay");
                add_i(256, 1);

                Serial.println("Delaying for 10 seconds");
                delay(10000);

                Serial.println("Initializing sensor");
                sensorObject.init();

                Serial.println("Delaying for 10 seconds");
                delay(10000);
                
                Serial.println("Entering read loop for 10,000 iterations");
                for(int i = 0; i < 10000; i++) 
                {
                    sensorObject.sample();
                }


            }

            void intense() 
            {
                Serial.println("Creating Sensor Object");
                hf::BpmSensor sensorObject = hf::BpmSensor(1);
                Serial.println("Delaying for 10 seconds");
                delay(10000);

                Serial.println("Adding (iter sim)");
                add_i(12000, 0);
                Serial.println("Multiplying");
                mult_i(256, 0);
                mult_i(256, 0);
                Serial.println("Adding w. 1ms delay");
                add_i(256, 0);
                Serial.println("Sividing");
                div_i(256, 0);

                Serial.println("Delaying for 10 seconds");
                delay(10000);

                Serial.println("Initializing sensor");
                sensorObject.init();

                Serial.println("Delaying for 10 seconds");
                delay(10000);
                
                Serial.println("Entering read loop for 10,000 iterations");
                for(int i = 0; i < 10000; i++) 
                {
                    sensorObject.sample();
                }


            }



    

    };
}


#endif