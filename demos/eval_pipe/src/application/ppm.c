#include "ppm.h"

#include <stdlib.h>
#include <stdio.h>

void ppm_init_empty(struct ppm *ppm) {
	ppm->width = 0;
	ppm->height = 0;
	ppm->data = NULL;
	ppm->count = 0;
}

void ppm_init(struct ppm *ppm,
              uint32_t width, uint32_t height) {
	ppm->width = width;
	ppm->height = height;
	ppm->count = width * height;
	ppm->data = malloc(ppm->count * sizeof(uint32_t));
}

void ppm_destroy(struct ppm *ppm) {
	ppm->width = 0;
	ppm->height = 0;
	free(ppm->data);
	ppm->data = NULL;
	ppm->count = 0;
}

int ppm_read(struct ppm *ppm, char *file) {
	int i;
	FILE *f;
	char buf[128];
	char *str;
	uint32_t width, height;

	f = fopen(file, "rb");
	if (!f) {
		printf("ERROR: Could not open file\n");
		goto err_open;
	}

	for (i = 0; i < 3; ) {
		fgets(buf, 128, f);
		str = strtok(buf, "\n");

		if (str[0] == '#')
			continue;

		switch (i++) {
			case 0:
				if (strcmp(str, "P6")) {
					printf("ERROR: Wrong encoding\n");
					goto err;
				}
				break;

			case 1:
				width = strtol(strtok(buf, " "), NULL, 10);
				height = strtol(strtok(NULL, " "), NULL, 10);
				ppm_init(ppm, width, height);
				break;

			case 2:
				if (!strcmp(buf, "255\r\n")) {
					printf("ERROR: Wrong encoding\n");
					goto err;
				}
				break;
		}
	}

	for (i = 0; i < ppm->count; i++) {
		ppm->data[i] = 0x000000FF;
		ppm->data[i] |= fgetc(f) << 24;
		ppm->data[i] |= fgetc(f) << 16;
		ppm->data[i] |= fgetc(f) << 8;

	}

	fclose(f);
	return 0;

err:
	fclose(f);
	ppm_destroy(ppm);

err_open:
	return -1;
}

int ppm_write(struct ppm *ppm, char *file) {
	int i;
	FILE *f;

	f = fopen(file, "wb");
	if (!f) {
		printf("ERROR: Could not open file\n");
		goto err_open;
	}

	fprintf(f, "P6\n");
	fprintf(f, "%d %d\n", ppm->width, ppm->height);
	fprintf(f, "255\n");

	for (i = 0; i < ppm->count; i++) {
		fputc((ppm->data[i] & 0xFF000000) >> 24, f);
		fputc((ppm->data[i] & 0x00FF0000) >> 16, f);
		fputc((ppm->data[i] & 0x0000FF00) >> 8, f);
	}

	fclose(f);
	return 0;

err_open:
	return -1;
}