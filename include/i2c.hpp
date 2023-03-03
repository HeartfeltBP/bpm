#include <zephyr.h>
#include <sys/printk.h>
#include <string.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>
#include <drivers/i2c.h>
// #include "bpmSensor.hpp"

#if DT_NODE_HAS_STATUS(DT_ALIAS(sercom_4), okay) \
&& DT_NODE_HAS_STATUS(DT_NODELABEL(max86150), okay)
#define I2C0_NODE DT_NODELABEL(max86150)
#endif

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console),zephyr_cdc_acm_uart),\
"Console device is not ACM CDC UART device");
BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_NODELABEL(max86150), atmel));

static const struct i2c_dt_spec i2c_dt = I2C_DT_SPEC_GET(I2C0_NODE);

// namespace hf {

// 	class I2c {
// 		protected:

// 			// add config stuff here
		
// 		public:
// 			I2c() {}
			
// 		struct i2c_dt_spec getI2cSpec() 
// 		{
// 			const struct i2c_dt_spec i2c_dt = I2C_DT_SPEC_GET(I2C0_NODE);
// 			return i2c_dt;
// 		}
// 	};

// }
