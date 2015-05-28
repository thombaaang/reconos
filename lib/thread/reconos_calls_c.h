/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Calls library header file
 *
 *   project:      ReconOS
 *   author:       Andreas Agne, University of Paderborn
 *                 Christoph Rüthing, University of Paderborn
 *   description:  ReconOS calls to allow interaction with POSIX-API of
 *                 system calls.
 *
 * ======================================================================
 */

#ifndef RECONOS_CALLS_H
#define RECONOS_CALLS_H

#include "reconos.h"
#include "reconos_defs.h"

#include "mbox.h"

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

/* == Call functions =================================================== */

/*
 * Creates a local ram to be used for mem functions. You may only pass
 * rams created by this macro to mem functions.
 *
 *   type - datatype of the ram 
 *   size - size of the ram
 *   name - name of the ram
 */
#define RAM(type,size,name)\
 	type *name

/*
 * Posts the semaphore specified by handle.
 *
 *   @see sem_post
 */
#define SEM_POST(p_handle)\
	sem_post((p_handle)->ptr)

/*
 * Waits for the semaphore specified by handle.
 *
 *   @see sem_wait
 */
#define SEM_WAIT(p_handle)\
	sem_wait((p_handle)->ptr)

/*
 * Locks the mutex specified by handle.
 *
 *   @see pthread_mutex_lock
 */
#define MUTEX_LOCK(p_hande)\
	pthread_mutex_lock((p_handle)->ptr)

/*
 * Unlocks the mutex specified by handle.
 *
 *   @see pthread_mutex_unlock
 */
#define MUTEX_UNLOCK(p_handle)\
	pthread_mutex_unlock((p_handle)->ptr)

/*
 * Tries to lock the mutex specified by handle and returns if successful or not.
 *
 *   @see pthread_mutex_trylock
 */

#define MUTEX_TRYLOCK(p_handle)\
	pthread_mutex_trylock((p_handle)->ptr)

/*
 * Puts a single word into the mbox specified by handle.
 *
 *   @see mbox_get
 */
#define MBOX_GET(res)\
	_MBOX_GET(res, __rt, __run_id)

static inline uint32_t _MBOX_GET(struct reconos_resource *res,
                                 struct reconos_thread *rt, int run_id) {
	uint32_t msg[2];

	if (res->mode == RECONOS_RESOURCE_MODE_SW) {
		return mbox_get(res->ptr);
	} else if (res->mode == RECONOS_RESOURCE_MODE_HW) {
		msg[0] = (run_id << 24) | (res->id << 16) | 0x00000001;
		msg[1] = OSIF_CMD_MBOX_GET;
		reconos_thread_swslot_write(rt, msg, 2);
		reconos_thread_swslot_read(rt, msg, 2);
		return msg[1];
	}

	return 0;
}

/*
 * Reads a single word from the mbox specified by handle.
 *
 *   @see mbox_put
 */
#define MBOX_PUT(res,data)\
	_MBOX_PUT(res, data, __rt, __run_id)

static inline void _MBOX_PUT(struct reconos_resource *res, uint32_t data,
                             struct reconos_thread *rt, int run_id) {
	uint32_t msg[3];

	if (res->mode == RECONOS_RESOURCE_MODE_SW) {
		mbox_put(res->ptr, data);
	} else if (res->mode == RECONOS_RESOURCE_MODE_HW) {
		msg[0] = (run_id << 24) | (res->id << 16) | 0x00000002;
		msg[1] = OSIF_CMD_MBOX_PUT;
		msg[2] = data;
		reconos_thread_swslot_write(rt, msg, 3);
		reconos_thread_swslot_read(rt, msg, 3);
	}
}

/*
 * Tries to put a single word into the mbox specified by handle but does not
 * blocks until the mbox gets populated.
 *
 *   @see mbox_tryget
 */
#define MBOX_TRYGET(p_handle,data)\
	mbox_tryget((p_handle)->ptr, (&data))

/*
 * Tries to read a single word from the mbox specified by handle but does not
 * blocks until the mbox gets free.
 *
 *   @see mbox_tryput
 */
#define MBOX_TRYPUT(p_handle,data)\
	mbox_tryput((p_handle)->ptr, (data))

/*
 * Gets the pointer to the initialization data of the ReconOS thread
 * specified by reconos_hwt_setinitdata.
 */
#define GET_INIT_DATA()\
	data

/*
 * Reads several words from the main memory into the local ram.
 *
 *   src - start address to read from the main memory
 *   dst - array to write data into
 *   len - number of bytes to transmit (bytes)
 */
#define MEM_READ(src,dst,len)\
	dst = (void *)src

/*
 * Writes several words from the local ram into the main memory.
 *
 *   src - array to read data from
 *   dst - start address to read from the main memory
 *   len - number of bytes to transmit (bytes)
 */
#define MEM_WRITE(src, dst, len)

#endif /* RECONOS_CALLS_H */