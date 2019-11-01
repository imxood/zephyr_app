#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* Enable Log Color */
#define SHELL_WITH_COLOR 1


/* Format Log Output */
#if SHELL_WITH_COLOR

#define LIGHT_RED_COLOR                 "\033[91m"
#define LIGHT_GREEN_COLOR               "\033[92m"
#define LIGHT_YELLOW_COLOR              "\033[93m"
#define LIGHT_CYAN_COLOR                "\033[96m"
#define COLOR_END                       "\033[0m"

#define printk_color(color, format_string, ...) printk(color format_string COLOR_END, ##__VA_ARGS__)

#else

#define printk_color(color, format_string, ...) printk(format_string, ##__VA_ARGS__)

#endif


/* simple Without "\n" */
#define _LOG_S_STATUS(msg, ...)   printk_color(LIGHT_CYAN_COLOR, msg, ##__VA_ARGS__)
#define _LOG_S_INFO(msg, ...)     printk(msg, ##__VA_ARGS__)
#define _LOG_S_ERROR(msg, ...)    printk_color(LIGHT_RED_COLOR, msg, ##__VA_ARGS__)

/* simple With "\n" */
#define LOG_S_STATUS(msg, ...)   _LOG_S_STATUS(msg "\n", ##__VA_ARGS__)
#define LOG_S_INFO(msg, ...)     _LOG_S_INFO(msg "\n", ##__VA_ARGS__)
#define LOG_S_ERROR(msg, ...)    _LOG_S_ERROR(msg "\n", ##__VA_ARGS__)

/* With "\n" and With __FILE__, __func__,  __LINE__ */
#define LOG_STATUS(msg, ...)     printk_color(LIGHT_CYAN_COLOR, "[%d: %s] " msg "\n", __LINE__, __func__, ##__VA_ARGS__)
#define LOG_INFO(msg, ...)       printk("[%d: %s] " msg "\n", __LINE__, __func__, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)      printk_color(LIGHT_RED_COLOR, "[%d: %s]\n" msg "\n", __LINE__, __func__, ##__VA_ARGS__)

#define LOG_RESULT(msg, ...)     printk_color(LIGHT_GREEN_COLOR, "[%d: %s] " msg "\n", __LINE__, __func__, ##__VA_ARGS__)


void hexdump(const uint8_t* buf, int len);
