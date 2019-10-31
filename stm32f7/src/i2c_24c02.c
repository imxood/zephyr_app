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

int i2c_24c02_read(struct device *dev, uint8_t start_addr, uint8_t *data, uint8_t len)
{
    // 1010(固定值) 000(A2A1A0)0(R:1,W:0)
    uint8_t addrs[] = {0xa0, start_addr};

    struct i2c_msg msgs[3];
    uint8_t addr_read = 0xa1;

    // write: device write address, device word address
    msgs[0].buf = addrs;
    msgs[0].len = 2;
    msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_RESTART;

    // write: device read address
    msgs[1].buf = &addr_read;
    msgs[1].len = 1;
    msgs[1].flags = I2C_MSG_WRITE | I2C_MSG_RESTART;

    // read data
    msgs[2].buf = data;
    msgs[2].len = len;
    msgs[2].flags = I2C_MSG_READ | I2C_MSG_STOP;

    return i2c_transfer(dev, msgs, 3, I2C_SLAVE_ADDR);
}

int i2c_24c02_write(struct device *dev, uint8_t start_addr, uint8_t *data, uint8_t len)
{
    // 1010(固定值) 000(A2A1A0)0(R:1,W:0)
    uint8_t addrs[] = {0xa0, start_addr};

    struct i2c_msg msgs[3];
    uint8_t addr_read = 0xa1;

    // write: device write address, device word address
    msgs[0].buf = addrs;
    msgs[0].len = 2;
    msgs[0].flags = I2C_MSG_WRITE;

    // write: device read address

    // write data
    msgs[1].buf = data;
    msgs[1].len = len;
    msgs[1].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

    return i2c_transfer(dev, msgs, 3, I2C_SLAVE_ADDR);
}

void test_i2c_24c02(void)
{
    int32_t ret = 0;
    uint8_t buf[256] = {0};
    printk("%s[%d]\n", __func__, __LINE__);

    i2c_dev = device_get_binding(I2C_DEV_NAME);
    if (i2c_dev == NULL)
    {
        printk("can't get i2c device[%s]\n", I2C_DEV_NAME);
        return;
    }

    ret = i2c_configure(i2c_dev, i2c_cfg);
    if (ret) {
        printk("run i2c_configure failed, ret: %d\n", ret);
        return;
    }

    ret = i2c_24c02_read(i2c_dev, 0x00, buf, 256);
    if (ret) {
        printk("run i2c_24c02_read failed, ret: %d\n", ret);
        return;
    }
    hexdump(buf, 256);

    for (int i = 0; i < 256; i++) {
        buf[i] = i;
    }

    ret = i2c_24c02_write(i2c_dev, 0x00, buf, 256);
    if (ret) {
        printk("run i2c_24c02_write failed, ret: %d\n", ret);
        return;
    }

    memset(buf, 0, 256);

    ret = i2c_24c02_read(i2c_dev, 0x00, buf, 256);
    if (ret) {
        printk("run i2c_24c02_read failed, ret: %d\n", ret);
        return;
    }
    hexdump(buf, 256);
}

void e24c02_service(struct device *unused)
{
    ARG_UNUSED(unused);
    test_i2c_24c02();
}

SYS_INIT(e24c02_service, APPLICATION, 90);
