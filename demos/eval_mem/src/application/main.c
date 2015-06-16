#include "ppm.h"

#include "reconos.h"
#include "reconos_app.h"
#include "mbox.h"
#include "pipe.h"
#include "timer.h"

#include <stdio.h>

int main(int argc, char **argv) {
	int i;
	struct ppm img;
	struct ppm img2;
	uint32_t buf0[256], buf1[256], buf2[256];
	unsigned int start, end;

	if (argc != 2) {
		printf("ERROR: First argument must be filname of image\n");
		return -1;
	}

	reconos_init();
	reconos_app_init();
	timer_init();

	reconos_thread_createi_hwt_read(buf0);
	reconos_thread_createi_hwt_write(0);
	reconos_thread_createi_hwt_brighten(buf1);
	reconos_thread_createi_hwt_mirror(buf2);

	ppm_init_empty(&img);
	ppm_read(&img, argv[1]);

	start = timer_get();
	mbox_put(read_cmd_ptr, (uint32_t)img.data);
	mbox_put(read_cmd_ptr, img.height);

	ppm_init(&img2, img.width, img.height);
	for (i = 0; i < img2.count; i++)
		img2.data[i] = 0xFF000000;

	mbox_put(write_cmd_ptr, (uint32_t)img2.data);
	mbox_put(write_cmd_ptr, img2.height);

	mbox_get(write_cmd_ptr);
	end = timer_get();

	ppm_write(&img2, "evalmem_out.ppm");

	printf("Writting output file (%f ms)\n", timer_diffms(start, end));

	reconos_app_cleanup();
	reconos_cleanup();
	timer_cleanup();

	return 0;
}
