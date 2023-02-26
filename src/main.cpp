// // #include <Arduino.h>
// // #include <Wire.h>

// // #include "./include/bpmWiFi.hpp"
// // #include "./include/bpmSensor.hpp"
// #include <zephyr.h>
// // max https://datasheets.maximintegrated.com/en/ds/MAX86150.pdf

// // hf::WindowHandler *windowHandler;
// // hf::BpmSensor *bpmSensor;
// // hf::BpmWiFi *bpmWiFi;

// int main()
// {
//     printk("Hello World! %s\n", CONFIG_BOARD);
    
//     // bpmWiFi = new hf::BpmWiFi(SSID, PASS);
//     // bpmWiFi->initWiFi(true);

//     // windowHandler = new hf::WindowHandler(SLOT_COUNT, false);
//     // bpmSensor = new hf::BpmSensor(windowHandler, SLOT_COUNT);

//     // Serial.begin(115200);
//     // Wire.begin();
//     // Wire.setClock(400000);
//     // Wire.setClock(100000);

//     // bpmSensor->init();

//     // loop until serial connection opens - diagnostic
//     // while (!Serial)
//     //     delay(1);
    
//     // Serial.println("Setup Complete");
//     // delay(100);
//     return 0;
// }

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0	""
#define PIN	0
#define FLAGS	0
#endif


void main(void)
{
	const struct device *dev;
	bool led_is_on = true;
	int ret;

	dev = device_get_binding(LED0);
	if (dev == NULL) {
		return;
	}

	ret = gpio_pin_configure(dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret < 0) {
		return;
	}

	while (1) {
		gpio_pin_set(dev, PIN, (int)led_is_on);
		led_is_on = !led_is_on;
		k_msleep(SLEEP_TIME_MS);
        printk("BALLS");
		const int *balls = (int*)k_malloc(10);
		k_free((void*)balls);
	}
}