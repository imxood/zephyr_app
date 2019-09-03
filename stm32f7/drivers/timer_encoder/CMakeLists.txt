
if(CONFIG_SMALL_CAR)
	zephyr_sources(timer_encoder.c)
	zephyr_sources_ifdef(CONFIG_USERSPACE timer_encoder_handler.c)
endif()
