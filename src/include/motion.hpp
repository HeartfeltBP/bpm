#ifndef HF_BPM_MOTION
#define HF_BPM_MOTION

#include <Arduino.h>
#include <Arduino_LSM6DS3.h>

namespace hf
{
    class Accelerometer
    {
        protected:
            float x, y, z;

        public:
            Accelerometer( ) {
                if(IMU.begin()) {
                    Serial.println("IMU init Success");
                    Serial.print("Sampling rate: "); Serial.println(IMU.accelerationSampleRate());
                } else {
                    Serial.println("IMU init Failed");
                }
            }

            float getPos() {

            }
        
    

    };

    class Gyro
    {
        protected:
            float x, y, z;

        public:
            Gyro() {
                if(IMU.begin()) {
                    Serial.println("IMU init Success");
                    Serial.print("Sampling rate: "); Serial.println(IMU.gyroscopeSampleRate());
                } else {
                    Serial.println("IMU init Failed");
                }
            }

            float getPos() {
                
            }

            
        
    }
}


#endif