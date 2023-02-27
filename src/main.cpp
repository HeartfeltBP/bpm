#include <zephyr.h>
#include <sys/printk.h>
#include <string.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>
#include "bpmSensor.hpp"

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console),zephyr_cdc_acm_uart), "Console device is not ACM CDC UART device");

int main() 
{
	// const struct device *max_dev = DEVICE_DT_GET(DT_CHOSEN(max86150));
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



	return 0;
}