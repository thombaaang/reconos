#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_int.h"

#define LINE_LEN_WORD 128
#define LINE_LEN_BYTE (LINE_LEN_WORD * 4)

THREAD_ENTRY() {
	THREAD_INIT();
	RAM(ap_uint<32>, LINE_LEN_WORD, line);

	while(1) {
		PIPE_READ(s0_out, line, LINE_LEN_BYTE);

		for (int i = 0; i < LINE_LEN_WORD; i++) {
			ap_uint<8> r = line[i](31, 24);
			ap_uint<8> g = line[i](23, 16);
			ap_uint<8> b = line[i](15, 8);
			ap_uint<8> a = line[i](7, 0);

			r = r > 204 ? 255 : r * 1.25;
			g = g > 204 ? 255 : g * 1.25;
			b = b > 204 ? 255 : b * 1.25;

			line[i] = (r, g, b, a);
		}

		PIPE_WRITE(s1_out, line, LINE_LEN_BYTE);
	}
}
