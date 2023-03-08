#ifndef HF_BPM_MAX
#define HF_BPM_MAX

#include <zephyr.h>
#include <string.h>

#include "constants.h"
#include "i2c.h"

void config()
{
    // sys control - reset, enable, fifo config
    // write(0x02, 0x80);
    i2c_reg_write_byte_dt(&i2c_dt, 0x0D, 0x01);

    i2c_reg_write_byte_dt(&i2c_dt, 0x0D, 0x01);
    i2c_reg_write_byte_dt(&i2c_dt, 0x0D, 0x04);
    // write(0x08, 0x7F);
    i2c_reg_write_byte_dt(&i2c_dt, 0x08, 0x1F);

    /* slot 1 = ppg(ir), slot 2 = ppg(red), slot 3 = ecg */
    i2c_reg_write_byte_dt(&i2c_dt, 9U, 0x02); 
    // write(10U, 0x09);

    // // PPG config (protocentral 0xD1 for Config1, maxim 0xD3) 0xD7 us: adc range, sample rate=200/s, led pulse width
    i2c_reg_write_byte_dt(&i2c_dt, 14U, 0xD7);
    // write(14U, 0xEB);
    // PPG sample averging
    i2c_reg_write_byte_dt(&i2c_dt, 15U, 0x00);

    // prox interrupt threshold
    i2c_reg_write_byte_dt(&i2c_dt, 16U, 0x18);

    // set led pulse amplitudes
    i2c_reg_write_byte_dt(&i2c_dt, 17U, 0xFF);
    i2c_reg_write_byte_dt(&i2c_dt, 18U, 0xFF);

    if (SLOT_COUNT > 2)
    {
        // ecg specific settings
        i2c_reg_write_byte_dt(&i2c_dt, 10U, 0x09);
        // set ECG sampling rate = 200Hz
        i2c_reg_write_byte_dt(&i2c_dt, 0x3C, 0x03);
        // // set ECG IA gain: 9.5; PGA gain: 8 (idk what this means)
        i2c_reg_write_byte_dt(&i2c_dt, 0x3E, 0x0D);

        // AFE config
        i2c_reg_write_byte_dt(&i2c_dt, 0xFF, 0x54);
        i2c_reg_write_byte_dt(&i2c_dt, 0xFF, 0x4D);
        i2c_reg_write_byte_dt(&i2c_dt, 0xCE, 0x0A);
        i2c_reg_write_byte_dt(&i2c_dt, 0xCF, 0x18);
        i2c_reg_write_byte_dt(&i2c_dt, 0xFF, 0x00);

        printk("ECG Configuration: Complete\n");
    }

    // // 0x14 = 'led range?' = led current (50 mA = 0x00)
    i2c_reg_write_byte_dt(&i2c_dt, 0x14, 0x00);

    // i2c_reg_write_byte_dt(&i2c_dt, 0x04, 0);
    // i2c_reg_write_byte_dt(&i2c_dt, 0x05, 0);
    // i2c_reg_write_byte_dt(&i2c_dt, 0x06, 0);

    printk("PPG Configuration: Complete\n");
    printk("FIFO Configuration: Complete\n");
}

uint8_t range()
{
    uint8_t readPtr, writePtr;
    i2c_reg_read_byte_dt(&i2c_dt, 6U, &readPtr);
    i2c_reg_read_byte_dt(&i2c_dt, 4U, &writePtr);

    return writePtr - readPtr;
}

void sample(uint32_t val)
{
    // uint8_t fifoRange = range();
    // printk("%d", fifoRange);

    // if (fifoRange != 0)
    // {
    //     if (fifoRange < 0)
    //         fifoRange += 32;
        
    //     int available = fifoRange * SLOT_COUNT * 3;
    //     // i2c_reg_write_byte_dt(&i2c_dt, 7U, NULL);

    //     while (available > 0)
    //     {
    //         int readBytes = available;

    //         if (readBytes > 32)
    //             readBytes = (32 - (32 % (3 * SLOT_COUNT)));

    //         available -= readBytes;

    //         uint8_t retVal[SLOT_COUNT];
    //         i2c_burst_read_dt(&i2c_dt, 7U, retVal, SLOT_COUNT * 3);
    //         readBytes -= SLOT_COUNT * 3;

    //         uint32_t temp; // add ecg condition
    //         memcpy(&temp, retVal, sizeof(temp));

    //         printk("%d\n", temp);
    //     }
    // }

    uint8_t retVal[SLOT_COUNT];
    i2c_burst_read_dt(&i2c_dt, 7U, retVal, SLOT_COUNT * 3);

    uint32_t temp; // add ecg condition
    memcpy(&temp, retVal, sizeof(temp));
    printk("%d\n", temp);

    val = &temp;
    return;
}

#endif // HF_BPM_MAX