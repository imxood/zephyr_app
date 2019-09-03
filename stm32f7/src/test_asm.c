/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/log.h>
#include <soc.h>

LOG_MODULE_REGISTER(asm);

void t1(void *p1, void *p2, void *p3)
{
	// Each asm statement is devided by colons into up to four parts:
	// 1. The assembler instructions, defined as a single string constant:
	// 2. A list of output operands, separated by commas. Our example uses just one:
	// 3. A comma separated list of input operands. Again our example uses one operand
	// 4. Clobbered registers, left empty in our example.

	/* example 1 */

	int result = 0, value = 0x10;
	LOG_INF("init: value: %d, result: %d, opt: result=(value>>2)", value, result);
	__asm__("mov %0, %1, ror #2" : "=r" (result) : "r" (value));

	// ldr r3, [sp, #0]
	// mov r3, r3, ror #1
	// str r3, [sp, #4]

	LOG_INF("after opt: value: %d, result: %d", value, result);

	// In the code section, operands are referenced by a percent sign followed by a single digit.
	// %0 refers to the first %1 to the second operand and so forth. From the above example:%0 refers to "=r" (result) and
	// %1 refers to "r" (value)


	/* example 2 */

	int a = 10, b = 20, c = 0;

	LOG_INF("init: %d, b: %d, c: %d", a, b, c);

	__asm__(
		"adds %0, %1\t"
		: "=r"(c)
		: "r"(a), "r"(b)
	);

	LOG_INF("after opts: %d, b: %d, c: %d", a, b, c);



	/* example 3 */

	int DataIn = 2, DataOut = 0;

	LOG_INF("init: DataIn: %d, DataOut: %d", DataIn, DataOut);

	__asm__(
		"mov r0, %0\n"
		"mov r3, #5\n"
		"udiv r0, r0, r3\n"
		"mov %1, r0\n"
		: "=r"(DataOut)
		: "r"(DataIn)
		: "cc", "r3"
	);

	/* 在这个代码中,输输入参数是一个C变量,名为DataIn(%0代表第一个参数),该代码把
结果返回到另外一个C变量DataOut中(%1表示第2个参数)。内联汇编的代码还手工修改了
寄存器r3,并且修改了条件标志cc,因此它们被列在被破坏的(clobbered)寄存器列表中。   */

	LOG_INF("after opts: DataIn: %d, DataOut: %d", DataIn, DataOut);
}

K_THREAD_DEFINE(asm_thread, 512, t1, NULL, NULL, NULL, 0, K_ESSENTIAL, K_NO_WAIT);
