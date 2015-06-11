#include "reconos_thread.h"
#include "reconos_calls.h"

#include <stdio.h>

#define COUNT_WORD 8
#define COUNT_BYTE (COUNT_WORD * 4)

void *rt_eval(void *data) {
	int i;
	uint32_t cmd;
	uint32_t buf[COUNT_WORD];

	THREAD_INIT();

	while (1) {
		for (i = 0; i < COUNT_WORD; i++) {
			buf[i] = 0xaffe0000 + i;
		}

		cmd = MBOX_GET(eval_cmd);
		switch (cmd) {
			case 0x00000000:
				printf("Writing: ");
				for (i = 0; i < COUNT_WORD; i++) {
					printf("0x%x ", buf[i]);
				}
				printf("\n");
				PIPE_WRITE(eval_pipe, (void *)buf, COUNT_BYTE);
				break;

			case 0x00000001:
				PIPE_READ(eval_pipe, (void *)buf, COUNT_BYTE);
				printf("Read: ");
				for (i = 0; i < COUNT_WORD; i++) {
					printf("0x%x ", buf[i]);
				}
				printf("\n");
				break;

			default:
				break;
		}
	}

	THREAD_EXIT();
}
