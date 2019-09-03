#include <zephyr.h>
#include <device.h>
#include <init.h>

extern struct device __device_init_start[];
extern struct device __device_init_end[];

static int get_device_list(struct device *unused)
{
	ARG_UNUSED(unused);

    int device_count;
    struct device *device_list;

    device_list = __device_init_start;
	device_count = __device_init_end - __device_init_start;

    printk("device count: %d\n", device_count);

    for (int i = 0; i < device_count; i++) {
        printk("device_list[%d].config->name: %s\n", i, device_list[i].config->name);
	}

    printk("\n");

    return 0;
}

SYS_INIT(get_device_list, APPLICATION, 90);
