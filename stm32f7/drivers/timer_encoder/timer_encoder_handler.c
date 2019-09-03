
#include <timer_encoder.h>
#include <syscall_handler.h>

Z_SYSCALL_HANDLER(encoder_start, dev)
{
	return z_impl_encoder_start((struct device *)dev);
}

Z_SYSCALL_HANDLER(encoder_stop, dev)
{
	return z_impl_encoder_stop((struct device *)dev);
}

Z_SYSCALL_HANDLER(encoder_getCounter, dev)
{
	return z_impl_encoder_getCounter((struct device *)dev);
}

Z_SYSCALL_HANDLER(encoder_setPwm, dev, wheel, pwm_a, pwm_b)
{
	return z_impl_encoder_setPwm((struct device *)dev, wheel, pwm_a, pwm_b);
}
