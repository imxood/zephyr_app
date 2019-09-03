#include <device.h>
#include <logging/log.h>
#include <pwm.h>
// #include <misc/dlist.h>
#include <soc.h>

LOG_MODULE_REGISTER(APP_PWM1, 4);

#define PWM_DEVICE_NAME	"PWM_2"

/* in micro second */
#define MIN_PERIOD	0x100

/* in micro second */
#define MAX_PERIOD	0xffff


void task_1(void *p1, void *p2, void *p3)
{
#if CONFIG_THREAD_NAME
	k_thread_name_set(k_current_get(), "APP_WHEEL_TASK");
#endif

	struct device *pwm_dev;
	u32_t period = MAX_PERIOD;
	u8_t dir = 0U;

	pwm_dev = device_get_binding(PWM_DEVICE_NAME);
	if (!pwm_dev) {
		LOG_ERR("Cannot find %s!\n", PWM_DEVICE_NAME);
		return;
	} else {
		LOG_INF("find device: %s", PWM_DEVICE_NAME);
	}

	while (1) {
		if (pwm_pin_set_cycles(pwm_dev, 2, MAX_PERIOD, 0)) {
			LOG_ERR("pwm pin set fails\n");
			return;
		} else {
			LOG_INF("period: %d, pulse: %d", period, period / 2);
		}
		if (pwm_pin_set_cycles(pwm_dev, 1, period, period / 2)) {
			LOG_ERR("pwm pin set fails\n");
			return;
		} else {
			LOG_INF("period: %d, pulse: %d", period, period / 2);
		}
		if (dir) {
			period *= 2;

			if (period > MAX_PERIOD) {
				dir = 0U;
				period = MAX_PERIOD;
			}
		} else {
			period /= 2;

			if (period < MIN_PERIOD) {
				dir = 1U;
				period = MIN_PERIOD;
			}
		}

		k_sleep(1000);
	}
}

K_THREAD_DEFINE(thread_1, 512, task_1, NULL, NULL, NULL, 0, 0, 2000);
