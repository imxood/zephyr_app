#include <zephyr.h>
#include <device.h>
#include <spi.h>
#include <logging/log.h>


LOG_MODULE_REGISTER(spi_pi, 4);


#define BUF_SIZE 128
static u8_t buffer_tx[BUF_SIZE];
static u8_t buffer_rx[BUF_SIZE];


#define SPI_DEVICE      DT_SPI_2_NAME

static struct spi_config spi_cfg = {
	.frequency = 400000,
	.operation = SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_LINES_SINGLE,
	.slave = 0,
	.cs = NULL
};

void test_spi_bme280(void)
{
	buffer_tx[0] = 0xd0;
	buffer_tx[1] = 0xff;

    const struct spi_buf tx_bufs[] = {
        {
            .buf = buffer_tx,
            .len = 2,
        },
    };

    const struct spi_buf rx_bufs[] = {
        {
            .buf = buffer_rx,
            .len = 2,
        },
    };

    const struct spi_buf_set tx = {
        .buffers = tx_bufs,
        .count = 1
    };

    const struct spi_buf_set rx = {
        .buffers = rx_bufs,
        .count = 1
    };

    struct device *dev = device_get_binding(SPI_DEVICE);

    if (dev == NULL) {
        LOG_ERR("cannot find device [%s]", log_strdup(SPI_DEVICE));
        return;
    }

    int ret;

    LOG_INF("Start");

    while(1) {

        (void)memset(buffer_rx, 0, BUF_SIZE);

        LOG_HEXDUMP_INF(buffer_tx, 2, "sent data:");

        ret = spi_transceive(dev, &spi_cfg, &tx, &rx);

        if (ret) {
            LOG_ERR("SPI transceive failed, ret: %d", ret);
        }
        else {
            LOG_HEXDUMP_INF(buffer_rx, 2, "received data:");
        }
        k_sleep(2000);
    }

}
