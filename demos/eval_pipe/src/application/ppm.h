#ifndef PPM_H
#define PPM_H

#include <stdint.h>
#include <string.h>

struct ppm {
	uint32_t width, height;
	volatile uint32_t *data;
	size_t count;
};

extern void ppm_init_empty(struct ppm *ppm);
extern void ppm_init(struct ppm *ppm,
                     uint32_t width, uint32_t height);
extern void ppm_destroy(struct ppm *ppm);
extern int ppm_read(struct ppm *ppm, char *file);
extern int ppm_write(struct ppm *ppm, char *file);

#endif /* PPM_H */