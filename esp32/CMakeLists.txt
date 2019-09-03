cmake_minimum_required(VERSION 3.13.1)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(base)

FILE(
	GLOB_RECURSE app_sources
	src/main.c
	# src/test_section.c
)

target_sources(app PRIVATE ${app_sources})

zephyr_linker_sources(SECTIONS link/custom-sections.ld)
