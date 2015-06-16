#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_cint.h"

#ifndef __SYNTHESIS__
#include "timer.h"
#endif

#define LEN_WORDS 128
#define LEN_BYTES (LEN_WORDS * 4)

THREAD_ENTRY(, uint32 timer) {
	uint32 d = 0xaffedead;
	uint32 ts = 0;
	uint32 buf[LEN_WORDS];

	THREAD_INIT();

	while(1) {
#ifdef __SYNTHESIS__
		uint32 cmd = MBOX_GET(eval_cmdhw);
#else
		uint32 cmd = MBOX_GET(eval_cmdsw);
#endif

		switch(cmd) {
			case 0x00000010:
				d = MBOX_GET(eval_mboxsw);
#ifdef __SYNTHESIS__
				ts = timer;
#else
				ts = timer_get();
#endif
				break;

			case 0x00000011:
				d = MBOX_GET(eval_mboxhw);
#ifdef __SYNTHESIS__
				ts = timer;
#else
				ts = timer_get();
#endif
				break;

			case 0x00000020:
#ifdef __SYNTHESIS__
				ts = timer;
#else
				ts = timer_get();
#endif
				MBOX_PUT(eval_mboxsw, d);
				break;

			case 0x00000021:
#ifdef __SYNTHESIS__
				ts = timer;
#else
				ts = timer_get();
#endif
				MBOX_PUT(eval_mboxhw, d);
				break;

			case 0x00000030:
				PIPE_READ(eval_pipesw, buf, LEN_BYTES);
#ifdef __SYNTHESIS__
				ts = timer;
#else
				ts = timer_get();
#endif
				break;

			case 0x00000031:
				PIPE_READ(eval_pipehw, buf, LEN_BYTES);
#ifdef __SYNTHESIS__
				ts = timer;
#else
				ts = timer_get();
#endif
				break;

			case 0x00000040:
#ifdef __SYNTHESIS__
				ts = timer;
#else
				ts = timer_get();
#endif
				PIPE_WRITE(eval_pipesw, buf, LEN_BYTES);
				break;

			case 0x00000041:
#ifdef __SYNTHESIS__
				ts = timer;
#else
				ts = timer_get();
#endif
				PIPE_WRITE(eval_pipehw, buf, LEN_BYTES);
				break;

			default:
				break;
		}
		
		MBOX_PUT(eval_time, ts);
	}
}