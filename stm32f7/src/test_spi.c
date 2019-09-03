#include <zephyr.h>
#include <spi.h>
#include <device.h>
#include <logging/log.h>
#include <shell/shell.h>

LOG_MODULE_REGISTER(stm32_spi, 4);

#define SPI_MODE_0 ((0 << 1) | (0 << 2))
#define SPI_MODE_1 (SPI_MODE_CPOL | (0 << 2))
#define SPI_MODE_2 ((0 << 1) | SPI_MODE_CPHA)
#define SPI_MODE_3 (SPI_MODE_CPOL | SPI_MODE_CPHA)

#define FLASH_READ (0x03)

#define FM25_WREN 0x06
#define FM25_WRDI 0x04
#define FM25_RDSR 0x05
#define FM25_WRSR 0x01
#define FM25_READ 0x03
#define FM25_FSTRD 0x0b
#define FM25_WRITE 0x02
#define FM25_SLEEP 0xb9
#define FM25_RDID 0x9f
#define FM25_SNR 0xc3

#define SPI_DEV ("SPI_2")
#define BUF_LEN (512)

const u32_t SPI_SPEED[7] = {
	500000, /*200 KHZ*/
	400000, /*400 KHZ*/
	1000000, /*1   MHZ*/
	4000000, /*4   MHZ*/
	10000000, /*10  MHZ*/
	25000000, /*25  MHZ*/
	40000000 /*40  MHZ*/
};

int result = -1;

u8_t buffer_print_tx[BUF_LEN * 5 + 1];
u8_t buffer_print_rx[BUF_LEN * 5 + 1];
u8_t buffer_tx[BUF_LEN];
u8_t buffer_rx[BUF_LEN];
u8_t _buffer_write[BUF_LEN + 4];
u8_t _buffer_read[BUF_LEN + 4];
u8_t _buffer_rx[BUF_LEN + 4];
u8_t flash_enable[1] = { FM25_WREN };

#define WRITE_BUF_LEN 128
#define READ_BUF_LEN 128

u8_t buffer_write[WRITE_BUF_LEN];
u8_t buffer_read[READ_BUF_LEN];

static void to_display_format(const u8_t *src, size_t size, char *dst)
{
	size_t i;

	for (i = 0; i < size; i++) {
		sprintf(dst + 5 * i, "0x%02x,", src[i]);
	}
}

int test_spi_data_read_write(void)
{
	int ret = 0;

	// get the spi device
	struct device *spi_dev = device_get_binding(SPI_DEV);

	if (NULL == spi_dev) {
		LOG_INF("Cannot get SPI device: %s\n", SPI_DEV);
		return -1;
	}

	// spi config
	struct spi_config spi_cfg = {
		.frequency = SPI_SPEED[1],
		.operation = SPI_OP_MODE_MASTER | SPI_MODE_3 |
			     SPI_TRANSFER_MSB | SPI_WORD_SET(8) |
			     SPI_LINES_SINGLE,
		.slave = 0,
		.cs = NULL,
	};

	// write data

	buffer_write[0] = FM25_WREN;
	buffer_write[1] = FM25_WRITE;
	buffer_write[2] = 0;
	buffer_write[3] = 0;
	buffer_write[4] = 0;

	for (int i = 5; i < WRITE_BUF_LEN - 1; i++) {
		buffer_write[i] = i;
	}

	buffer_write[WRITE_BUF_LEN - 1] = FM25_WRDI;

	struct spi_buf write_buf[] = { {
		.buf = buffer_write,
		.len = WRITE_BUF_LEN,
	} };

	const struct spi_buf_set write_cmd = { .buffers = write_buf,
					       .count = ARRAY_SIZE(write_buf) };

	ret = spi_transceive(spi_dev, &spi_cfg, &write_cmd, NULL);

	if (ret) {
		LOG_INF("SPI transceive failed, Error Code %d\n", ret);
		return -1;
	}

	LOG_INF("write data\n");

	// read data
	memset(buffer_read, 0, READ_BUF_LEN);

	return 0;
}

