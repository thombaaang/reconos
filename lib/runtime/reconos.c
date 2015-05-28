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
 *                 Markus Happe, University of Paderborn
 *                 Sebastian Meisner, University of Paderborn
 *                 Christoph Rüthing, University of Paderborn
 *   description:  ReconOS runtime library managing all threads and
 *                 internal data structures. It provides functions
 *                 to manipulate the state of the system.
 *
 * ======================================================================
 */

#include "reconos.h"
#include "private.h"
#include "arch/arch.h"
#include "comp/mbox.h"

#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>

int RECONOS_NUM_HWSLOTS = 0;
int RECONOS_NUM_SWSLOTS = 0;

static struct hwslot *_hwslots;
static struct swslot *_swslots;

static int _proc_control;
static pthread_t _pgf_handler;
#if 0
static struct sigaction _dt_signal;
#endif

static int _thread_id;


/* == ReconOS resource ================================================= */

/*
 * @see header
 */
void reconos_resource_init(struct reconos_resource *res, int id,
                              int type, int mode, void *ptr) {
	res->id = id;
	res->type = type;
	res->mode = mode;
	res->ptr = ptr;
}

/* == ReconOS thread =================================================== */

/*
 * @see header
 */
void reconos_thread_init(struct reconos_thread *rt,
                         char* name,
                         int state_size) {
	rt->id = _thread_id++;
	rt->name = name;

	rt->init_data = NULL;
	rt->resources = NULL;
	rt->resource_count = 0;

	rt->state = RECONOS_THREAD_STATE_INIT;
	rt->state_size = state_size;

	if (state_size != 0) {
		rt->state_data = malloc(state_size);
		if (!rt->state_data) {
			panic("[reconos-core] ERROR: failed to allocate memory for state");
		}
		memset((void *)(rt->state_data), 0, state_size);
	} else {
		rt->state_data = NULL;
	}

	rt->hwslot = NULL;
	rt->swslot = NULL;

	rt->bitstreams = NULL;
	rt->bitstream_lengths = NULL;
}

/*
 * @see header
 */
void reconos_thread_setinitdata(struct reconos_thread *rt, void *init_data) {
	rt->init_data = init_data;
}

/*
 * @see header
 */
void reconos_thread_setresources(struct reconos_thread *rt,
                                 struct reconos_resource *resources,
                                 int resource_count) {
	rt->resources = resources;
	rt->resource_count = resource_count;
}

/*
 * @see header
 */
void reconos_thread_setresourcepointers(struct reconos_thread *rt,
                                        struct reconos_resource **resources,
                                        int resource_count) {
	int i;

	rt->resources = (struct reconos_resource *)malloc(resource_count * sizeof(struct reconos_resource));
	if (!rt->resources) {
		panic("[reconos-core] ERROR: failed to allocate memory for resources");
	}

	for (i = 0; i < resource_count; i++) {
		rt->resources[i] = *resources[i];
	}
	rt->resource_count = resource_count;
}

/*
 * @see header
 */
void reconos_thread_setbitstream(struct reconos_thread *rt,
                                 char **bitstreams,
                                 int *bitstream_lengths) {
	rt->bitstreams = bitstreams;
	rt->bitstream_lengths = bitstream_lengths;
}

/*
 * @see header
 */
void reconos_thread_setswentry(struct reconos_thread *rt,
                               void *(*swentry)(void *data)) {
	rt->swentry = swentry;
}

/*
 * @see header
 */
void reconos_thread_create(struct reconos_thread *rt, int mode) {
	if (!rt->state == RECONOS_THREAD_STATE_INIT) {
		panic("[reconos-core] ERROR: thread was already created");
	}

	switch (mode) {
		case RECONOS_THREAD_MODE_SW:
			rt->swslot = swslot_findfree();
			if (!rt->swslot) {
				whine("[reconos-core] ERROR: no free software slot found");
				return;
			}
			swslot_createthread(rt->swslot, rt);
			rt->state = RECONOS_THREAD_STATE_RUNNING_SW;
			break;

		case RECONOS_THREAD_MODE_HW:
			rt->hwslot = hwslot_findfree();
			if (!rt->hwslot) {
				whine("[reconos-core] ERROR: no free hardware slot found");
				return;	
			}
			hwslot_createthread(rt->hwslot, rt);
			rt->state = RECONOS_THREAD_STATE_RUNNING_HW;
			break;

		default:
			break;
	}
}

