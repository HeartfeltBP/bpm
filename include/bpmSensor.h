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
        
    return 0;
}

int config()
{
    // if(i2c_(&i2c_dt)) {
    //     printk("I2C not READY - EIO");
    //     return -EIO;
    // }

    int addr = i2c_dt.addr;
    int bus = i2c_dt.bus;

    // sys control - reset, enable, fifo config
    if (i2c_reg_write_byte_dt(&i2c_dt, 0x02, 0x80))
        return -EIO;
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
    #if SLOT_COUNT >= 1
    if (i2c_reg_write_byte_dt(&i2c_dt, 9U, 0x02))
        return -EIO;
    #elif SLOT_COUNT >= 2
    if (i2c_reg_write_byte_dt(&i2c_dt, 9U, 0x21))
        return -EIO;
    #endif

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

    #if (SLOT_COUNT > 2)
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
    #endif

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
    // uint8_t available = range();

    // if (available != 0)
    // {
    //     if (available < 0)
    //         available += 32;

        // for (int i = 0; i < available; i += SLOT_COUNT)
        // {
            // // add ovf read for repeated starts
            // int readBytes = available;

            // if ((readBytes * 3) > 32)
            //     readBytes = (32 - (32 % (3 * SLOT_COUNT)));

            uint32_t ppgBuffer0;
            // uint32_t ppgBuffer1;
            // int32_t  ecgBuffer0;
            uint8_t buffer[SLOT_COUNT*3];

            // int addr = 7U;
            if(i2c_burst_read_dt(&i2c_dt, 7, buffer, SLOT_COUNT * 3)) {printk("EIO ERROR\n"); return -EIO;}
            
            memcpy(&ppgBuffer0, buffer, 4);
            // ppgBuffer0 = ((buffer[0] << 16) | (buffer[1] << 8) | buffer[2]) & 0x7FFFF;
            ppgBuffer0 &= 0x7FFFF;
            printk("%u,\n", ppgBuffer0);

            // memcpy(&ppgBuffer1, &buffer[3], 3U);
            // // ppgBuffer1 = (buffer[3] | buffer[4] | buffer[5]) & 0x7FFFF;
            // ppgBuffer1 &= 0x7FFFF;
            // // printk("VIEW1: %u\n", ppgBuffer1);

            // memcpy(&ppgBuffer0, &buffer[6], 3U);
            // // int32_t ecgTemp = (buffer[6] << 16 | buffer[7] << 8 | buffer[8]);
            // // ecgBuffer0 = ecgTemp & (1 << 17) ? ecgTemp -= (1 << 18) & 0x3FFFF : ecgTemp & 0x3FFFF;
            // ecgBuffer0 &= 0x3FFFF;
            // // printk("VIEW1: %d\n", ecgBuffer0);
    //     }
    // }
    return 0;
}

#endif // HF_BPM_MAX