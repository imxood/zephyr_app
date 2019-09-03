
#include <soc.h>
#include <device.h>
#include <kernel.h>
#include <init.h>

#include <logging/log.h>

#include <syscall_handler.h>
#include <timer_encoder.h>

LOG_MODULE_REGISTER(sys, 4);

/**
 * define wheel struct
 */
struct wheel_struct_type {
	wheel_t wheel;
	uint8_t channel_a;
	uint8_t channel_b;
	TIM_TypeDef* tim_def;
};

typedef struct wheel_struct_type wheel_struct_t;


static wheel_struct_t wheels[] = {
	{
		.wheel = WheelLeftFront,
		.tim_def = TIM3,
		.channel_a = 1,
		.channel_b = 2,
	},
	{
		.wheel = WheelLeftBack,
		.tim_def = TIM3,
		.channel_a = 3,
		.channel_b = 4,
	},
	{
		.wheel = WheelRightFront,
		.tim_def = TIM4,
		.channel_a = 1,
		.channel_b = 2,
	},
	{
		.wheel = WheelRightBack,
		.tim_def = TIM4,
		.channel_a = 3,
		.channel_b = 4,
	},
};

static wheel_struct_t *GetWheel(wheel_t wheel)
{
	for (int i=0; i<ARRAY_SIZE(wheels); i++)
	{
		if (wheels[i].wheel == wheel) {
			return wheels + i;
		}
	}
	return NULL;
}

static void timer3_isr(void *arg)
{
	LOG_INF("timer3_isr");
	if (LL_TIM_IsActiveFlag_UPDATE(TIM3)) {
		LL_TIM_ClearFlag_UPDATE(TIM3);
	}
}

static void timer4_isr(void *arg)
{
	LOG_INF("timer4_isr");
	if (LL_TIM_IsActiveFlag_UPDATE(TIM4)) {
		LL_TIM_ClearFlag_UPDATE(TIM4);
	}
}

static void timer3_start(void) {

	irq_connect_dynamic(TIM3_IRQn, 0, timer3_isr, NULL, 0);
	irq_enable(TIM3_IRQn);

	LL_TIM_EnableUpdateEvent(TIM3);

	LL_TIM_SetCounter(TIM3, 0);
	LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2 | LL_TIM_CHANNEL_CH3 | LL_TIM_CHANNEL_CH4);
	LL_TIM_EnableCounter(TIM3);
}

static void timer4_start(void) {

	irq_connect_dynamic(TIM4_IRQn, 0, timer4_isr, NULL, 0);
	irq_enable(TIM4_IRQn);

	LL_TIM_SetCounter(TIM4, 0);
	LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2 | LL_TIM_CHANNEL_CH3 | LL_TIM_CHANNEL_CH4);
	LL_TIM_EnableCounter(TIM4);
}

static void timer3_stop(void) {
	LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2 | LL_TIM_CHANNEL_CH3 | LL_TIM_CHANNEL_CH4);
	LL_TIM_DisableCounter(TIM3);
}

static void timer4_stop(void) {
	LL_TIM_CC_DisableChannel(TIM4, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2 | LL_TIM_CHANNEL_CH3 | LL_TIM_CHANNEL_CH4);
	LL_TIM_DisableCounter(TIM4);
}


static int start()
{
	timer3_start();
	timer4_start();
	LOG_INF("Executing %s() successed", __func__);
	return 0;
}

static int stop()
{
	timer3_stop();
	timer4_stop();
	LOG_INF("Executing %s() successed", __func__);
	return 0;
}

static uint32_t getCounter()
{
	return LL_TIM_GetCounter(TIM3);
}

static int32_t setPwmOutput(TIM_TypeDef *tim_typedef, uint8_t channel, uint16_t pwm)
{
	switch(channel){
		case 1:
			LL_TIM_OC_SetCompareCH1(tim_typedef, pwm);
			break;
		case 2:
			LL_TIM_OC_SetCompareCH2(tim_typedef, pwm);
			break;
		case 3:
			LL_TIM_OC_SetCompareCH3(tim_typedef, pwm);
			break;
		case 4:
			LL_TIM_OC_SetCompareCH4(tim_typedef, pwm);
			break;
		default:
			return -1;
	}
	return 0;
}

