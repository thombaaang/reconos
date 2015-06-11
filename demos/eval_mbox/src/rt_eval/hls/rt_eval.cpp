#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_cint.h"

#ifndef __SYNTHESIS__
#include "timer.h"
#endif

THREAD_ENTRY(, uint32 timer) {
	uint32 d = 0xaffedead;
	uint32 ts = 0;

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

			default:
				break;
		}
		
		MBOX_PUT(eval_time, ts);
	}
}