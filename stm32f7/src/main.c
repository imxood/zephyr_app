#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>

extern void test_section();

extern void test_sensor_bme280(void);
extern void test_spi_bme280(void);

extern void test_gpio(void);

void test() {

#if CONFIG_TEST_GPIO
	test_gpio();
#endif

#if CONFIG_TEST_SPI_PI
	test_spi();
#endif

#if CONFIG_TEST_BME280
	test_spi_bme280();
//	test_i2c_bme280();
	test_sensor_bme280();
#endif

}

#if CONFIG_ZTEST
void test_main(void)
#else
void main(void)
#endif
{
	printk("hello, the world\n");
	test();

	// test_section();
	/* test_userspace(); */
}
