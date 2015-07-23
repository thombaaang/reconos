#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_cint.h"

#define LINE_LEN_WORD 128
#define LINE_LEN_BYTE (LINE_LEN_WORD * 4)

THREAD_ENTRY() {
	THREAD_INIT();
	RAM(ap_uint<32>, LINE_LEN_WORD, line);

	while(1) {
		PIPE_READ(s1_out, line, LINE_LEN_BYTE);

		for (int i = 0; i < LINE_LEN_WORD / 2; i++) {
			ap_uint<32> tmp = line[i];
			line[i] = line[LINE_LEN_WORD - i - 1];
			line[LINE_LEN_WORD - i - 1] = tmp;
		}

		PIPE_WRITE(s2_out, line, LINE_LEN_BYTE);
	}
}