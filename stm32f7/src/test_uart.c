#include <zephyr.h>
#include <device.h>
#include <init.h>
#include <drivers/uart.h>
#include <shell/shell.h>
#include <logging/log.h>
#include <string.h>

// #define LOG_LEVEL LOG_LEVEL_ERROR
LOG_MODULE_REGISTER(uart, 4);

#define DEVICE_NAME         DT_UART_STM32_USART_6_NAME

static struct device *uart_dev;

typedef struct data_item {
	char *data;
	char data_len;
} data_item_t;

struct {
	data_item_t *buf;
	struct k_fifo fifo;
} rx = {
	.fifo = Z_FIFO_INITIALIZER(rx.fifo)
};

struct {
	data_item_t *buf;
	struct k_fifo fifo;
} tx = {
	.fifo = Z_FIFO_INITIALIZER(tx.fifo)
};

static u8_t recvData[1024];

static inline void process_rx(void)
{
	int count = 0;
	while (1) {
		count = uart_fifo_read(uart_dev, recvData, 1024);
		if (count < 1024) {
			break;
		}
	}
	LOG_INF("received: %d bytes", count);
}

static inline void process_tx(void)
{
	int bytes;

	if (!tx.buf) {
		tx.buf = k_fifo_get(&tx.fifo, K_NO_WAIT);
		if (!tx.buf) {
			LOG_ERR("TX interrupt but no pending buffer!");
			uart_irq_tx_disable(uart_dev);
			return;
		}
	}

	bytes = uart_fifo_fill(uart_dev, tx.buf->data, tx.buf->data_len);

	if (tx.buf->data_len) {
		return;
	}

	tx.buf = k_fifo_get(&tx.fifo, K_NO_WAIT);
	if (!tx.buf) {
		uart_irq_tx_disable(uart_dev);
	}
}

static void uart_isr(struct device *dev)
{
	while (uart_irq_update(dev) && uart_irq_is_pending(uart_dev)) {

		if (uart_irq_tx_ready(uart_dev)) {
			process_tx();
		}
		else if(uart_irq_rx_ready(uart_dev)) {
			process_rx();
		}
	}
}

static void uart_send(char *data, uint32_t length)
{
	data_item_t d = {
		.data = data,
		.data_len = length
	};

	LOG_INF("sended data: %s, length: %d, &d: 0x%x", log_strdup(d.data), d.data_len, &d);
	k_fifo_alloc_put(&tx.fifo, &d);

	// uart_irq_tx_enable(uart_dev);
}

static void rx_thread_entry(void *p1, void *p2, void *p3)
{
	printk("%s\n", __func__);

	data_item_t *p = NULL;
	while (1) {
		p = k_fifo_get(&tx.fifo, K_FOREVER);
		if (p) {
			LOG_INF("recved data: %s, length: %d, p: 0x%x", log_strdup(p->data), p->data_len, p);
		}
		else {
			LOG_INF("recved data: NULL");
		}
	}
}

static int shell_send(const struct shell *shell, size_t argc, char **argv)
{
	uart_send(argv[1], strlen(argv[1]));
	return 0;
}

static int uart_init(struct device *port)
{
	printk("%s\n", __func__);

	uart_dev = device_get_binding(DEVICE_NAME);

	if (uart_dev == NULL) {
		LOG_ERR("can't find device[%s]", DEVICE_NAME);
		return -1;
	}

	uart_irq_rx_disable(uart_dev);
	uart_irq_tx_disable(uart_dev);

	uart_irq_callback_set(uart_dev, uart_isr);

	return 0;
}


SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_test,
	SHELL_CMD_ARG(send, NULL, "test uart send data. usage: uart send DATA", shell_send, 2, 0),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(uart, &sub_test, "base commands", NULL);

SYS_INIT(uart_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

K_THREAD_DEFINE(rx_thread, 1024, rx_thread_entry, NULL, NULL, NULL, CONFIG_APPLICATION_INIT_PRIORITY, 0, 0);
