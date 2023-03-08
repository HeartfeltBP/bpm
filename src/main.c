#include <zephyr.h>
#include <sys/printk.h>
#include <string.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>

#include "bpmSensor.h"

int main() 
{
	config();
	
	const struct device *usb_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint32_t dtr = 0;

	if (usb_enable(NULL)) {
		return -1;
	}

	// while until data terminal ready (dtr/serial open)
	while (!dtr) {
		uart_line_ctrl_get(usb_dev, UART_LINE_CTRL_DTR, &dtr);
		k_sleep(K_MSEC(100));
	}

	for(int i = 0; i < 1000; i++) {
		sample();
		// printk("Value: %d\n", i);
		k_msleep(1000);
	}

	return 0;
}