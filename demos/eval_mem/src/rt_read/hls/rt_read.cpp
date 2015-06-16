#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_cint.h"

#define LINE_LEN_WORD 128
#define LINE_LEN_BYTE (LINE_LEN_WORD * 4)

THREAD_ENTRY() {
	THREAD_INIT();
	RAM(uint32, LINE_LEN_WORD, line);

	uint32 init = GET_INIT_DATA();

	while (1) {
		uint32 addr = MBOX_GET(read_cmd);
		uint32 count = MBOX_GET(read_cmd);

		uint32 out0_addr = init;
		uint32 out1_addr = init + LINE_LEN_BYTE;
		uint32 out_addr;
		bool toggle = 0;

		int i;
		for (i = 0; i < count; i++) {
			out_addr = toggle ? out0_addr : out1_addr;

			MEM_READ(addr, line, LINE_LEN_BYTE);
			MEM_WRITE(line, out_addr, LINE_LEN_BYTE);

			MBOX_PUT(s0_out, out_addr);

			addr += LINE_LEN_BYTE;
			toggle = toggle ? 0 : 1;
		}
	}
}