#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_cint.h"

#define COUNT_WORD 8
#define COUNT_BYTE (COUNT_WORD * 4)

THREAD_ENTRY() {
	THREAD_INIT();
	RAM(uint32, COUNT_WORD, buf);
	uint32 cmd;

	while(1) {
		for (int i = 0; i < COUNT_WORD; i++) {
			buf[i] = 0xDEAD0000 + i;
		}

		cmd = MBOX_GET(eval_cmd);

		if (cmd == 0x00000000) {
			PIPE_WRITE(eval_pipe, buf, COUNT_BYTE);
		} else if (cmd == 0x00000001) {
			PIPE_READ(eval_pipe, buf, COUNT_BYTE);
		} else {
			// unsupported command
		}
	}
}