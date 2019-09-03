#pragma once

#include <zephyr.h>
#include <logging/log.h>

#define _STR(a) #a
#define STR(a) _STR(a)

#define TestFunc(CallFunc, _gotoFlag) \
	do { \
		int ret = CallFunc; \
		if (ret == 0) { \
			LOG_INF("Executing %s successed\n", STR(CallFunc)); \
		} else { \
			LOG_ERR("Executing %s failed\n", STR(CallFunc)); \
			goto _gotoFlag; \
		} \
	} while (0)

/* the macro to know where the code run. */
#define FUNC_RUN_FLAG() LOG_INF("\n\t\t***************** Run here *****************\n")

/* the macro to be easy to code in test function's start and end */
#define FUNC_TEST() LOG_INF("\n\t\t***************** [%s test] *****************\n", __func__)
#define FUNC_TEST_START() LOG_INF("\n\t\t***************** {%s test start} *****************\n", __func__)
#define FUNC_TEST_END() LOG_INF("\n\t\t***************** {%s test end} *******************\n", __func__)
