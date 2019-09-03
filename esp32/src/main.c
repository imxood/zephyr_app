#include "main.h"

#include <zephyr.h>
#include <sys/printk.h>

extern void test_section();

void main(void)
{
	// test_section();

	while(1) {
		printk("Hello World! %s\n", CONFIG_BOARD);
		k_sleep(5000);
	}
}