int test_spi_transfer(const char *spi_name, u32_t speed, u8_t spi_mode,
		      u8_t bit_order, u8_t framesize, u16_t buf_len,
		      u8_t cs_gpio)
{
	int i;
	int ret = -1;

	LOG_INF("SPI controller: %s, Speed: %d kHz, Spi mode: %d, Bit order: %d, Framesize: %d, Buffer_len: %d, CS_GPIO: %d\n",
	       spi_name, speed / 1000, spi_mode, bit_order, framesize, buf_len,
	       cs_gpio);

	memset(buffer_rx, 0, buf_len);
	memset(_buffer_write, 0, buf_len + 4);
	memset(_buffer_read, 0, buf_len + 4);
	memset(_buffer_rx, 0, buf_len + 4);

	for (i = 0; i < buf_len; i++) {
		buffer_tx[i] = buf_len - 1 - i;
	}

	_buffer_write[0] = FM25_WRITE;
	_buffer_write[1] = 0;
	_buffer_write[2] = 0;
	_buffer_write[3] = 0;

	_buffer_read[0] = FLASH_READ;
	_buffer_read[1] = 0;
	_buffer_read[2] = 0;
	_buffer_read[3] = 0;

	for (i = 0; i < buf_len; i++) {
		_buffer_write[i + 4] = buffer_tx[i];
	}

	struct device *spi_dev = device_get_binding(spi_name);

	if (NULL == spi_dev) {
		// LOG_INF("Cannot get SPI device: %s\n", spi_name);
		return 0;
	}

	struct spi_config spi_cfg = {
		.frequency = speed,
		.operation = SPI_OP_MODE_MASTER | spi_mode | bit_order |
			     SPI_WORD_SET(framesize) | SPI_LINES_SINGLE,
		.slave = 0,
		.cs = NULL,
	};

	struct spi_cs_control spi_cs = {
		.gpio_pin = 0,
		.delay = 0,
	};

	if (cs_gpio) {
		LOG_INF("Set CS_GPIO controller\n");

		spi_cs.gpio_dev = device_get_binding("GPIO_0");
		if (!spi_cs.gpio_dev) {
			LOG_INF("Cannot find GPIO_0!\n");
			return 0;
		} else {
			LOG_INF("Get GPIO device\n");
		}

		spi_cfg.cs = &spi_cs;

	} else {
		spi_cfg.cs = NULL;
	}

	const struct spi_buf enable_bufs[] = {
		{
			.buf = flash_enable,
			.len = 1,
		},
	};

	const struct spi_buf write_bufs[] = {
		{
			.buf = _buffer_write,
			.len = buf_len + 4,
		},
	};

	const struct spi_buf read_bufs[] = {
		{
			.buf = _buffer_read,
			.len = 4,
		},
	};

	const struct spi_buf rx_bufs[] = {
		{
			.buf = _buffer_rx,
			.len = buf_len + 4,
		},
	};

	const struct spi_buf_set enable_cmd = {
		.buffers = enable_bufs, .count = ARRAY_SIZE(enable_bufs)
	};
	const struct spi_buf_set write_cmd = { .buffers = write_bufs,
					       .count =
						       ARRAY_SIZE(write_bufs) };
	const struct spi_buf_set read_cmd = { .buffers = read_bufs,
					      .count = ARRAY_SIZE(read_bufs) };
	const struct spi_buf_set rx = { .buffers = rx_bufs,
					.count = ARRAY_SIZE(rx_bufs) };

	// Enable flash device
	LOG_INF("Enable Flash\n");
	ret = spi_transceive(spi_dev, &spi_cfg, &enable_cmd, NULL);

	if (ret) {
		LOG_INF("SPI transceive failed, Error Code %d\n", ret);
		return 0;
	}

	LOG_INF("Write FLASH\n");
	// Write [write cmd + addr + txBuf]
	ret = spi_transceive(spi_dev, &spi_cfg, &write_cmd, NULL);
	if (ret) {
		LOG_INF("SPI transceive failed, Error Code %d\n", ret);
		return 0;
	}

	LOG_INF("Read FLASH\n");

	// Read [read cmd + addr + txBuf]
	ret = spi_transceive(spi_dev, &spi_cfg, &read_cmd, &rx);
	if (ret) {
		LOG_INF("SPI transceive failed, Error Code %d\n", ret);
		return 0;
	}

	for (i = 0; i < buf_len; i++) {
		if (_buffer_write[i + 4] != _buffer_rx[i + 4]) {
			LOG_INF("Read %d value incorrect, expected is %d, but is %d\n",
			       i, _buffer_write[i + 4], _buffer_rx[i + 4]);
		}
	}

	if (memcmp(_buffer_write + 4, _buffer_rx + 4, buf_len)) {
		to_display_format(_buffer_write + 4, buf_len, buffer_print_tx);
		to_display_format(_buffer_rx + 4, buf_len, buffer_print_rx);
		LOG_INF("Buffer contents are different: %s\n", buffer_print_tx);
		LOG_INF("                           vs: %s\n", buffer_print_rx);
		LOG_INF("Buffer contents are different\n");
		return 0;
	}

	return 0;
}

int spi_callback(const struct shell *shell, size_t argc, char **argv)
{
	if (test_spi_transfer(SPI_DEV, SPI_SPEED[0], SPI_MODE_0, SPI_TRANSFER_MSB, 8, 128, 0)) {
		LOG_INF("test_spi_transfer failed\n");
	} else {
		LOG_INF("test_spi_transfer successed\n");
	}

    LOG_INF("\nargc: %d\n", argc);

    for(int i=0; i<argc; i++){
        LOG_INF("argv[%d]: %s\t", i, argv[i]);
    }

    LOG_INF("\n");

	return 0;
}

SHELL_CREATE_STATIC_SUBCMD_SET(sub_test){
	SHELL_CMD(transfer, NULL, "test spi transfer.", spi_callback),
	SHELL_CMD_ARG(transfer_argv, NULL, "test spi transfer argv.", spi_callback, 2, 1),
    SHELL_SUBCMD_SET_END
};

/* Creating root (level 0) command "demo" without a handler */
SHELL_CMD_REGISTER(spi, &sub_test, "spi commands", NULL);

// 42264
// 11132

// 42264
// 11132
