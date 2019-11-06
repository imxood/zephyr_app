cmake_minimum_required(VERSION 3.13.1)

set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/dts/stm32f7.overlay.dts")

include_directories(include)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(base)

include(cmake/common.cmake)

if(CONFIG_CUSTOM_CONFIG)
	set(ZEPHYR_RUNNER_CONFIG_BOARD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/board" CACHE STRING "Board definition directory" FORCE)
endif()

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_FULL_LL_DRIVER")
# add_definitions(-DUSE_FULL_LL_DRIVER)

include_directories(common/inc)

add_sources(
	common/src/helper.c
	common/src/init_log.c
	src/main.c
	src/led.c
	src/i2c_24c02.c
	src/oled.c
	# src/test_section.c
	# src/test_uart.c
	# tests/kernel/mem_protect/userspace/*.c
	# src/encoder_run.c
	# src/test_esp32_uart.c
	# drivers/wifi/esp32_core.c
	# src/test_esp32_spi.c
	# src/test_pwm.c
	# src/device_list.c
	# src/test_spi.c
	# src/test_base.c
	# src/test_thread.c
	# src/test_thread_wrap.c
	# src/test_asm.c
)

add_glob_sources_if(CONFIG_TEST_BME280 tests/drivers/bme280/*.c)

add_glob_sources_if(
	CONFIG_TEST_GPIO
	tests/drivers/gpio/*.c
)

zephyr_linker_sources(SECTIONS link/custom-sections.ld)
