#include <zephyr.h>

__attribute__((__section__(".devconfig.init"))) int test_a = 10;
__attribute__((__section__(".devconfig.init"))) int test_b = 10;

extern int __test_int_start[0];
extern int __test_int_end[0];

void test_section()
{
	int size = __test_int_end - __test_int_start;
	printk("[%s][%d][%s], size: %d, value[0]: %d, value[1]: %d, \n", __FILE__, __LINE__, __func__, size, __test_int_start[0], __test_int_start[1]);
}
// test_int