/*
 * @see header
 */
void reconos_thread_suspend(struct reconos_thread *rt) {
#if 0
	if (rt->state != RECONOS_THREAD_STATE_RUNNING_HW) {
		panic("[reconos-core] ERROR: cannot suspend not running thread\n");
	}

	rt->state = RECONOS_THREAD_STATE_SUSPENDING;
	hwslot_suspendthread(rt->hwslot);
	rt->state = RECONOS_THREAD_STATE_SUSPENDED;
#endif
}

/*
 * @see header
 */
void reconos_thread_suspend_block(struct reconos_thread *rt) {
#if 0
	if (rt->state != RECONOS_THREAD_STATE_RUNNING_HW) {
		panic("[reconos-core] ERROR: cannot suspend not running thread\n");
	}

	rt->state = RECONOS_THREAD_STATE_SUSPENDING;
	hwslot_suspendthread(rt->hwslot);
	hwslot_jointhread(rt->hwslot);
	rt->state = RECONOS_THREAD_STATE_SUSPENDED;
#endif
}

/*
 * @see header
 */
void reconos_thread_resume(struct reconos_thread *rt, int slot) {
#if 0
	int i;

	if (slot < 0 || slot >= RECONOS_NUM_HWTS) {
		panic("[reconos-core] ERROR: slot id out of range\n");
	}

	for (i = 0; i < rt->allowed_hwslot_count; i++) {
		if (rt->allowed_hwslots[i]->id == _hwslots[slot].id) {
			break;
		}
	}
	if (i == rt->allowed_hwslot_count) {
		panic("[reconos-core] ERROR: thread not allowed to run in slot\n");
	}

	hwslot_resumethread(rt->hwslot, rt);
	rt->state = RECONOS_THREAD_STATE_RUNNING_HW;
#endif
}

/*
 * @see header
 */
void reconos_thread_join(struct reconos_thread *rt) {
#if 0
	hwslot_jointhread(rt->hwslot);
#endif
}

/*
 * @see header
 */
void reconos_thread_signal(struct reconos_thread *rt) {
#if 0
	hwslot_setsignal(rt->hwslot, 1);
#endif
}

/*
 * @see header
 */
int reconos_thread_get_runid(struct reconos_thread *rt) {
	switch (rt->state) {
		case RECONOS_THREAD_STATE_RUNNING_SW:
			return rt->swslot->id;

		case RECONOS_THREAD_STATE_RUNNING_HW:
			return rt->hwslot->id;

		default:
			return -1;
	}
}

/*
 * @see header
 */
size_t reconos_thread_swslot_read(struct reconos_thread *rt,
                                  uint32_t *buf, size_t count) {
	if (!rt->swslot){
		panic("[reconos-core] "
		      "ERROR: No swslot found");
	}

	return reconos_osif_read(rt->swslot->osif, buf, count * sizeof(uint32_t));
}

/*
 * @see header
 */
void reconos_thread_swslot_write(struct reconos_thread *rt,
                                 uint32_t *buf, size_t count) {
	if (!rt->swslot){
		panic("[reconos-core] "
		      "ERROR: No swslot found");
	}

	reconos_osif_write(rt->swslot->osif, buf, count * sizeof(uint32_t));
}


/* == General functions ================================================ */

/*
 * Signal handler for exiting the program
 *
 *   sig - signal
 */
void exit_signal(int sig) {
	reconos_cleanup();

	printf("[reconos-core] aborted\n");

	exit(0);
}

/*
 * Signal handler for ínterrupting a delegate thread
 *
 *   sig - signal
 */
void delegate_signal(int sig) {
	debug("[reconos-core] delegate received signal");
}

/*
 * @see header
 */
