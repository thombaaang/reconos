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

	if (argc != 2) {
		printf("ERROR: First argument must be filname of image\n");
		return -1;
	}

	reconos_init();
	reconos_app_init();

	reconos_thread_create_hwt_read();
	reconos_thread_create_hwt_write();
	reconos_thread_create_hwt_brighten();
	reconos_thread_create_hwt_mirror();

	ppm_init_empty(&img);
	ppm_read(&img, argv[1]);

	mbox_put(read_cmd_ptr, (uint32_t)img.data);
	mbox_put(read_cmd_ptr, img.height);

	ppm_init(&img2, img.width, img.height);
	for (i = 0; i < img2.count; i++)
		img2.data[i] = 0xFF000000;

	mbox_put(write_cmd_ptr, (uint32_t)img2.data);
	mbox_put(write_cmd_ptr, img2.height);

	mbox_get(write_cmd_ptr);

	ppm_write(&img2, "evalpipe_out.ppm");

	reconos_app_cleanup();
	reconos_cleanup();

	return 0;
}
