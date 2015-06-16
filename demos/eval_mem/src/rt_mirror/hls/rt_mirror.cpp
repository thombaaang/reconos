#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_cint.h"

#define LINE_LEN_WORD 128
#define LINE_LEN_BYTE (LINE_LEN_WORD * 4)

THREAD_ENTRY() {
	THREAD_INIT();
	RAM(uint32, LINE_LEN_WORD, line);

	uint32 init = GET_INIT_DATA();
	uint32 in_addr;
	uint32 out0_addr = init;
	uint32 out1_addr = init + LINE_LEN_BYTE;
	uint32 out_addr;
	bool toggle = 0;

	while(1) {
		in_addr = MBOX_GET(s1_out);
		out_addr = toggle ? out0_addr : out1_addr;

		MEM_READ(in_addr, line, LINE_LEN_BYTE);

		int i;
		for (i = 0; i < LINE_LEN_WORD / 2; i++) {
			uint32 tmp = line[i];
			line[i] = line[LINE_LEN_WORD - i - 1];
			line[LINE_LEN_WORD - i - 1] = tmp;
		}

		MEM_WRITE(line, out_addr, LINE_LEN_BYTE);
		MBOX_PUT(s2_out, out_addr);

		toggle = toggle ? 0 : 1;
	}
}