void reconos_init() {
	int i, osif, id;

	signal(SIGINT, exit_signal);
	signal(SIGTERM, exit_signal);
	signal(SIGABRT, exit_signal);

	reconos_drv_init();

	_thread_id = 0;

	_proc_control = reconos_proc_control_open();
	if (_proc_control < 0) {
		panic("[reconos-core] ERROR: unable to open proc control");
	}

	RECONOS_NUM_HWSLOTS = reconos_proc_control_get_num_hwts(_proc_control);

	_hwslots = (struct hwslot *)malloc(RECONOS_NUM_HWSLOTS * sizeof(struct hwslot));
	if (!_hwslots) {
		panic("[reconos-core] ERROR: unable to allocate memory for slots");
	}

	for (i = 0; i < RECONOS_NUM_HWSLOTS; i++) {
		id = i;
		osif = reconos_osif_open(0xFF000000, id << 24);

		hwslot_init(&_hwslots[i], id, osif);
	}

	RECONOS_NUM_SWSLOTS = 4;

	_swslots = (struct swslot *)malloc(RECONOS_NUM_SWSLOTS * sizeof(struct swslot));
	if (!_swslots) {
		panic("[reconos-core] ERROR: unable to open osif");
	}

	for (i = 0; i < RECONOS_NUM_SWSLOTS; i++) {
		id = RECONOS_NUM_HWSLOTS + i;
		osif = reconos_osif_open(0x00FF0000, id << 16);

		swslot_init(&_swslots[i], id, osif);
	}

#if 0
	_dt_signal.sa_handler = delegate_signal;
	sigaction(SIGUSR1, &_dt_signal, NULL);
#endif

	reconos_proc_control_sys_reset(_proc_control);

	reconos_proc_control_set_pgd(_proc_control);

#ifdef RECONOS_OS_linux
	pthread_create(&_pgf_handler, NULL, proc_pgfhandler, NULL);
#endif
}

/*
 * @see header
 */
void reconos_cleanup() {
	reconos_proc_control_sys_reset(_proc_control);
}

/*
 * @see header
 */
void reconos_cache_flush() {
	reconos_proc_control_cache_flush(_proc_control);
}


/* == ReconOS proc control============================================== */

/*
 * @see header
 */
void *proc_pgfhandler(void *arg) {
	while (1) {
		uint32_t *addr;

		addr = (uint32_t *)reconos_proc_control_get_fault_addr(_proc_control);

		printf("[reconos_core] "
		       "page fault occured at address 0x%08x\n", (unsigned int)addr);

		*addr = 0;

		reconos_proc_control_clear_page_fault(_proc_control);
	}
}


/* == ReconOS hwslot =================================================== */

/*
 * @see header
 */
void hwslot_init(struct hwslot *slot, int id, int osif) {
	slot->id = id;
	slot->osif = osif;

	slot->rt = NULL;

	slot->dt = 0;
#if 0
	slot->dt_state = DELEGATE_STATE_STOPPED;
	slot->dt_flags = 0;
	sem_init(&slot->dt_exit, 0, 0);
#endif
}

/*
 * @see header
 */
void hwslot_createthread(struct hwslot *slot,
                         struct reconos_thread *rt) {
	uint32_t msg[3];

	if (slot->rt) {
		panic("[reconos-core] ERROR: a thread is already running");
	}

	slot->rt = rt;

	hwslot_reset(slot);

	msg[0] = 0xFF000002 | (slot->id << 16);
	msg[1] = OSIF_SIGNAL_THREAD_START;
	msg[2] = slot->id;
	reconos_osif_write(slot->osif, msg, 3 * sizeof(uint32_t));

	pthread_create(&slot->dt, NULL, dt_delegate, slot);
}

/*
 * @see header
 */
void hwslot_reset(struct hwslot *slot) {
	debug("[reconos-core] resetting slot %d", slot->id);
	reconos_proc_control_hwt_reset(_proc_control, slot->id, 1);
	reconos_proc_control_hwt_signal(_proc_control, slot->id, 0);
	reconos_proc_control_hwt_reset(_proc_control, slot->id, 0);
}

/*
 * @see header
 */
void hwslot_setreset(struct hwslot *slot, int reset) {
	reconos_proc_control_hwt_reset(_proc_control, slot->id, reset);
}

/*
 * @see header
 */
void hwslot_setsignal(struct hwslot *slot, int sig) {
	reconos_proc_control_hwt_signal(_proc_control, slot->id, sig);
}

/*
 * @see header
 */
void hwslot_suspendthread(struct hwslot *slot) {
#if 0
	if (!slot->rt) {
		panic("[reconos-core] ERROR: no thread running\n");
	}

	slot->dt_flags |= DELEGATE_FLAG_PAUSE_SYSCALLS;
	slot->dt_flags |= DELEGATE_FLAG_SUSPEND;

	do {
		switch (slot->dt_state) {
			case DELEGATE_STATE_BLOCKED_OSIF:
				reconos_osif_break(slot->osif);
				break;

			case DELEGATE_STATE_BLOCKED_SYSCALL:
				pthread_kill(slot->dt, SIGUSR1);
				break;
		}

		sched_yield();
	} while (slot->dt_flags & DELEGATE_FLAG_SUSPEND);
#endif
}

