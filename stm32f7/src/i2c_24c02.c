#include <zephyr.h>
#include <drivers/i2c.h>


// iic_scl, PH4
// iic_sda, PH5
#define I2C_DEV_NAME        "I2C_2"
#define I2C_SLAVE_ADDR      0x50

static struct device *i2c_dev;

static u32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER;

int i2c_24c02_read(struct device *dev, uint8_t offset_addr, uint8_t *data, uint8_t len)
{
    // 1010(固定值) 000(A2A1A0)0(R:1,W:0)
    uint8_t reg_addr = 0xa0;

    struct i2c_msg msgs[2];

    // write addr
    msgs[0].buf = &reg_addr;
    msgs[0].len = 1;
    msgs[0].flags = I2C_MSG_WRITE;

    // read data
    msgs[1].buf = &reg_addr;
    msgs[1].len = 1;
    msgs[1].flags = I2C_MSG_READ | I2C_MSG_RESTART | I2C_MSG_STOP;

    return i2c_transfer(i2c_dev, msgs, 2, I2C_SLAVE_ADDR);
}

void test_i2c_24c02(void)
{
    i2c_dev = device_get_binding(I2C_DEV_NAME);
    if (i2c_dev == NULL) {
        printk("can't get i2c device[%s]\n", I2C_DEV_NAME);
        return;
    }

    i2c_configure(i2c_dev, i2c_cfg);



    i2c_transfer()

}
