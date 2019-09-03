#include <zephyr.h>
#include <device.h>
#include <shell/shell.h>
#include <spi.h>
#include <helper.h>

LOG_MODULE_REGISTER(test_esp32_uart, 4);

#define UART_DEV ("UART_1")

#define WRITE_BUF_LEN 128
#define READ_BUF_LEN 128

// static u8_t buffer_write[WRITE_BUF_LEN];
// static u8_t buffer_read[READ_BUF_LEN];

void esp32_init()
{
	struct device* uart_dev = device_get_binding(UART_DEV);

	if (uart_dev) {
		LOG_INF("Executing device_get_binding successed");
	}
	else {
		LOG_ERR("Executing device_get_binding failed");
		return;
	}

}
void task_1(void *p1, void *p2, void *p3)
{
	esp32_init();
}

K_THREAD_DEFINE(thread_1, 512, task_1, NULL, NULL, NULL, 0, 0, 1000);