/*
 * @see header
 */
void hwslot_resumethread(struct hwslot *slot,
                         struct reconos_thread *rt) {
#if 0
	if (slot->rt) {
		panic("[reconos-core] ERROR: a thread is already running\n");
	}

	reconos_proc_control_hwt_reset(_proc_control, slot->id, 1);
	reconos_proc_control_hwt_signal(_proc_control, slot->id, 0);
	reconos_proc_control_hwt_reset(_proc_control, slot->id, 0);

	reconos_osif_write(slot->osif, (uint32_t)OSIF_SIGNAL_THREAD_RESUME);

	slot->rt = rt;
#endif
}

/*
 * @see header
 */
void hwslot_jointhread(struct hwslot *slot) {
#if 0
	if (!slot->rt) {
		panic("[reconos-core] ERROR: no thread running\n");
	}

	sem_wait(&slot->dt_exit);

	reconos_proc_control_hwt_reset(_proc_control, slot->id, 1);
	reconos_proc_control_hwt_signal(_proc_control, slot->id, 0);
	slot->rt = NULL;
#endif
}

/*
 * @see header
 */
struct hwslot *hwslot_findfree() {
	int i;
	struct hwslot *hwslot = NULL;

	for (i = 0; i < RECONOS_NUM_HWSLOTS; i++) {
		hwslot = &_hwslots[i];

		if (!hwslot->rt)
			break;
	}

	return hwslot;
}


/* == ReconOS swslot =================================================== */

/*
 * @see header
 */
void swslot_init(struct swslot *slot, int id, int osif) {
	slot->id = id;
	slot->osif = osif;

	slot->rt = NULL;

	slot->thread = 0;
}

/*
 * @see header
 */
void swslot_createthread(struct swslot *slot,
                         struct reconos_thread *rt) {
	if (slot->rt) {
		panic("[reconos-core] ERROR: a thread is already running");
	}

	slot->rt = rt;

	pthread_create(&slot->thread, NULL, rt->swentry, rt);
} 

/*
 * @see header
 */
struct swslot *swslot_findfree() {
	int i;
	struct swslot *swslot = NULL;

	for (i = 0; i < RECONOS_NUM_SWSLOTS; i++) {
		swslot = &_swslots[i];

		if (!swslot->rt)
			break;
	}

	return swslot;
}


/* == ReconOS delegate ================================================= */

#if 0
#define RESOURCE_CHECK_TYPE(p_handle, p_type, p_res) {\
	int __i;\
	for (__i = 0; __i < slot->rt->resource_count; __i++) {\
		if (slot->rt->resources[__i].id == (p_handle))\
			break;\
	}\
	if (__i == slot->rt->resource_count)\
		panic("[reconos-dt-%d] "\
		      "ERROR: resource %d not found\n", slot->id, (p_handle));\
	\
	if (slot->rt->resources[(__i)].type != (p_type)) {\
		panic("[reconos-dt-%d] "\
		      "ERROR: wrong resource type\n", slot->id);\
	}\
	(p_res) = slot->rt->resources[(__i)].ptr;}

#define SYSCALL_NONBLOCK(p_call)\
	if (slot->dt_flags & DELEGATE_FLAG_PAUSE_SYSCALLS) {\
		debug("[reconos-dt-%d] "\
		      "interrupted in nonblocking syscall\n", slot->id);\
		goto intr;\
	}\
	p_call;

#define SYSCALL_BLOCK(p_call)\
	if (slot->dt_flags & DELEGATE_FLAG_PAUSE_SYSCALLS) {\
		debug("[reconos-dt-%d] "\
		      "interrupted before blocking syscall\n", slot->id);\
		goto intr;\
	}\
	slot->dt_state = DELEGATE_STATE_BLOCKED_SYSCALL;\
	if ((p_call) < 0) {\
		debug("[reconos-dt-%d] "\
		      "interrupted in blocking syscall\n", slot->id);\
		goto intr;\
	}\
	slot->dt_state = DELEGATE_STATE_PROCESSING;

