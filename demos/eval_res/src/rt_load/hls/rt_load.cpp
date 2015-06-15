#include "reconos_thread.h"
#include "reconos_calls.h"

#include "ap_cint.h"

THREAD_ENTRY() {
	THREAD_INIT();

	uint32 cmd = MBOX_GET(load_cmd);

	while(1) {
		switch(cmd) {
			case 0x00000010:
				MBOX_GET(load_mbox);
				break;

			case 0x00000020:
				MBOX_PUT(load_mbox, 0xaffedead);
				break;

			default:
				break;
		}
	}
}