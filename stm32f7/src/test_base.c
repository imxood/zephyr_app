#include <zephyr.h>
#include <shell/shell.h>
#include <logging/log.h>

// #define LOG_LEVEL LOG_LEVEL_ERROR
LOG_MODULE_REGISTER(base, 4);

int test_large_small_end(void)
{
    uint32_t a1 = 0x01020304;

    uint8_t *p1 = (uint8_t *)(&a1);
    LOG_DBG("value: 0x01020304, *p1: %u, *(p1+1): %u, *(p1+2): %u, *(p1+3): %u", *p1, *(p1+1), *(p1+2), *(p1+3));

    uint64_t a2 = 0x0102030405060708;

    uint8_t *p2_1 = (uint8_t *)(&a2);
    LOG_DBG("value: 0x0102030405060708, by 1 byte: *p2_1: %u, *(p2_1+1): %u, *(p2_1+2): %u, *(p2_1+3): %u, *(p2_1+4): %u, *(p2_1+5): %u, *(p2_1+6): %u, *(p2_1+7): %u", *p2_1, *(p2_1+1), *(p2_1+2), *(p2_1+3), *(p2_1+4), *(p2_1+5), *(p2_1+6), *(p2_1+7));

    uint32_t *p2_2 = (uint32_t *)(&a2);
    LOG_DBG("value: 0x0102030405060708, by 4 bytes: *p2_2: %x, *(p2_2+1): %x", *p2_2, *(p2_2+1));

    return 0;
}

static int test(const struct shell *shell, size_t argc, char **argv)
{
    if(test_large_small_end()){
        LOG_DBG("spi failed");
    }

    return 0;
}

SHELL_CREATE_STATIC_SUBCMD_SET(sub_test){
	SHELL_CMD(test_base, NULL, "test base.", test),
    SHELL_SUBCMD_SET_END
};

/* Creating root (level 0) command "demo" without a handler */
SHELL_CMD_REGISTER(base, &sub_test, "base commands", NULL);
