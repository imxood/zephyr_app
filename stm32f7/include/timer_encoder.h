#pragma once

#include <device.h>

#include <stm32f7xx_ll_tim.h>
#include <stm32f7xx_ll_gpio.h>

#define ENCODER_NAME_1	CONFIG_SMALL_CAR_ENCODER_NAME
#define PWM_PSC			CONFIG_SMALL_CAR_ENCODER_PSC
#define PWM_ARR			CONFIG_SMALL_CAR_ENCODER_ARR

#ifdef __cplusplus
extern "C" {
#endif


/* -----------------------define car wheels----------------------- */

enum wheel_type {
	WheelLeftFront = 0,
	WheelLeftBack,
	WheelRightFront,
	WheelRightBack
};

typedef enum wheel_type wheel_t;

/** -----------------------PWM Encoder driver API definition---------------------- */
typedef int (*encoder_start_t)();
typedef int (*encoder_stop_t)();
typedef uint32_t (*encoder_get_counter_t)();
typedef int (*encoder_setPwm_t)(wheel_t wheel, uint16_t pwm_a, uint16_t pwm_b);

struct encoder_api_t {
	encoder_start_t start;
	encoder_stop_t stop;
	encoder_get_counter_t getCounter;
	encoder_setPwm_t setPwm;
};


/* -----------------------define syscalls----------------------- */

__syscall int encoder_start(struct device *dev);

static inline int z_impl_encoder_start(struct device *dev)
{
	const struct encoder_api_t *api = (const struct encoder_api_t *)dev->driver_api;
	return api->start();
}

__syscall int encoder_stop(struct device *dev);

static inline int z_impl_encoder_stop(struct device *dev)
{
	const struct encoder_api_t *api = (const struct encoder_api_t *)dev->driver_api;
	return api->stop();
}

__syscall uint32_t encoder_getCounter(struct device *dev);

static inline uint32_t z_impl_encoder_getCounter(struct device *dev)
{
	const struct encoder_api_t *api = (const struct encoder_api_t *)dev->driver_api;
	return api->getCounter();
}

__syscall int encoder_setPwm(const struct device* dev, wheel_t wheel, uint16_t pwm_a, uint16_t pwm_b);

static inline int z_impl_encoder_setPwm(const struct device* dev, wheel_t wheel, uint16_t pwm_a, uint16_t pwm_b)
{
	const struct encoder_api_t *api = (const struct encoder_api_t *)dev->driver_api;
	return api->setPwm(wheel, pwm_a, pwm_b);
}

#ifdef __cplusplus
}
#endif

#include <syscalls/timer_encoder.h>
