/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Application library
 *
 *   project:      ReconOS
 *   author:       Andreas Agne, University of Paderborn
 *                 Christoph Rüthing, University of Paderborn
 *   description:  Auto-generated application specific header file
 *                 including definitions of all resources and functions
 *                 to instantiate resources and threads automatically.
 *
 * ======================================================================
 */

<<reconos_preproc>>

#include "reconos_app.h"

#include "reconos.h"
#include "utils.h"

/* == Application resources ============================================ */

/*
 * @see header
 */
<<generate for RESOURCES(Type == "mbox" and Mode == "sw")>>
struct mbox <<NameLower>>_s;
struct mbox *<<NameLower>>_ptr = &<<NameLower>>_s;
<<end generate>>

<<generate for RESOURCES(Type == "sem" and Mode == "sw")>>
sem_t <<NameLower>>_s;
sem_t *<<NameLower>>_ptr = &<<NameLower>>_s;
<<end generate>>

<<generate for RESOURCES(Type == "mutex" and Mode == "sw")>>
pthread_mutex_t <<NameLower>>_s;
pthread_mutex_t *<<NameLower>>_ptr = &<<NameLower>>_s;
<<end generate>>

<<generate for RESOURCES(Type == "pipe" and Mode == "sw")>>
struct pipe <<NameLower>>_s;
struct pipe *<<NameLower>>_ptr = &<<NameLower>>_s;
<<end generate>>

<<generate for RESOURCES>>
struct reconos_resource <<NameLower>>_res;
struct reconos_resource *<<NameLower>> = &<<NameLower>>_res;
<<end generate>>


/* == Application functions ============================================ */

/*
 * @see header
 */
void reconos_app_init() {
	<<generate for RESOURCES(Type == "mbox" and Mode == "sw")>>
	mbox_init(<<NameLower>>_ptr, <<Args>>);
	<<end generate>>

	<<generate for RESOURCES(Type == "sem" and Mode == "sw")>>
	sem_init(<<NameLower>>_ptr, <<Args>>);
	<<end generate>>

	<<generate for RESOURCES(Type == "mutex" and Mode == "sw")>>
	pthread_mutex_init(<<NameLower>>_ptr, NULL);
	<<end generate>>

	<<generate for RESOURCES(Type == "cond" and Mode == "sw")>>
	pthread_cond_init(<<NameLower>>_ptr, NULL);
	<<end generate>>

	<<generate for RESOURCES(Type == "pipe" and Mode == "sw")>>
	pipe_init(<<NameLower>>_ptr);
	<<end generate>>

	<<generate for RESOURCES(Mode == "sw")>>
	reconos_resource_init(&<<NameLower>>_res, <<Id|0x{:02x}>>,
	                      RECONOS_RESOURCE_TYPE_<<TypeUpper>>, RECONOS_RESOURCE_MODE_SW,
	                      &<<NameLower>>_s);
	<<end generate>>
	<<generate for RESOURCES(Mode == "hw")>>
	reconos_resource_init(&<<NameLower>>_res, <<Id|0x{:02x}>>,
	                      RECONOS_RESOURCE_TYPE_<<TypeUpper>>, RECONOS_RESOURCE_MODE_HW,
	                      NULL);
	<<end generate>>
}

/*
 * @see header
 */
void reconos_app_cleanup() {
	<<generate for RESOURCES(Type == "mbox" and Mode == "sw")>>
	mbox_destroy(<<NameLower>>_ptr);
	<<end generate>>

	<<generate for RESOURCES(Type == "sem" and Mode == "sw")>>
	sem_destroy(<<NameLower>>_ptr);
	<<end generate>>

	<<generate for RESOURCES(Type == "mutex" and Mode == "sw")>>
	pthread_mutex_destroy(<<NameLower>>_ptr);
	<<end generate>>

	<<generate for RESOURCES(Type == "cond" and Mode == "sw")>>
	pthread_cond_destroy(<<NameLower>>_ptr, NULL);
	<<end generate>>

	<<generate for RESOURCES(Type == "pipe" and Mode == "sw")>>
	pipe_destroy(<<NameLower>>_ptr);
	<<end generate>>
}

/*
 * Empty software thread if no software specified
 *
 *   data - pointer to ReconOS thread
 */
void *swt_idle(void *data) {
	pthread_exit(0);
}

<<generate for THREADS>>

struct reconos_resource *resources_<<Name>>[] = {<<Resources>>};

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_hwt_<<Name>>() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	reconos_thread_init(rt, "<<Name>>", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setresourcepointers(rt, resources_<<Name>>, <<ResourceCount>>);
	reconos_thread_create(rt, RECONOS_THREAD_MODE_HW);

	return rt;
}

extern void *rt_<<Name>>(void *data);

/*
 * @see header
 */
struct reconos_thread *reconos_thread_create_swt_<<Name>>() {
	struct reconos_thread *rt = (struct reconos_thread *)malloc(sizeof(struct reconos_thread));
	if (!rt) {
		panic("[reconos-core] ERROR: failed to allocate memory for thread\n");
	}

	reconos_thread_init(rt, "<<Name>>", 0);
	reconos_thread_setinitdata(rt, 0);
	reconos_thread_setresourcepointers(rt, resources_<<Name>>, <<ResourceCount>>);
	reconos_thread_setswentry(rt, rt_<<Name>>);
	reconos_thread_create(rt, RECONOS_THREAD_MODE_SW);

	return rt;
}


/*
 * @see header
 */
void reconos_thread_destroy_<<Name>>(struct reconos_thread *rt) {
	// not implemented yet
}
<<end generate>>