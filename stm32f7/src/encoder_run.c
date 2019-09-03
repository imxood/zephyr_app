#include <zephyr.h>
#include <device.h>
#include <logging/log.h>
#include <timer_encoder.h>
#include <soc.h>

#define WHEEL_PWM_MAX	CONFIG_SMALL_CAR_ENCODER_ARR

LOG_MODULE_REGISTER(encoder_run, 4);

static void monitor_task(void *p1, void *p2, void *p3)
{
	struct device* encoder_dev = device_get_binding(CONFIG_SMALL_CAR_ENCODER_NAME);
	if (encoder_dev != NULL) {
		LOG_INF("Executing device_get_binding successed");
	} else {
		LOG_ERR("Executing device_get_binding failed");
		return;
	}
	volatile uint32_t counter = 0;
	while(1) {
		counter = encoder_getCounter(encoder_dev);
		LOG_INF("counter: %u", counter);
		k_sleep(200);
	}
}

static void encode_task(void *p1, void *p2, void *p3)
{
	struct device* encoder_dev = device_get_binding(CONFIG_SMALL_CAR_ENCODER_NAME);
	if (encoder_dev != NULL) {
		LOG_INF("Executing device_get_binding successed");
	} else {
		LOG_ERR("Executing device_get_binding failed");
		return;
	}

	int16_t  pwm_a = 0, pwm_b = CONFIG_SMALL_CAR_ENCODER_PSC;
	int8_t pwm_a_direction = 1, pwm_b_direction = -1;

	while(1) {

		LOG_INF("set pwm_a: %d, pwm_b: %d\n", pwm_a, pwm_b);

		pwm_a = 1000;
		pwm_b = WHEEL_PWM_MAX;

		encoder_setPwm(encoder_dev, 0, pwm_a, pwm_b);

		k_sleep(500);

		pwm_a = 10000;
		pwm_b = WHEEL_PWM_MAX;

		encoder_setPwm(encoder_dev, 0, pwm_a, pwm_b);

		k_sleep(500);

		pwm_a = WHEEL_PWM_MAX;
		pwm_b = 1000;

		encoder_setPwm(encoder_dev, 0, pwm_a, pwm_b);

		k_sleep(500);

		pwm_a = WHEEL_PWM_MAX;
		pwm_b = 10000;

		encoder_setPwm(encoder_dev, 0, pwm_a, pwm_b);

		k_sleep(500);

		pwm_a = WHEEL_PWM_MAX;
		pwm_b = WHEEL_PWM_MAX;

		encoder_setPwm(encoder_dev, 0, pwm_a, pwm_b);

		k_sleep(500);

		// pwm_a += 5000 * pwm_a_direction;

		// if (pwm_a > WHEEL_PWM_MAX) {
		// 	pwm_a_direction = -1;
		// 	pwm_a = WHEEL_PWM_MAX;
		// } else if (pwm_a < 0) {
		// 	pwm_a_direction = 1;
		// 	pwm_a = 0;
		// }

		// pwm_b += 100 * pwm_b_direction;

		// if (pwm_b > WHEEL_PWM_MAX) {
		// 	pwm_b_direction = -1;
		// 	pwm_b = WHEEL_PWM_MAX;
		// } else if (pwm_b < 0) {
		// 	pwm_b_direction = 1;
		// 	pwm_b = 0;
		// }

		// k_sleep(5000);
	}
}

K_THREAD_DEFINE(monitor_thread, 512, monitor_task, NULL, NULL, NULL, 0, 0, K_NO_WAIT);
K_THREAD_DEFINE(encode_thread, 512, encode_task, NULL, NULL, NULL, 0, 0, K_NO_WAIT);
