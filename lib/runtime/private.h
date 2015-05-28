/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Private header file
 *
 *   project:      ReconOS
 *   author:       Andreas Agne, University of Paderborn
 *                 Christoph Rüthing, University of Paderborn
 *   description:  Head file with private only definitions.
 *
 * ======================================================================
 */

#ifndef RECONOS_PRIVATE_H
#define RECONOS_PRIVATE_H

#include "reconos.h"
#include "reconos_defs.h"
#include "utils.h"

#include <stdint.h>
#include <semaphore.h>


/* == ReconOS proc control ============================================= */

/*
 * Handles page faults from the memory subsystem
 *
 *   arg - null
 */
void *proc_pgfhandler(void *arg);


/* == ReconOS hwslot =================================================== */

/*
 * Definition of the delegate states.
 *
 *   stoped          - delegate not running
 *   init            - initializing
 *   blocked_osif    - blocked in reading from osif
 *   blocked_syscall - blocked in syscall
 *   processing      - processing a request
 */
#define DELEGATE_STATE_STOPPED          0x01
#define DELEGATE_STATE_INIT             0x02
#define DELEGATE_STATE_BLOCKED_OSIF     0x04
#define DELEGATE_STATE_BLOCKED_SYSCALL  0x08
#define DELEGATE_STATE_PROCESSING       0x10

/*
 * Definition of signals to the delegate thread.
 *
 *   pause_syscalls - pauses incoming syscalls by not sending result
 *   suspend        - requested suspend of thread
 */
#define DELEGATE_FLAG_PAUSE_SYSCALLS 0x01
#define DELEGATE_FLAG_SUSPEND 0x02

/*
 * Object representing a hardware slot on the FPGA.
 *
 *   id - global id as used in the hardware design
 *   rt - pointer to the currently executing thread
 *
 *   osif     - file descriptor of the osif
 *   dt       - reference to the delegate thread
 *   dt_state - state of the delegate thread
 *   dt_flags - flags to the delegate thread
 *   dt_exit  - semaphore for synchronizing with the delegate on exit
 */
struct hwslot {
	int id;
	struct reconos_thread *rt;

	int osif;
	pthread_t dt;
#if 0
	int dt_state;
	int dt_flags;
	sem_t dt_exit;
#endif
};

/*
 * Initializes the slot.
 *
 *   slot - pointer to the hardware slot
 */
void hwslot_init(struct hwslot *slot, int id, int osif);

/*
 * Executes the given ReconOS thread in the slot by resetting
 * the hardware and reconfiguring it if needed. Running threads
 * will be killed or an error occurs.
 *
 *   slot - pointer to the hardware slot
 *   rt   - pointer to the ReconOS thread
 */
void hwslot_createthread(struct hwslot *slot,
                         struct reconos_thread *rt);

/*
 * Reset the slot.
 *
 *   slot  - pointer to the hardware slot
 */
void hwslot_reset(struct hwslot *slot);

/*
 * Sets the reset of the slot.
 *
 *   slot  - pointer to the hardware slot
 *   reset - zero or one to set the reset
 */
void hwslot_setreset(struct hwslot *slot, int reset);

/*
 * Sets the signal of the slot.
 *
 *   slot  - pointer to the hardware slot
 *   reset - zero or one to set the signal
 */
void hwslot_setsignal(struct hwslot *slot, int sig);

/*
 * Stops the delegate thread at an appropriate point in time
 *
 *   slot - pointer to the hardware slot
 */
void hwslot_stopdelegate(struct hwslot *slot);

/*
 * Suspends the active thread by saving its state and termination the
 * delegate thread.
 *
 *   slot - pointer to the hardware slot
 */
void hwslot_suspendthread(struct hwslot *slot);

/*
 * Resumes the thread by restoring its state. Running threads will be
 * killed or an error occurs.
 *
 *   slot - pointer to the hardware slot
 */
void hwslot_resumethread(struct hwslot *slot,
                         struct reconos_thread *rt);

/*
 * Waits for the termination of the running thread and resets the thread
 * afterwards.
 *
 *   slot - pointer to the hardware slot
 */
void hwslot_jointhread(struct hwslot *slot);

/*
 * Finds a free slot from all available slots.
 *
 *   @returns pointer to free slot
 *
 */
struct hwslot *hwslot_findfree();


/* == ReconOS swslot =================================================== */

/*
 * Object representing a software slot.
 *
 *   id - global id
 *   rt - pointer to the currently executing thread
 *
 *   osif      - file descriptor of the osif
 *   pthread_t - reference to thread
 */
struct swslot {
	int id;
	struct reconos_thread *rt;

	int osif;
	pthread_t thread;
};

/*
 * Initializes the slot.
 *
 *   slot - pointer to the software slot
 */
void swslot_init(struct swslot *slot, int id, int osif);

/*
 * Executes the given ReconOS thread in the slot by sarting a pthread.
 *
 *   slot - pointer to the hardware slot
 *   rt   - pointer to the ReconOS thread
 */
void swslot_createthread(struct swslot *slot,
                         struct reconos_thread *rt);

/*
 * Finds a free slot from all available slots.
 *
 *   @returns pointer to free slot
 *
 */
struct swslot *swslot_findfree();


/* == ReconOS delegate ================================================= */

/*
 * Global method of the delegate thread
 *
 *   arg - pointer to the ReconOS hwslot
 */
void *dt_delegate(void *arg);

#endif /* RECONOS_PRIVATE_H */