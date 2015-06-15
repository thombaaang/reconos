#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_cint.h"

#define LINE_LEN_WORD 128
#define LINE_LEN_BYTE (LINE_LEN_WORD * 4)

THREAD_ENTRY() {
	THREAD_INIT();
	RAM(uint32, LINE_LEN_WORD, line);

	while (1) {
		uint32 addr = MBOX_GET(write_cmd);
		uint32 count = MBOX_GET(write_cmd);

		for (int i = 0; i < count; i++) {
			PIPE_READ(s2_out, line, LINE_LEN_BYTE);
			MEM_WRITE(line, addr, LINE_LEN_BYTE);
			addr += LINE_LEN_BYTE;
		}

		MBOX_PUT(write_cmd, addr);
	}
}