/*
 * Delegate function: Get initialization data
 *   Command: OSIF_CMD_GET_INIT_DATA
 *
 *   slot - pointer to the hardware slot
 */
static inline int dt_get_init_data(struct hwslot *slot) {
	reconos_osif_write(slot->osif, (uint32_t)(slot->rt->init_data));

	return 0;
}

/*
 * Delegate function: Post Semaphore
 *   Command: OSIF_CMD_SEM_POST
 *   Syscall: sem_post
 *
 *   slot - pointer to the hardware slot
 */
static inline int dt_sem_post(struct hwslot *slot) {
	int handle, ret;
	sem_t *res;

	handle = reconos_osif_read(slot->osif);
	RESOURCE_CHECK_TYPE(handle, RECONOS_RESOURCE_TYPE_SEM, res);

	debug("[reconos-dt-%d] (sem_post on %d) ...\n", slot->id, handle);
	SYSCALL_NONBLOCK(ret = sem_post(res));
	debug("[reconos-dt-%d] (sem_post on %d) done\n", slot->id, handle);

	reconos_osif_write(slot->osif, (uint32_t)ret);

	return 0;

intr:
	return -1;
}

/*
 * Delegate function: Acquire Semaphore
 *   Command: OSIF_CMD_SEM_WAIT
 *   Syscall: sem_wait
 *
 *   slot - pointer to the hardware slot
 */
static inline int dt_sem_wait(struct hwslot *slot) {
	int handle, ret;
	sem_t *res;

	handle = reconos_osif_read(slot->osif);
	RESOURCE_CHECK_TYPE(handle, RECONOS_RESOURCE_TYPE_SEM, res);

	debug("[reconos-dt-%d] (sem_wait on %d) ...\n", slot->id, handle);
	SYSCALL_BLOCK(ret = sem_wait(res));
	debug("[reconos-dt-%d] (sem_wait on %d) done\n", slot->id, handle);

	reconos_osif_write(slot->osif, (uint32_t)ret);

	return 0;

intr:
	return -1;
}

/*
 * Delegate function: Lock a mutex
 *   Command: OSIF_CMD_MUTEX_LOCK
 *   Syscall: pthread_mutex_lock
 *
 *   slot - pointer to the hardware slot
 */
static inline int dt_mutex_lock(struct hwslot *slot) {
	int handle, ret;
	pthread_mutex_t *res;

	handle = reconos_osif_read(slot->osif);
	RESOURCE_CHECK_TYPE(handle, RECONOS_RESOURCE_TYPE_MUTEX, res);

	debug("[reconos-dt-%d] (mutex_lock on %d) ...\n", slot->id, handle);
	SYSCALL_BLOCK(ret = pthread_mutex_lock(res));
	debug("[reconos-dt-%d] (mutex_lock on %d) done\n", slot->id, handle);

	reconos_osif_write(slot->osif, (uint32_t)ret);

	return 0;

intr:
	return -1;
}

/*
 * Delegate function: Unlock a mutex
 *   Command: OSIF_CMD_MUTEX_UNLOCK
 *   Syscall: pthread_mutex_unlock
 *
 *   slot - pointer to the hardware slot
 */
static inline int dt_mutex_unlock(struct hwslot *slot) {
	int handle, ret;
	pthread_mutex_t *res;

	handle = reconos_osif_read(slot->osif);
	RESOURCE_CHECK_TYPE(handle, RECONOS_RESOURCE_TYPE_MUTEX, res);

	debug("[reconos-dt-%d] (mutex_unlock on %d) ...\n", slot->id, handle);
	SYSCALL_NONBLOCK(ret = pthread_mutex_unlock(res));
	debug("[reconos-dt-%d] (mutex_unlock on %d) done\n", slot->id, handle);

	reconos_osif_write(slot->osif, (uint32_t)ret);

	return 0;

intr:
	return -1;
}

/*
 * Delegate function: Tries to lock a mutex
 *   Command: OSIF_CMD_MUTEX_TRYLOCK
 *   Syscall: pthread_mutex_trylock
 *
 *   slot - pointer to the hardware slot
 */
