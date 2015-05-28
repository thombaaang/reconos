#include "reconos_thread.h"
#include "reconos_calls.h"

void *rt_delegate(void * data) {
	THREAD_INIT();

	while(1) {
		MBOX_PUT(resources_address, MBOX_GET(delegateresources_daddress));
		MBOX_PUT(delegateresources_dacknowledge, MBOX_GET(resources_acknowledge));
	}

	THREAD_EXIT();
}