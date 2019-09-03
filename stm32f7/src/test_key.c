#include <zephyr.h>
#include <misc/printk.h>
#include <misc/util.h>
#include <device.h>
#include <gpio.h>

#include <power.h>
#include <soc_power.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(stm32f767_key);

#define KEYUP_DEVICE_NAME   "GPIOA"
#define KEYUP_GPIO_PIN      0

/* change to use another GPIO pin interrupt config */
#define EDGE (GPIO_INT_EDGE | GPIO_INT_ACTIVE_LOW)

/* change this to enable pull-up/pull-down */
#define PULL_UP 0

void gpio_callback(struct device *device, struct gpio_callback *cb, uint32_t pins);

struct gpio_callback gpio_cb;

struct k_timer key_scan_timer;

void key_scan_callback(struct k_timer *timer_id);

struct device *gpioa = NULL;

void main(void)
{

    printk("hello, i'm main thread\n");

	int ret = 0;

	gpioa = device_get_binding(KEYUP_DEVICE_NAME);

	if (!gpioa) {
		LOG_ERR("get device \"%s\" failed", KEYUP_DEVICE_NAME);
		return;
	}

    printk("get device \"%s\" success\n", KEYUP_DEVICE_NAME);

	ret = gpio_pin_configure(gpioa, KEYUP_GPIO_PIN, GPIO_DIR_IN | GPIO_INT | PULL_UP | EDGE);

	if (ret) {
		LOG_ERR("gpio configure error");
		return;
	}

    LOG_INF("gpio configure success");


    k_timer_init(&key_scan_timer, key_scan_callback, NULL);

    LOG_INF("timer configure success");

	/* gpio_init_callback(&gpio_cb, gpio_callback, BIT(KEYUP_GPIO_PIN));

	ret = gpio_add_callback(gpioa, &gpio_cb);

	if (ret) {
		LOG_ERR("gpio add callback error\n");
		return;
	}

    LOG_INF("gpio add callback success\n");

	gpio_pin_enable_callback(gpioa, KEYUP_GPIO_PIN); */

    k_timer_start(&key_scan_timer, K_MSEC(2), K_MSEC(2));

}

void gpio_callback(struct device *device, struct gpio_callback *cb, uint32_t pins)
{
    /* start periodic timer that expires once every second */
    /* k_timer_start(&key_scan_timer, K_MSEC(10), K_MSEC(10)); */

    /* if (k_timer_status_get(&key_scan_timer)) {

    } */

	volatile static s32_t last_time = 0;

    u32_t pin_value;

    gpio_pin_read(device, 0, &pin_value);

    LOG_INF("diff time: %d", k_uptime_get_32() - last_time);

    last_time = k_uptime_get_32();

	/* if (k_uptime_get_32() - last_time > 50) {
		// printk("key pressed! uptime: %d, key diff time: %d\n", k_uptime_get_32(), k_uptime_get_32() - last_time);
		last_time = k_uptime_get();
	} else {
		// printk(" < 50");
	} */
}

/* 按键状态检测 */
void key_scan_callback(struct k_timer *timer_id) {

    volatile static u8_t keyUp = 0;
    volatile static u8_t KeyUpStatus;

    volatile static u32_t timeout = 1000; /* 第一次超过时 */
    volatile static u8_t afterTimeout = 0;

    volatile static s32_t last_time = 0;
    volatile static s32_t keyUpPressedTime = 0;

    u32_t pin_value;

    gpio_pin_read(gpioa, 0, &pin_value);
    keyUp = ((keyUp << 1) | pin_value);

    if (keyUp == 0xff) { /* 此次是按下的 */

        if (KeyUpStatus) { /* 如果前一次也是按下, 则累加过去的时间 */
		    keyUpPressedTime += (k_uptime_get_32() - last_time);
	    } else {
		    keyUpPressedTime = 0;
            LOG_INF("pressed");
	    }

	    KeyUpStatus = 1; /* 更新此次的状态 */

        if (keyUpPressedTime >= timeout) { /* 若长按 [afterTimeout] ms */
            if (!afterTimeout) {
                afterTimeout = 1;
                LOG_INF("timeout: %d, keyUpPressedTime: %d", timeout, keyUpPressedTime);
            }
        }

    } else {
        KeyUpStatus = 0;
        afterTimeout = 0;
    }

    last_time = k_uptime_get_32();

}

void sys_pm_notify_lps_entry(enum power_states state)
{
	printk("Entering Low Power state (%d)\n", state);
}

void sys_pm_notify_lps_exit(enum power_states state)
{
	printk("Exiting Low Power state (%d)\n", state);
}