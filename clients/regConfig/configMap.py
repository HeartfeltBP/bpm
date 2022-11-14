import numpy as np
import bitarray as ba
import json

bool_opts = {
    "off":  0,
    "on":   1
}

slot_opts = {
    'ppg_led1_ir':      0b0101,
    'ppg_led2_red':     0b0110,
    'pilot_led1_ir':    0b1001,
    'pilot_led_red':    0b1010,
    'ecg':              0b1101
}

adc_range_opts = {
    '4096nA':   0b00,
    '8192nA':   0b01,
    '16384nA':  0b10,
    '32768nA':  0b11
}

sample_rate_opts = {
    'pulse#:1': {
        '10':   0b0000,
        '20':   0b0001,
        '50':   0b0010,
        '84':   0b0011,
        '100':  0b0100,
        '200':  0b0101,
        '400':  0b0110,
        '800':  0b0111,
        '1000': 0b1000,
        '1600': 0b1001,
        '3200': 0b1010
    },
    'pulse#:2': {
        '10':   0b1011,
        '20':   0b1100,
        '50':   0b1101,
        '84':   0b1110,
        '100':  0b1111
    }
}

pulse_w_opts = {
    '50us': 0b00,
    '100us': 0b01,
    '200us': 0b10,
    '400us': 0b11
}

i2c_address = 0x5E

# maybe for single bit settings use 'index' instead of 'range'
# so could just val = ba.index? | ba.range?

maxConfig = {
    'Default': {
        'settings': {
            # set rest to zero (?)
            'reset_control':                    'on',
            'fifo_enable':                      'on',

            'fifo_free_space_before_interrupt': '17',
            'fifo_roll_on_full':                'on',
            'almost_full_flag_persist':         'on',

            'fifo1_slot2':                      'ppg_led2_red',
            
            'adc_range_control':                '32768nA',                  
        }
    }
}



with open()

