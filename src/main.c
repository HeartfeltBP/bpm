#include <zephyr.h>
#include <sys/printk.h>
#include <string.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>

#include "bpmSensor.h"

int main() 
{
	config();

	ppgInt arr0[WINDOW_LENGTH];
	ppgInt arr1[WINDOW_LENGTH];
	ecgInt arr2[WINDOW_LENGTH];
	ecgInt *arr3 = k_malloc(sizeof(ecgInt) * WINDOW_LENGTH);

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

	for(int i = 0; i < WINDOW_LENGTH; i++) {
		uint32_t val = -1;
		sample(val);
		
		arr0[i] = val;
		arr1[i] = val;
		arr2[i] = val;
		arr3[i] = val;

		printk("Value: %d\n", val);
		k_msleep(500);
	}

	return 0;
}