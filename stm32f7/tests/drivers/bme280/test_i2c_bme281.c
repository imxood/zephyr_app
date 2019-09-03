#include <zephyr.h>
#include <device.h>
#include <i2c.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(i2c_bme, 4);

#define BUF_SIZE 128

#define I2C_DEVICE      CONFIG_I2C_1_NAME

void test_i2c_bme280(void) {

	int ret = 0;
	uint8_t id;

	struct device *dev = device_get_binding(I2C_DEVICE);

	if (dev == NULL) {
		LOG_ERR("get device '%s' failed", I2C_DEVICE);
		return;
	}

	/*ret = i2c_configure(dev, &i2c_cfg);

	if (ret) {
		LOG_ERR("i2c configure failed, ret: %d", ret);
		return;
	}*/

	while (1) {
		ret = i2c_burst_read(dev, 0x76, 0xd0, &id, 1);

		if (ret == 0) {
			LOG_INF("i2c get id: %d", id);
			break;
		}

		LOG_ERR("i2c get id failed, ret: %d", ret);
		k_sleep(2000);
	}

}
