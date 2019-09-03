5/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <kernel.h>
#include <misc/printk.h>
#include <shell/shell.h>
#include <stdio.h>
#include <stdlib.h>

/* size of stack area used by each thread */
#define STACK_SIZE 512

/* scheduling priority used by each thread */
#define PRIORITY 7
static struct k_sem TASKASEM;

K_THREAD_STACK_DEFINE(stak0, STACK_SIZE);
K_THREAD_STACK_DEFINE(stak1, STACK_SIZE);
K_THREAD_STACK_DEFINE(stak2, STACK_SIZE);

void transfer_task_for_multi_thread_1(void *, void *, void *);
void transfer_task_for_multi_thread_2(void *, void *, void *);
void transfer_task_for_multi_thread_3(void *, void *, void *);

/*************************************Shell***************************************************/
#if 1
static int test_ose_wdt_1_cb(const struct shell *shell, size_t argc,
			     char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	printk("\nthis is the shell test: test_ose_wdt_1_cb\n");
	return 0;
}

static int test_print_test(const struct shell *shell, size_t argc, char *argv[])
{
	printk("argc = %d \n", argc);
	for (int j = 0; j < argc; j++) {
		printk("argv[%d] = %s \n", j, argv[j]);
	}
	int i = 0;
	int cycle = atoi(argv[1]);
	while (i < cycle) {
		printk("thread %d--%d \n", cycle, i);
		i++;
	}
	return 0;
}

SHELL_CREATE_STATIC_SUBCMD_SET(test_cmds){
	SHELL_CMD(test_ose_wdt_1_cb, NULL, NULL, test_ose_wdt_1_cb),
	SHELL_CMD_ARG(test_print, NULL, "test_print argv[1] (argv[2]) ",
		      test_print_test, 2, 1),
	SHELL_SUBCMD_SET_END
};
#endif
/************************************multi-thread***************************************************/
#if 1

void transfer_task_for_multi_thread_1(void *argv1, void *argv2, void *argv3)
{
	k_sem_take(&TASKASEM, K_FOREVER);
	ARG_UNUSED(argv1);
	ARG_UNUSED(argv2);
	ARG_UNUSED(argv3);
	printk("\ntransfer_task_for_multi_thread_1 is starting\n");
	int cycle = 0;
	while (cycle < 100) {
		printk("transfer_task_for_multi_thread_1 cycles = %d\n", cycle);
		cycle++;
		k_sleep(10);
	}
	k_sem_give(&TASKASEM);
}

void transfer_task_for_multi_thread_2(void *argv1, void *argv2, void *argv3)
{
	k_sem_take(&TASKASEM, K_FOREVER);
	ARG_UNUSED(argv1);
	ARG_UNUSED(argv2);
	ARG_UNUSED(argv3);
	printk("\ntransfer_task_for_multi_thread_2 is starting\n");
	int cycle = 0;
	while (cycle < 100) {
		printk("transfer_task_for_multi_thread_2 cycles = %d\n", cycle);
		cycle++;
		k_sleep(10);
	}
	k_sem_give(&TASKASEM);
}

void transfer_task_for_multi_thread_3(void *argv1, void *argv2, void *argv3)
{
	k_sem_take(&TASKASEM, K_FOREVER);
	ARG_UNUSED(argv1);
	ARG_UNUSED(argv2);
	ARG_UNUSED(argv3);
	printk("\ntransfer_task_for_multi_thread_3 is starting\n");
	int cycle = 0;
	while (cycle < 100) {
		printk("transfer_task_for_multi_thread_3 cycles = %d\n", cycle);
		cycle++;
		k_sleep(10);
	}
	k_sem_give(&TASKASEM);
}

void test_dma_transfer_multi_thread(void)
{
	printk("main thread start\n");
	struct k_thread thd0, thd1, thd2;
	k_sem_init(&TASKASEM, 0, 1);

	int p = k_thread_priority_get(k_current_get());
	printk("New thread with priority: %d\n", p);
	k_tid_t tid_0 =
		k_thread_create(&thd0, stak0, K_THREAD_STACK_SIZEOF(stak0),
				transfer_task_for_multi_thread_1, NULL, NULL,
				NULL, p, 0, K_NO_WAIT);/*K_NO_WAIT*/

	k_tid_t tid_1 =
		k_thread_create(&thd1, stak1, K_THREAD_STACK_SIZEOF(stak1),
				transfer_task_for_multi_thread_2, NULL, NULL,
				NULL, p, 0, K_NO_WAIT);

	k_tid_t tid_2 =
		k_thread_create(&thd2, stak2, K_THREAD_STACK_SIZEOF(stak2),
				transfer_task_for_multi_thread_3, NULL, NULL,
				NULL, p, 0, K_NO_WAIT);

	ARG_UNUSED(tid_0);
	ARG_UNUSED(tid_1);
	ARG_UNUSED(tid_2);
	k_yield();
	printk("main thread end\n");
	//k_sem_take(&TASKASEM, K_FOREVER);
}
#endif

#if 0
K_THREAD_DEFINE(multi_thread_1, STACK_SIZE, transfer_task_for_multi_thread_1, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(multi_thread_2, STACK_SIZE, transfer_task_for_multi_thread_1, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
K_THREAD_DEFINE(multi_thread_3, STACK_SIZE, transfer_task_for_multi_thread_1, NULL, NULL, NULL,
		PRIORITY, 0, K_NO_WAIT);
#endif

void main(void)
{
	int i = 0;
	/* Register Shell to run TCs that will block fw */
	SHELL_CMD_REGISTER(test, &test_cmds, NULL, NULL);
	while (1) {
		printk("\nHello World! %s, i = %d\n", CONFIG_BOARD, i);
		k_sleep(10000);
		i++;
		if (i == 5) {
			int p = k_thread_priority_get(k_current_get());
			printk("current thread_priority = %d", p);
#if 1
			test_dma_transfer_multi_thread();
#endif
		}
	}
}


