/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Utilities
 *
 *   project:      ReconOS
 *   author:       Andreas Agne, University of Paderborn
 *                 Markus Happe, University of Paderborn
 *                 Sebastian Meisner, University of Paderborn
 *                 Christoph Rüthing, University of Paderborn
 *   description:  Some simple helper funtions used internally.
 *
 * ======================================================================
 */

#ifndef RECONOS_UTILS_H
#define RECONOS_UTILS_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define RECONOS_NODEBUG

#ifdef RECONOS_DEBUG
static inline void debug(char *msg, ...) {
	va_list vl;

	va_start(vl, msg);
	vfprintf(stdout, msg, vl);
	va_end(vl);
	fprintf(stdout, "\n");

	fflush(stdout);
}
#else
#define debug(...)
#endif

static inline void die() {
	exit(EXIT_FAILURE);
}

static inline void panic(char *msg, ...) {
	va_list vl;

	va_start(vl, msg);
	vfprintf(stderr, msg, vl);
	va_end(vl);
	fprintf(stderr, "\n");

	fflush(stderr);
	die();
}

static inline void whine(char *msg, ...) {
	va_list vl;

	va_start(vl, msg);
	vfprintf(stderr, msg, vl);
	va_end(vl);
	fprintf(stderr, "\n");

	fflush(stderr);
}

static inline void debug_array(uint32_t *arr, size_t count,
                               char *msg, ...) {
	va_list vl;
	int i;

	va_start(vl, msg);
	vfprintf(stdout, msg, vl);
	va_end(vl);

	for (i = 0; i < count; i++) {
		fprintf(stdout, "0x%08x ", arr[i]);
	}
	fprintf(stdout, "\n");

	fflush(stdout);
}

#endif /* RECONOS_UTILS_H */
