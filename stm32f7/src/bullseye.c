#include <stdint.h>

__attribute__((__section__(".bcovbss"))) uint8_t bcovbss[3072];
__attribute__((__section__(".bcovcon"))) static uint8_t bcovcon[1024];
