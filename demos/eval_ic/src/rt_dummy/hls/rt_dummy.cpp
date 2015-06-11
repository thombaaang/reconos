#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_cint.h"

#ifndef __SYNTHESIS__
#include "timer.h"
#endif

<<reconos_preproc>>

THREAD_ENTRY() {
	THREAD_INIT();

	uint32 d;
	while(1) {
		<<generate for RESOURCES>>
		d = MBOX_GET(dummy_dummy<<_i>>);
		MBOX_PUT(dummy_dummy<<_i>>, d);
		<<end generate>>
	}
}
