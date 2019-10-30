#include <stdio.h>
#include <stdint.h>
#include <helper.h>

void hexdump(const uint8_t* buf, int len)
{
	if (len < 1 || buf == NULL) return;

	const char *hexChars = "0123456789ABCDEF";
	int i = 0;
	char c = 0x00;
	char str_print_able[17];
	char str_hex_buffer[16 * 3 + 1];

	for (i = 0; i < (len / 16) * 16; i += 16)
	{
		int j = 0;
		for (j = 0; j < 16; j++)
		{
			c = buf[i + j];

			// hex
			int z = j * 3;
			str_hex_buffer[z++] = hexChars[(c >> 4) & 0x0F];
			str_hex_buffer[z++] = hexChars[c & 0x0F];
			str_hex_buffer[z++] = (j < 10 && !((j + 1) % 8)) ? '_' : ' ';

			// string with space repalced
			if (c < 32 || c == '\0' || c == '\t' || c == '\r' || c == '\n' || c == '\b')
				str_print_able[j] = '.';
			else
				str_print_able[j] = c;
		}
		str_hex_buffer[16 * 3] = 0x00;
		str_print_able[j] = 0x00;

		printk("%04x  %s %s\n", i, str_hex_buffer, str_print_able);
	}

	// 处理剩下的不够16字节长度的部分
	int leftSize = len % 16;
	if (leftSize < 1) return;
	int j = 0;
	int pos = i;
	for (; i < len; i++)
	{
		c = buf[i];

		// hex
		int z = j * 3;
		str_hex_buffer[z++] = hexChars[(c >> 4) & 0x0F];
		str_hex_buffer[z++] = hexChars[c & 0x0F];
		str_hex_buffer[z++] = ' ';

		// string with space repalced
		if (c < 32 || c == '\0' || c == '\t' || c == '\r' || c == '\n' || c == '\b')
			str_print_able[j] = '.';
		else
			str_print_able[j] = c;
		j++;
	}
	str_hex_buffer[leftSize * 3] = 0x00;
	str_print_able[j] = 0x00;

	for (j = leftSize; j < 16; j++)
	{
		int z = j * 3;
		str_hex_buffer[z++] = ' ';
		str_hex_buffer[z++] = ' ';
		str_hex_buffer[z++] = ' ';
	}
	str_hex_buffer[16 * 3] = 0x00;
	printk("%04x  %s %s\n", pos, str_hex_buffer, str_print_able);
}
