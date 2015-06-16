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
		in_addr = MBOX_GET(s0_out);
		out_addr = toggle ? out0_addr : out1_addr;

		MEM_READ(in_addr, line, LINE_LEN_BYTE);

		int i;
		for (i = 0; i < LINE_LEN_WORD; i++) {
			uint8 r = apint_get_range(line[i], 31, 24);
			uint8 g = apint_get_range(line[i], 23, 16);
			uint8 b = apint_get_range(line[i], 15, 8);
			uint8 a = apint_get_range(line[i], 7, 0);

			r = r > 204 ? 255 : r * 1.25;
			g = g > 204 ? 255 : g * 1.25;
			b = b > 204 ? 255 : b * 1.25;

			line[i] = apint_concatenate(r, apint_concatenate(g, apint_concatenate(b, a)));
		}

		MEM_WRITE(line, out_addr, LINE_LEN_BYTE);
		MBOX_PUT(s1_out, out_addr);

		toggle = toggle ? 0 : 1;
	}
}