static inline uint32_t dt_mutex_trylock(struct hwslot *slot) {
	int handle, ret;
	pthread_mutex_t *res;

	handle = reconos_osif_read(slot->osif);
	RESOURCE_CHECK_TYPE(handle, RECONOS_RESOURCE_TYPE_MUTEX, res);

	debug("[reconos-dt-%d] (mutex_trylock on %d) ...\n", slot->id, handle);
	SYSCALL_NONBLOCK(ret = pthread_mutex_trylock(res));
	debug("[reconos-dt-%d] (mutex_trylock on %d) done\n", slot->id, handle);

	reconos_osif_write(slot->osif, (uint32_t)ret);

	return 0;

intr:
	return -1;
}

/*
 * Delegate function: Waits for a condition variable
 *   Command: OSIF_CMD_COND_WAIT
 *   Syscall: pthread_cond_wait
 *
 *   slot - pointer to the hardware slot
 */
static inline int dt_cond_wait(struct hwslot *slot) {
#ifndef RECONOS_MINIMAL
	int handle, handle2, ret;
	pthread_cond_t *res;
	pthread_mutex_t *res2;

	handle = reconos_osif_read(slot->osif);
	handle2 = reconos_osif_read(slot->osif);
	RESOURCE_CHECK_TYPE(handle, RECONOS_RESOURCE_TYPE_COND, res);
	RESOURCE_CHECK_TYPE(handle2, RECONOS_RESOURCE_TYPE_MUTEX, res2);

	debug("[reconos-dt-%d] (cond_wait on %d) ...\n", slot->id, handle);
	SYSCALL_BLOCK(ret = pthread_cond_wait(res,res2));
	debug("[reconos-dt-%d] (cond_wait on %d) done\n", slot->id, handle);

	reconos_osif_write(slot->osif, (uint32_t)ret);

	return 0;
#else
	panic("[reconos-dt-%d] ERROR: (cond_wait on %d) not supported\n", slot->id, handle);

	return 0;
#endif

intr:
	return -1;
}

/*
 * Delegate function: Signals a condition variable
 *   Command: OSIF_CMD_COND_SIGNAL
 *   Syscall: pthread_cond_signal
 *
 *   slot - pointer to the hardware slot
 */
static inline int dt_cond_signal(struct hwslot *slot) {
#ifndef RECONOS_MINIMAL
	int handle, ret;
	pthread_cond_t *res;

	handle = reconos_osif_read(slot->osif);
	RESOURCE_CHECK_TYPE(handle, RECONOS_RESOURCE_TYPE_COND, res);

	debug("[reconos-dt-%d] (cond_signal on %d) ...\n", slot->id, handle);
	SYSCALL_NONBLOCK(ret = pthread_cond_signal(res));
	debug("[reconos-dt-%d] (cond_signal on %d) done\n", slot->id, handle);

	reconos_osif_write(slot->osif, (uint32_t)ret);

	return 0;
#else
	panic("[reconos-dt-%d] ERROR: (cond_signal on %d) not supported\n", slot->id, handle);

	return 0;
#endif

intr:
	return -1;
}

/*
 * Delegate function: Broadcasts a condition variable
 *   Command: OSIF_CMD_COND_BROADCAST
 *   Syscall: pthread_cond_broadcast
 *
 *   slot - pointer to the hardware slot
 */
static inline uint32_t dt_cond_broadcast(struct hwslot *slot) {
#ifndef RECONOS_MINIMAL
	int handle, ret;
	pthread_cond_t *res;

	handle = reconos_osif_read(slot->osif);
	RESOURCE_CHECK_TYPE(handle, RECONOS_RESOURCE_TYPE_COND, res);

	debug("[reconos-dt-%d] (cond_broadcast on %d) ...\n", slot->id, handle);
	SYSCALL_NONBLOCK(ret = pthread_cond_broadcast(res));
	debug("[reconos-dt-%d] (cond_broadcast on %d) done\n", slot->id, handle);

	reconos_osif_write(slot->osif, (uint32_t)ret);

	return 0;
#else
	panic("[reconos-dt-%d] ERROR: (cond_broadcast on %d) not supported\n", slot->id, handle);

	return 0;
#endif

intr:
	return -1;
}

/*
 * Delegate function: Gets a message from mbox
 *   Command: OSIF_CMD_MBOX_GET
 *   Syscall: mbox_get
 *
 *   slot - pointer to the hardware slot
 */
