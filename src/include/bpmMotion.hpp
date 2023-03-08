// #ifndef HF_BPM_MOTION
// #define HF_BPM_MOTION

// #include <Arduino.h>
// #include <Arduino_LSM6DS3.h>

// namespace hf
// {
//     class Accelerometer
//     {
//     protected:
//         float _x, _y, _z;

//     public:
//         Accelerometer()
//         {
//             if (IMU.begin())
//             {
//                 Serial.println("Acc: IMU init Success");
//                 Serial.print("Sampling rate: ");
//                 Serial.println(IMU.accelerationSampleRate());
//             }
//             else
//             {
//                 Serial.println("Acc: IMU init Failed");
//             }
//         }

//         float *getPos()
//         {
//             if (!IMU.accelerationAvailable())
//                 return;
//             IMU.readAcceleration(_x, _y, _z);
//             return new float[3]{_x, _y, _z};
//         }
//     };

//     class Gyro
//     {
//     protected:
//         float _x, _y, _z;

//     public:
//         Gyro()
//         {
//             if (IMU.begin())
//             {
//                 Serial.println("Gyro: IMU init Success");
//                 Serial.print("Sampling rate: ");
//                 Serial.println(IMU.gyroscopeSampleRate());
//             }
//             else
//             {
//                 Serial.println("ERROR: IMU init Failed");
//             }
//         }

//         float *getPos()
//         {
//             if (!IMU.gyroscopeAvailable())
//                 return;
//             IMU.readAcceleration(_x, _y, _z);
//             return new float[3]{_x, _y, _z};
//         }
//     };
// }

// #endif