#include <zephyr.h>
#include <device.h>
#include <shell/shell.h>
#include <logging/log.h>
#include <spi.h>
#include <helper.h>

LOG_MODULE_REGISTER(test_esp32_spi, 4);

#define SPI_DEV ("SPI_2")

#define WRITE_BUF_LEN 128
#define READ_BUF_LEN 128

static u8_t buffer_write[WRITE_BUF_LEN];
static u8_t buffer_read[READ_BUF_LEN];


void esp32_reset()
{
	struct device* spi_dev = device_get_binding(SPI_DEV);

	if (spi_dev) {
		LOG_INF("Exevuting device_get_binding successed");
	}
	else {
		LOG_ERR("Exevuting device_get_binding failed");
		return;
	}
	struct spi_config spi_cfg = {
		.frequency = 1000000,
		.operation = SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE,
		.slave = 0,
		.cs = NULL
	};

	/* write data */

	buffer_write[0] = ;

}


static int shell_entry(const struct shell *shell, size_t argc, char **argv)
{
    if(strcmp(argv[0], "reset") == 0){
        esp32_reset();
    }

	else if(strcmp(argv[0], "reset") == 0){
        LOG_DBG("spi failed");
    }

    return 0;
}



SHELL_CREATE_STATIC_SUBCMD_SET(sub_test){
	SHELL_CMD_ARG(reset, NULL, "reset esp32 module.", shell_entry, 1, 0),
    SHELL_SUBCMD_SET_END
};

/* Creating root (level 0) command "demo" without a handler */
SHELL_CMD_REGISTER(esp32, &sub_test, "base commands", NULL);
