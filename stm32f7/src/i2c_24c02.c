#include <zephyr.h>
#include <init.h>
#include <drivers/i2c.h>
#include <helper.h>

// iic_scl, PH4
// iic_sda, PH5
#define I2C_DEV_NAME CONFIG_I2C_2_NAME
#define I2C_SLAVE_ADDR 0x50

static struct device *i2c_dev;

static u32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER;


void test_zephyr_i2c_write_read()
{
	const char *devname = "I2C_2";
	struct device *i2c_dev = device_get_binding(devname);
	if (i2c_dev == NULL) {
		printk("can't get ic2_dev['%s']\n", devname);
		return;
	}

	u32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_FAST) | I2C_MODE_MASTER;

	if (i2c_configure(i2c_dev, i2c_cfg)) {
		printk("I2C config Fail.\n");
		return;
	}

    k_sleep(100);

	uint8_t buf[5] = {0, 0, 1, 2, 3};

	if (i2c_write(i2c_dev, buf, 5, 0x50)) {
		printk("i2c_write Fail.\n");
		return;
	}
    k_sleep(100);

	printk("write data:\n");
	hexdump(buf+1, 4);

	if (i2c_write(i2c_dev, buf, 1, 0x50)) {
		printk("i2c_write Fail.\n");
		return;
	}

    k_sleep(100);

	if (i2c_read(i2c_dev, buf + 1, 4, 0x50)) {
		printk("i2c_read Fail.\n");
		return;
	}

    k_sleep(100);

	printk("read data:\n");
	hexdump(buf+1, 4);


    buf[1] = 2;
	buf[2] = 3;
	buf[3] = 4;
    buf[4] = 5;

	if (i2c_write(i2c_dev, buf, 5, 0x50)) {
		printk("i2c_write Fail.\n");
		return;
	}
	printk("write data:\n");
	hexdump(buf+1, 4);

    k_sleep(100);

	if (i2c_write(i2c_dev, buf, 1, 0x50)) {
		printk("i2c_write Fail.\n");
		return;
	}

    k_sleep(100);

	if (i2c_read(i2c_dev, buf + 1, 4, 0x50)) {
		printk("i2c_read Fail.\n");
		return;
	}

	printk("read data:\n");
	hexdump(buf+1, 4);
}

void e24c02_service(struct device *unused)
{
    ARG_UNUSED(unused);
    test_zephyr_i2c_write_read();
}

SYS_INIT(e24c02_service, APPLICATION, 90);
