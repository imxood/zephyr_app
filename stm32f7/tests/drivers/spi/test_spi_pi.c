#include <zephyr.h>
#include <device.h>
#include <spi.h>
#include <logging/log.h>


LOG_MODULE_REGISTER(spi_pi, 4);


#define BUF_SIZE 17
u8_t buffer_tx[] = "0123456789abcdef\0";
u8_t buffer_rx[BUF_SIZE] = {};


#define SPI_DEVICE      DT_SPI_2_NAME

struct spi_config spi_cfg_slow = {
	.frequency = 300000,
	.operation = SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_LINES_SINGLE,
	.slave = 0,
	.cs = NULL
};

struct spi_config spi_cfg_fast = {
	.frequency = 16000000,
	.operation = SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_LINES_SINGLE,
	.slave = 0,
	.cs = NULL
};

void test_spi(void)
{
    const struct spi_buf tx_bufs[] = {
        {
            .buf = buffer_tx,
            .len = BUF_SIZE,
        },
    };

    const struct spi_buf rx_bufs[] = {
        {
            .buf = buffer_rx,
            .len = 8,
        },
    };

    const struct spi_buf_set tx = {
        .buffers = tx_bufs,
        .count = ARRAY_SIZE(tx_bufs)
    };

    const struct spi_buf_set rx = {
        .buffers = rx_bufs,
        .count = ARRAY_SIZE(rx_bufs)
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

        LOG_INF("send data: '%s', wait to responce...", log_strdup(buffer_tx));

        ret = spi_transceive(dev, &spi_cfg_slow, &tx, &rx);

        if (ret) {
            LOG_ERR("SPI transceive failed, ret: %d", ret);
        }
        else {
            LOG_HEXDUMP_INF(buffer_rx, sizeof(buffer_rx), "received data:");
        }
        k_sleep(1000);
    }

}
