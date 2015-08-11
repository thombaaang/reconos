/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Runtime library
 *
 *   project:      ReconOS
 *   author:       Andreas Agne, University of Paderborn
 *                 Christoph Rüthing, University of Paderborn
 *   description:  ReconOS runtime library managing all threads and
 *                 internal data structures. It provides functions
 *                 to manipulate the state of the system.
 *
 * ======================================================================
 */

#ifndef RECONOS_H
#define RECONOS_H

#include "reconos_defs.h"

#include <pthread.h>
#include <stdint.h>

extern int RECONOS_NUM_HWTS;

/* == ReconOS resource ================================================= */

/*
 * Object representing a single resource.
 *
 *   id   - id of resource
 *   type - type of the resource (RECONOS_RESOURCE_TYPE_...)
 *   mode - implementation mode of the resource (RECONOS_RESOURCE_MODE_ ...)
 *   ptr  - pointer to the representation of the resource
 */
struct reconos_resource {
	int id;
	int type;
	int mode;
	void *ptr;
};

/*
 * Initializes a software resource.
 *
 *   res  - pointer to the resource
 *   id   - id of the resource
 *   type - type of the resource
 *   mode - mode of the resource
 *   ptr  - pointer to the representation of the resource
 */
void reconos_resource_init(struct reconos_resource *res, int id,
                           int type, int mode, void *ptr);


/* == ReconOS thread =================================================== */

/*
 * Object representing a reconos thread
 *
 *   name              - name of the thread
 *   id                - id of the thread
 *   run_id            - run id of the thread
 *
 *   init_data         - pointer to the initialization data
 *   resources         - array of resources associated
 *   resource_count    - number of resources in resource array
 *
 *   state             - current state (refers to RECONOS_THREAD_STATE_...)
 *   state_data        - memory to store the internal state
 *
 *   allowed_slots     - allowed slots to execute the thread in
 *   hw_slot           - hardware slot the thread is executing in
 *
 *   bitstreams        - array of bitstreams for the different slots
 *   bitstream_lengths - length of the bitstreams
 *   swentry           - pointer to the software entry function
 *
 *   osif              - osif associated to the thread
 */
struct reconos_thread {
	char *name;
	int id;

	void *init_data;
	struct reconos_resource *resources;
	int resource_count;

	int state;
	int state_size;
	volatile void *state_data;

	struct hwslot *hwslot;
	struct swslot *swslot;

	char **bitstreams;
	int *bitstream_lengths;
	struct hwslot **allowed_hwslots;
	size_t allowed_hwslots_count;
	void *(*swentry)(void *data);

	int osif;
};

/*
 * Initializes the ReconOS thread. Must be called before all other
 * methods.
 *
 *   rt         - pointer to the ReconOS thread
 *   name       - name of the thread (can be null)
 *   state_size - size in bytes for the state of the thread
 */
void reconos_thread_init(struct reconos_thread *rt,
                         char* name,
                         int state_size);

/*
 * Associates initialization data to this thread.
 *
 *   rt        - pointer to the ReconOS thread
 *   init_data - pointer to the initialization data
 */
void reconos_thread_setinitdata(struct reconos_thread *rt, void *init_data);

/*
 * Associates the resource array to this thread. The resource array
 * is used directly and no copy is created. Make shure to not modify
 * it and not put it on the stack.
 *
 *   rt             - pointer to the ReconOS thread
 *   resources      - array of resources
 *   resource_count - number of resources in resource array
 */
void reconos_thread_setresources(struct reconos_thread *rt,
                                 struct reconos_resource *resources,
                                 int resource_count);

/*
 * Associates the resource array to this thread. Therefore it allocates
 * memory and copies the provided resource structures. You can savely
 * free all memory allocated to pass data to this function.
 *
 *   rt             - pointer to the ReconOS thread
 *   resources      - array of pointers to resources
 *   resource_count - number of resources in resource array
 */

void reconos_thread_setresourcepointers(struct reconos_thread *rt,
                                        struct reconos_resource **resources,
                                        int resource_count);

/*
 * Assigns the bitstream array to the hardware thread. The bitstream
 * array must contain a bitstream for each hardware slot.
 *
 *   rt  - pointer to the ReconOS thread
 *   bitstreams - array of bitstreams (array of chars)
 *   bitstream_lengths - lengths of the different bitstreams
 */
