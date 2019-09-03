
#include <test_thread.h>
#include <shell/shell.h>

static int test_thread(const struct shell *shell, size_t argc, char **argv)
{
	thread_main();
    return 0;
}

SHELL_CREATE_STATIC_SUBCMD_SET(sub_test){
	SHELL_CMD(test, NULL, "test thread.", test_thread),
    SHELL_SUBCMD_SET_END
};

/* Creating root (level 0) command "demo" without a handler */
SHELL_CMD_REGISTER(thread, &sub_test, "thread test", NULL);