static int setPwm(wheel_t wheel, uint16_t pwm_a, uint16_t pwm_b)
{
	int ret = 0;
	if ((pwm_a > PWM_ARR || pwm_a < 0) && (pwm_b > PWM_ARR || pwm_b < 0)) {
		return -1;
	}

	wheel_struct_t *wheel_struct = GetWheel(wheel);

	if(wheel_struct == NULL) {
		LOG_ERR("the wheel[%d] not exist", wheel);
		return -1;
	}

	ret = setPwmOutput(wheel_struct->tim_def, wheel_struct->channel_a, pwm_a);

	if (ret != 0) {
		LOG_ERR("Ecevuting setPwmOutput failed. channel: %d, ret: %d\n", wheel_struct->channel_a, ret);
		return ret;
	}

	ret = setPwmOutput(wheel_struct->tim_def, wheel_struct->channel_b, pwm_b);

	if (ret != 0) {
		LOG_ERR("Ecevuting setPwmOutput failed. channel: %d, ret: %d\n", wheel_struct->channel_b, ret);
		return ret;
	}

	return 0;
}

/* TIM3 init function */
static void MX_TIM3_Init(void) {
	LL_TIM_InitTypeDef TIM_InitStruct = { 0 };
	LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = { 0 };

	LL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* Peripheral clock enable */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

	TIM_InitStruct.Prescaler = 9999;
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload = PWM_ARR;
	TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	LL_TIM_Init(TIM3, &TIM_InitStruct);
	LL_TIM_DisableARRPreload(TIM3);

	LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH1);
	TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
	TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.CompareValue = 1000;
	TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
	LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH1);

	LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH2);
	TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.CompareValue = 3000;
	LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH2);

	LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH3);
	TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.CompareValue = 5000;
	LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH3, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH3);

	LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH4);
	TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.CompareValue = 7000;
	LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH4, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH4);

	LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
	LL_TIM_DisableMasterSlaveMode(TIM3);

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	/**TIM3 GPIO Configuration
	 PA6     ------> TIM3_CH1
	 PA7     ------> TIM3_CH2
	 PB0     ------> TIM3_CH3
	 PB1     ------> TIM3_CH4
	 */
	GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}
/* TIM4 init function */
static void MX_TIM4_Init(void) {
	LL_TIM_InitTypeDef TIM_InitStruct = { 0 };
	LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = { 0 };

	LL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* Peripheral clock enable */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

	TIM_InitStruct.Prescaler = 0;
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload = PWM_ARR;
	TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	LL_TIM_Init(TIM4, &TIM_InitStruct);
	LL_TIM_DisableARRPreload(TIM4);

	LL_TIM_OC_EnablePreload(TIM4, LL_TIM_CHANNEL_CH1);
	TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
	TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.CompareValue = 2000;
	TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
	LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM4, LL_TIM_CHANNEL_CH1);

	LL_TIM_OC_EnablePreload(TIM4, LL_TIM_CHANNEL_CH2);
	TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.CompareValue = 4000;
	LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM4, LL_TIM_CHANNEL_CH2);

	LL_TIM_OC_EnablePreload(TIM4, LL_TIM_CHANNEL_CH3);
	TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.CompareValue = 6000;
	LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH3, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM4, LL_TIM_CHANNEL_CH3);

	LL_TIM_OC_EnablePreload(TIM4, LL_TIM_CHANNEL_CH4);
	TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
	TIM_OC_InitStruct.CompareValue = 8000;
	LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH4, &TIM_OC_InitStruct);
	LL_TIM_OC_DisableFast(TIM4, LL_TIM_CHANNEL_CH4);

	LL_TIM_SetTriggerOutput(TIM4, LL_TIM_TRGO_RESET);
	LL_TIM_DisableMasterSlaveMode(TIM4);

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);

	/**TIM4 GPIO Configuration
	 PD12     ------> TIM4_CH1
	 PD13     ------> TIM4_CH2
	 PD14     ------> TIM4_CH3
	 PD15     ------> TIM4_CH4
	 */
	GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_13;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_14;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

static int init(struct device *dev)
{
	MX_TIM3_Init();
	MX_TIM4_Init();

	start();

	return 0;
}

static struct encoder_api_t encoder_api = {
	.start = start,
	.stop = stop,
	.getCounter = getCounter,
	.setPwm = setPwm,
};

#define ENCODER_DEVICE_INIT_STM32()                                            \
	DEVICE_AND_API_INIT(timer_encoder_1, ENCODER_NAME_1, init, NULL, NULL, \
			    POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,   \
			    &encoder_api);

ENCODER_DEVICE_INIT_STM32();
