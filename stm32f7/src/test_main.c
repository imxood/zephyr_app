/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/log.h>
#include <ztest.h>

#include <test_thread.h>
#include <test_spi.h>
#include <test_base.h>

void test_main(void)
{
	ztest_test_suite(test_suite, ztest_unit_test(test_spi), ztest_unit_test(test_base)/* ,
			 ztest_unit_test(test_thread) */);

	ztest_run_test_suite(test_suite);
}
