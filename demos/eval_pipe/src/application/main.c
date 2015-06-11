#include "reconos.h"
#include "reconos_app.h"
#include "mbox.h"
#include "pipe.h"
#include "timer.h"

#include <stdio.h>

#define EVAL_WRITE 0x00000000
#define EVAL_READ  0x00000001

int main(int argc, char **argv) {
	printf("Hello World\n");

	reconos_init();
	reconos_app_init();
	//timer_init();

	reconos_thread_create_swt_eval();
	reconos_thread_create_hwt_eval();

	while(1) {
		mbox_put(eval_cmd_ptr, EVAL_WRITE);
		mbox_put(eval_cmd_ptr, EVAL_READ);
	}

	reconos_app_cleanup();
	reconos_cleanup();
	//timer_cleanup();
	
	return 0;
}