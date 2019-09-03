#include <zephyr.h>
#include <sys/printk.h>

int __attribute__ ((section(".test_int"))) test_int = 1;
char *test_str = "Hello, the world~";

void main(void)
{
    while(1)
    {
        printk("Hello, %s~", CONFIG_BOARD);
        printk("%s, %d\n", test_str, test_int);
        k_sleep(5000);
    }
}
