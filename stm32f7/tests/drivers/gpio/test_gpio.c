#include <zephyr.h>
#include <device.h>
#include <common.h>
#include <drivers/gpio.h>

#define GPIO_DEVICE 	"GPIOA"

#define GPIO_PIN_IN 	4
#define GPIO_PIN_OUT 	5

struct device *dev;

void test_gpio_read_write() {

	gpio_pin_disable_callback(dev, GPIO_PIN_IN);
	gpio_pin_configure(dev, GPIO_PIN_IN, GPIO_DIR_IN);

	gpio_pin_configure(dev, GPIO_PIN_OUT, GPIO_DIR_OUT);

	u32_t value = 0;
	int count = 5000000;

	while (count--) {
		value = 0;
		gpio_pin_read(dev, GPIO_PIN_IN, &value);
		gpio_pin_write(dev, GPIO_PIN_OUT, !value);
		k_sleep(2000);
	}
	LOG_INF("ok~");
}

void test_gpio(void) {
	dev = device_get_binding(GPIO_DEVICE);
	if (dev == NULL) {
		LOG_ERR("Can't get device '%s'", GPIO_DEVICE);
		return;
	}
	LOG_INF("Get device '%s'", GPIO_DEVICE);
	test_gpio_read_write();
}
