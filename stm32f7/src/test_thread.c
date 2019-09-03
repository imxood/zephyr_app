
#include <test_thread.h>
#include <logging/log.h>
#include <misc/dlist.h>
#include <soc.h>

LOG_MODULE_REGISTER(TEST_THREAD);

void task_1(void *p1, void *p2, void *p3);

#define STACK_SIZE 512

K_THREAD_DEFINE(thread_1, STACK_SIZE, task_1, NULL, NULL, NULL, 0, 0, K_NO_WAIT);

void task_1(void *p1, void *p2, void *p3)
{
    #if CONFIG_THREAD_NAME
        k_thread_name_set(k_current_get(), "task_5");
    #endif

    ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	sys_dnode_t *dnode;
	k_tid_t thread;
    k_tid_t current_thread;
    uint8_t i;

	while (1) {
		current_thread = k_current_get();

		i = 0;

        /* LOG_INF("%p: current_thread", current_thread); */

		/* SYS_DLIST_FOR_EACH_NODE (&(current_thread->base.qnode_dlist), dnode) {
			thread = (k_tid_t)dnode;
			LOG_INF("the running thread:");
			LOG_INF("%2d: thread address: %p", i, thread);
			if (thread->name) {
				LOG_INF("  thread name: %s", thread->name);
			}
            i++;
		} */

		k_sleep(2000);
    }
}

void thread_main()
{
	LOG_INF("0x%x: thread main start\n", (uint32_t)k_current_get());

	LOG_INF("0x%x: thread main end\n", (uint32_t)k_current_get());
}
