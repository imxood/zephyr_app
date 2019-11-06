#include <zephyr.h>
#include <device.h>
#include <init.h>
#include <drivers/gpio.h>

// LED0, PB1

#define LED_DEVICE_NAME     DT_GPIO_STM32_GPIOB_LABEL
#define LED_PIN             1

static struct device *led_dev;

void led_shine_function(struct k_timer *timer_id)
{
    static uint8_t status = 0;
    gpio_pin_write(led_dev, LED_PIN, status);
    status = !status;
    printk("system uptime: %dms\n", k_uptime_get_32());
}

K_TIMER_DEFINE(led_shine_timer, led_shine_function, NULL);


void led_service(struct device *unused)
{
    ARG_UNUSED(unused);

    led_dev = device_get_binding(LED_DEVICE_NAME);
    if (led_dev == NULL) {
        printk("can't get gpio device[%s]\n", LED_DEVICE_NAME);
        return;
    }

    gpio_pin_disable_callback(led_dev, LED_PIN);
    gpio_pin_configure(led_dev, LED_PIN, GPIO_DIR_OUT);

    k_timer_start(&led_shine_timer, K_SECONDS(1), 0);
}

SYS_INIT(led_service, APPLICATION, 90);
