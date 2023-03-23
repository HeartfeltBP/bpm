#ifndef HF_BPM_MAX
#define HF_BPM_MAX

#include <zephyr.h>
#include <string.h>

#include "constants.h"
#include "i2c.h"

int clear_fifo()
{
    if (i2c_reg_write_byte_dt(&i2c_dt, 0x04, 0))
        return -EIO;
    if (i2c_reg_write_byte_dt(&i2c_dt, 0x05, 0))
        return -EIO;
    if (i2c_reg_write_byte_dt(&i2c_dt, 0x06, 0))
        return -EIO;
}

int config()
{
    // if(!i2c_is_ready_dt(&i2c_dt)) {
    //     printk("I2C not READY - EIO");
    //     return -EIO;
    // }

    // sys control - reset, enable, fifo config
    // if (i2c_reg_write_byte_dt(&i2c_dt, 0x02, 0x80))
    //     return -EIO;
    if (i2c_reg_write_byte_dt(&i2c_dt, 0x0D, 0x01))
        return -EIO;
    if (i2c_reg_write_byte_dt(&i2c_dt, 0x0D, 0x01))
        return -EIO;
    if (i2c_reg_write_byte_dt(&i2c_dt, 0x0D, 0x04))
        return -EIO;
    // write(0x08, 0x7F);
    if (i2c_reg_write_byte_dt(&i2c_dt, 0x08, 0x1F))
        return -EIO;

    /* slot 1 = ppg(ir), slot 2 = ppg(red), slot 3 = ecg */
    if (i2c_reg_write_byte_dt(&i2c_dt, 9U, 0x21))
        return -EIO;

    // // PPG config (protocentral 0xD1 for Config1, maxim 0xD3) 0xD7 us: adc range, sample rate=200/s, led pulse width
    if (i2c_reg_write_byte_dt(&i2c_dt, 14U, 0xD7))
        return -EIO;
    // write(14U, 0xEB);
    // PPG sample averging
    if (i2c_reg_write_byte_dt(&i2c_dt, 15U, 0x00))
        return -EIO;

    // prox interrupt threshold
    if (i2c_reg_write_byte_dt(&i2c_dt, 16U, 0x18))
        return -EIO;

    // set led pulse amplitudes
    if (i2c_reg_write_byte_dt(&i2c_dt, 17U, 0x55))
        return -EIO;
    if (i2c_reg_write_byte_dt(&i2c_dt, 18U, 0x55))
        return -EIO;

    if (SLOT_COUNT > 2)
    {
        // ecg specific settings
        // slot 3 for ecg
        if (i2c_reg_write_byte_dt(&i2c_dt, 10U, 0x09))
            return -EIO;
        // set ECG sampling rate = 200Hz
        if (i2c_reg_write_byte_dt(&i2c_dt, 0x3C, 0x03))
            return -EIO;
        // // set ECG IA gain: 9.5; PGA gain: 8 (idk what this means)
        if (i2c_reg_write_byte_dt(&i2c_dt, 0x3E, 0x0D))
            return -EIO;

        // AFE config
        if (i2c_reg_write_byte_dt(&i2c_dt, 0xFF, 0x54))
            return -EIO;
        if (i2c_reg_write_byte_dt(&i2c_dt, 0xFF, 0x4D))
            return -EIO;
        if (i2c_reg_write_byte_dt(&i2c_dt, 0xCE, 0x0A))
            return -EIO;
        if (i2c_reg_write_byte_dt(&i2c_dt, 0xCF, 0x18))
            return -EIO;
        if (i2c_reg_write_byte_dt(&i2c_dt, 0xFF, 0x00))
            return -EIO;

        printk("ECG Configuration: Complete\n");
    }

    // // 0x14 = 'led range?' = led current (50 mA = 0x00)
    if (i2c_reg_write_byte_dt(&i2c_dt, 0x14, 0x00))
        return -EIO;

    if (clear_fifo()) return -EIO;

    printk("PPG Configuration: Complete\n");
    printk("FIFO Configuration: Complete\n");
}

uint8_t range()
{
    uint8_t readPtr, writePtr;
    if (i2c_reg_read_byte_dt(&i2c_dt, 6U, &readPtr))
        return -EIO;
    if (i2c_reg_read_byte_dt(&i2c_dt, 4U, &writePtr))
        return -EIO;

    return writePtr - readPtr;
}

int sample(uint32_t val)
{
    uint8_t fifoRange = range();

    if (fifoRange != 0)
    {
        if (fifoRange < 0)
            fifoRange += 32;

        int available = fifoRange * SLOT_COUNT * 3;

        printk("[%d]\n", fifoRange);
        while (available > 0)
        {
            // add ovf read for repeated starts
            int readBytes = available;

            if (readBytes > 32)
                readBytes = (32 - (32 % (3 * SLOT_COUNT)));

            uint8_t buffer[SLOT_COUNT][SAMPLE_INT_SIZE];

            for (int i = 1; readBytes > 0; readBytes -= (SLOT_COUNT * 3), i++)
            {
                int ret = i2c_burst_read_dt(&i2c_dt, 7U, buffer[i], SAMPLE_INT_SIZE);

                if ((i % 3) == 0)
                {
                    ecgInt ecgVal;
                    memcpy(&ecgVal, buffer[i], SAMPLE_INT_SIZE);

                    if (ecgVal & (1 << 17))
                        ecgVal -= (1 << 18);

                    ecgVal &= 0x3FFFF;
                    printk("%d:%ld\n", i, ecgVal);
                }
                else
                {
                    ppgInt ppgVal;
                    memcpy(&ppgVal, buffer[i], SAMPLE_INT_SIZE);

                    if ((i % 2) == 0)
                    {
                        printk("%d:%lu\n", i, ppgVal);
                    }
                    else
                    {
                        printk("%d:%lu\n", i, ppgVal);
                    }
                }
            }

            // uint32_t view;
            // view = buffer[0] << 16 | (buffer[1] << 8) | (buffer[2]);
            // printk("VIEW0: %d\n", view);

            // view = buffer[3] << 16 | (buffer[4] << 8) | (buffer[5]);
            // printk("VIEW1: %d\n", view);

            // view = buffer[6] << 16 | (buffer[7] << 8) | (buffer[8]);
            // printk("VIEW2: %d\n", view);
            // printk("{0: %d}\n", temp0);
            // printk("{1: %d}\n", temp1);
            // printk("{2: %d}\n", temp2);

            // printk("%d,%d,%d\n", temp0, temp1, temp2);

            // printk("RET: %d\n", ret);
            val = -69;

            // fifoRange = range();
            // printk("[++%d\n]", fifoRange);
            // k_msleep(10);
        }
    }
    return 0;
}

#endif // HF_BPM_MAX