static inline int dt_mbox_get(struct hwslot *slot) {
	int handle, ret;
	uint32_t msg;
	struct mbox *res;

	handle = reconos_osif_read(slot->osif);
	RESOURCE_CHECK_TYPE(handle, RECONOS_RESOURCE_TYPE_MBOX, res);

	debug("[reconos-dt-%d] (mbox_get on %d) ...\n", slot->id, handle);
	SYSCALL_BLOCK(ret = mbox_get_interruptible(res, &msg));
	debug("[reconos-dt-%d] (mbox_get on %d) done\n", slot->id, handle);

	reconos_osif_write(slot->osif, msg);

	return 0;

intr:
	return -1;
}

/*
 * Delegate function: Puts a message into mbox
 *   Command: OSIF_CMD_MBOX_PUT
 *   Syscall: mbox_put
 *
 *   slot - pointer to the hardware slot
 */
static inline int dt_mbox_put(struct hwslot *slot) {
	int handle, ret;
	uint32_t arg0;
	struct mbox *res;

	handle = reconos_osif_read(slot->osif);
	RESOURCE_CHECK_TYPE(handle, RECONOS_RESOURCE_TYPE_MBOX, res);

	arg0 = reconos_osif_read(slot->osif);

	debug("[reconos-dt-%d] (mbox_put on %d) ...\n", slot->id, handle);
	SYSCALL_NONBLOCK(ret = mbox_put(res, arg0));
	debug("[reconos-dt-%d] (mbox_put on %d) done\n", slot->id, handle);

	reconos_osif_write(slot->osif, (uint32_t)ret);

	return 0;

intr:
	return -1;
}

/*
 * Delegate function: Tries to get a message from mbox
 *   Command: OSIF_CMD_MBOX_TRYGET
 *   Syscall: mbox_tryget
 *
 *   slot - pointer to the hardware slot
 */
static inline int dt_mbox_tryget(struct hwslot *slot) {
	int handle, ret;
	uint32_t data;
	struct mbox *res;

	handle = reconos_osif_read(slot->osif);
	RESOURCE_CHECK_TYPE(handle, RECONOS_RESOURCE_TYPE_MBOX, res);

	debug("[reconos-dt-%d] (mbox_tryget on %d) ...\n", slot->id, handle);
	SYSCALL_NONBLOCK(ret = mbox_tryget(res, &data));
	reconos_osif_write(slot->osif, data);
	debug("[reconos-dt-%d] (mbox_tryget on %d) done\n", slot->id, handle);

	reconos_osif_write(slot->osif, (uint32_t)ret);

	return 0;

intr:
	return -1;
}

/*
 * Delegate function: Tries to put a message into mbox
 *   Command: OSIF_CMD_MBOX_TYPUT
 *   Syscall: mbox_tryput
 *
 *   slot - pointer to the hardware slot
 */
static inline int dt_mbox_tryput(struct hwslot *slot) {
	int handle, ret;
	uint32_t arg0;
	struct mbox *res;

	handle = reconos_osif_read(slot->osif);
	RESOURCE_CHECK_TYPE(handle, RECONOS_RESOURCE_TYPE_MBOX, res);

	arg0 = reconos_osif_read(slot->osif);

	debug("[reconos-dt-%d] (mbox_tryput on %d) ...\n", slot->id, handle);
	SYSCALL_NONBLOCK(ret = mbox_tryput(res, arg0));
	debug("[reconos-dt-%d] (mbox_tryput on %d) done\n", slot->id, handle);

	reconos_osif_write(slot->osif, (uint32_t)ret);

	return 0;

intr:
	return -1;
}
#endif

#define RESOURCE_CHECK_TYPE(res, rtype)\
	if ((res)->type != (rtype)) {\
			panic("[reconos-rt-%d] "\
			      "ERROR: resource 0x%02x of wrong type", slot->id, res_id);\
	}

/*
 * @see header
 */
