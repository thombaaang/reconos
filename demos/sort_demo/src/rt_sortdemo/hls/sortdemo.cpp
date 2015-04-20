#include "reconos_calls.h"
#include "reconos_thread.h"

#include "ap_cint.h"

#define BLOCK_SIZE 2048

THREAD_ENTRY() {
	RAM(uint32, BLOCK_SIZE, ram);

	THREAD_INIT();

	while(1) {
		uint32 addr = MBOX_GET(resources_address);
		MEM_READ(addr, ram, BLOCK_SIZE * 4);

		uint32 i, j;
		uint32 tmp;
		for (i = 0; i < BLOCK_SIZE; i++) {
			for (j = 0; j < BLOCK_SIZE - 1; j++) {
				if (ram[j] > ram[j + 1]) {
					tmp = ram[j];
					ram[j] = ram[j + 1];
					ram[j + 1] = tmp;
				}
			}
		}

		MEM_WRITE(ram, addr, BLOCK_SIZE * 4);
		MBOX_PUT(resources_acknowledge, addr);
	}
}