void reconos_thread_setbitstream(struct reconos_thread *rt,
                                 char **bitstreams,
                                 int *bitstream_lengths);

/*
 * Assigns the allowed hardware slots the thread can be exectued in.
 *
 *   rt                    - pointer to the ReconOS thread
 *   allowed_slotids       - array of allowed slot ids
 *   allowed_slotids_count - number of allowed slots
 */
void reconos_thread_setallowedhwslots(struct reconos_thread *rt,
                                      int *allowed_slotids,
                                      size_t allowed_slotids_count);

/*
 * Sets the main method of the software thread. When createing the
 * thread, a pointer to the thread is passed via the data parameter.
 *
 *   rt      - pointer to the ReconOS thread
 *   swentry - function pointer to the main entry point
 */
void reconos_thread_setswentry(struct reconos_thread *rt,
                               void *(*swentry)(void *data));

/*
 * Creates the ReconOS thread and executes it in a free slot.
 *
 *   rt - pointer to the ReconOS thread
 *   tt - software or hardware thread
 *
 */
void reconos_thread_create(struct reconos_thread *rt, int mode);

/*
 * Suspends the ReconOS thread by saving its state and pausing execution.
 * This method does not block until the thread is suspended, use
 * reconos_thread_join(...) to wait for termination of thread.
 *
 *   rt   - pointer to the ReconOS thread
 */
void reconos_thread_suspend(struct reconos_thread *rt);

/*
 * Suspends the ReconOS thread by saving its state and pausing execution.
 * This method blocks unit the thread is suspended.
 */
void reconos_thread_suspend_block(struct reconos_thread *rt);

/*
 * Resumes the ReconOS thread in the given slot by restoring its state
 * and starting execution.
 *
 *   rt   - pointer to the ReconOS thread
 *   slot - slot number to execute the thread in
 */
void reconos_thread_resume(struct reconos_thread *rt, int slot);

/*
 * Waits for the termination of the hardware thread.
 *
 *   rt - pointer to the ReconOS thread
 */
void reconos_thread_join(struct reconos_thread *rt);

/*
 * Sets a signal to the hardware thread. The signal must be cleared
 * by the hardware using the right system call.
 *
 *   rt - pointer to the ReconOS thread
 */
void reconos_thread_signal(struct reconos_thread *rt);

/*
 * Returns the run id of the thread. This is the id of the hardware
 * or software slot, respecively.
 *
 *   rt - pointer to the ReconOS thread
 *
 *   @returns run id
 */
int reconos_thread_get_runid(struct reconos_thread *rt);

/*
 * Reads from the osif of the software slot.
 *
 *   rt    - pointer to the ReconOS thread
 *   buf   - pointer to buffer to read from
 *   count - number of words to write
 *
 *   @returns number of read words
 */
size_t reconos_thread_swslot_read(struct reconos_thread *rt,
                                  uint32_t *buf, size_t count);

/*
 * Writes to the osif of the software slot.
 *
 *   rt    - pointer to the ReconOS thread
 *   buf   - pointer to buffer to read from
 *   count - number of words to write
 */
void reconos_thread_swslot_write(struct reconos_thread *rt,
                                 uint32_t *buf, size_t count);

/*
 * Reads the signal state.
 *
 *   rt - pointer to the ReconOS thread
 *
 *   @returns signal state
 */
int reconos_thread_swslot_signal(struct reconos_thread *rt);

/*
 * Cleanups the thread before exiting.
 *
 *   rt - pointer to the ReconOS thread
 */
void reconos_thread_swslot_cleanup(struct reconos_thread *rt);


/* == General functions ================================================ */

/*
 * Initializes the ReconOS environment and resets the hardware. You must
 * call this method before you can use any of the other functions.
 */
void reconos_init();

/*
 * Cleans up the ReconOS environment and resets the hardware. You should
 * call this method before termination to prevent the hardware threads from
 * operating and avoid undesirable effects.
 * By default this method is registered as a signal handler for SIGINT,
 * SIGTERM and SIGABRT. Keep this in mind when overriding these handlers.
 */
void reconos_cleanup();

/*
 * Flushes the cache if needed.
 */
void reconos_cache_flush();

#endif /* RECONOS_H */