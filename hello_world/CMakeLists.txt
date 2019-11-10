# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(hello_world)

message(STATUS "ZEPHYR_PREBUILT_EXECUTABLE: ${ZEPHYR_PREBUILT_EXECUTABLE}")

# zephyr_sources(bullseye/libcov-userDefined-small.c)

target_sources(app PRIVATE src/main.c)

add_library(bullseye OBJECT bullseye/libcov-userDefined-small.c)

# target_link_libraries(zephyr_interface INTERFACE bullseye)