void *dt_delegate(void *arg) {
	struct hwslot *slot;
	uint32_t recv[128], send[128];
	size_t send_count;

	int i;
	struct reconos_resource *res;
	int res_id;

	slot = (struct hwslot *)arg;

#if 0
	slot->dt_state = DELEGATE_STATE_PROCESSING;
#endif

	while (1) {
#if 0
		if (slot->dt_flags & DELEGATE_FLAG_SUSPEND) {
			slot->dt_flags &= ~DELEGATE_FLAG_SUSPEND;
			reconos_proc_control_hwt_signal(_proc_control, slot->id, 1);
		}

		debug("[reconos-dt-%d] waiting for command ...\n", slot->id);
		slot->dt_state = DELEGATE_STATE_BLOCKED_OSIF;
		cmd = reconos_osif_read(slot->osif);
		slot->dt_state = DELEGATE_STATE_PROCESSING;
		debug("[reconos-dt-%d] received command 0x%x\n", slot->id, cmd);
#endif 

		reconos_osif_read(slot->osif, recv, 128 * sizeof(uint32_t));

		res_id = RECONOS_OSIF_DST(recv[0]);
		for (i = 0; i < slot->rt->resource_count; i++) {
			res = &slot->rt->resources[i];
			if (res->id == res_id) {
				break;
			}
		}
		if (i == slot->rt->resource_count) {
			panic("[reconos-rt-%d] "
			      "ERROR: resource 0x%02x not found", slot->id, res_id);
		}

		switch (recv[1] & OSIF_CMD_MASK) {
			case OSIF_CMD_MBOX_GET:
				RESOURCE_CHECK_TYPE(res, RECONOS_RESOURCE_TYPE_MBOX);
				send_count = 1;
				send[1] = mbox_get(res->ptr);
				break;

			case OSIF_CMD_MBOX_PUT:
				RESOURCE_CHECK_TYPE(res, RECONOS_RESOURCE_TYPE_MBOX);
				mbox_put(res->ptr, recv[2]);
				send_count = 1;
				send[1] = 0x00000000;
				break;

			default:
				panic("[reconos-rt-%d] "
				      "ERROR: unknown command 0x%08x received", slot->id, recv[1]);
				break;
		}

		if (send_count > 0)
		send[0] = 0xFF000000 | (slot->id << 16) | send_count;
		reconos_osif_write(slot->osif, send, (send_count + 1) * sizeof(uint32_t));

#if 0
		switch (cmd & OSIF_CMD_MASK) {
			case OSIF_CMD_MBOX_PUT:
				dt_mbox_put(slot);
				break;

			case OSIF_CMD_MBOX_TRYPUT:
				dt_mbox_tryput(slot);
				break;

			case OSIF_CMD_SEM_POST:
				dt_sem_post(slot);
				break;

			case OSIF_CMD_MUTEX_UNLOCK:
				dt_mutex_unlock(slot);
				break;

			case OSIF_CMD_COND_SIGNAL:
				dt_cond_signal(slot);
				break;

			case OSIF_CMD_COND_BROADCAST:
				dt_cond_broadcast(slot);
				break;

			case OSIF_CMD_MBOX_GET:
				dt_mbox_get(slot);
				break;

			case OSIF_CMD_MBOX_TRYGET:
				dt_mbox_tryget(slot);
				break;

			case OSIF_CMD_SEM_WAIT:
				dt_sem_wait(slot);
				break;

			case OSIF_CMD_MUTEX_LOCK:
				dt_mutex_lock(slot);
				break;

			case OSIF_CMD_MUTEX_TRYLOCK:
				dt_mutex_trylock(slot);
				break;

			case OSIF_CMD_COND_WAIT:
				dt_cond_wait(slot);
				break;

			case OSIF_CMD_THREAD_GET_INIT_DATA:
				dt_get_init_data(slot);
				break;

			case OSIF_CMD_THREAD_EXIT:
				reconos_proc_control_hwt_reset(_proc_control, slot->id, 1);
				reconos_proc_control_hwt_signal(_proc_control, slot->id, 0);
				slot->dt_flags = 0;
				sem_post(&slot->dt_exit);
				break;

			case OSIF_CMD_THREAD_GET_STATE_ADDR:
				slot->dt_flags &= ~DELEGATE_FLAG_PAUSE_SYSCALLS;
				reconos_osif_write(slot->osif, (uint32_t)(slot->rt->state_data));
				break;

			case OSIF_CMD_THREAD_CLEAR_SIGNAL:
				reconos_proc_control_hwt_signal(_proc_control, slot->id, 0);
				break;

			case OSIF_INTERRUPTED:
				break;

			default:
				panic("[reconos-dt-%d] ERROR received unknown command 0x%08x\n", slot->id, cmd);
				break;
		}

		debug("[reconos-dt-%d] executed command 0x%x\n", slot->id, cmd);
#endif
	}

	return NULL;
}
