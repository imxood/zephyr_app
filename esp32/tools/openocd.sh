#!/bin/bash

# ~/programs/openocd-esp32/bin/openocd -f interface/jlink.cfg -f board/esp-wroom-32.cfg

# flash
~/programs/openocd-esp32/bin/openocd -f interface/jlink.cfg -f board/esp-wroom-32.cfg -c "program_esp32 ~/develop/sources/zephyrproject/zephyr/zephyr_output/zephyr/zephyr.bin 0x10000 verify exit"
