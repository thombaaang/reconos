/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Thread library header file
 *
 *   project:      ReconOS
 *   author:       Andreas Agne, University of Paderborn
 *                 Christoph Rüthing, University of Paderborn
 *   description:  Auto-generated thread specific header file including
 *                 resource definitions and helper macros.
 *
 * ======================================================================
 */

<<reconos_preproc>>

#ifndef RECONOS_THREAD_H
#define RECONOS_THREAD_H

#include "reconos.h"
#include "reconos_app.h"


/* == Thread resources ================================================= */

/*
 * Definiton of resources for ReconOS thread. Only use these resource
 * definition via the appropriate call functions.
 */
<<generate for RESOURCES>>
extern struct reconos_resource *<<NameLower>>;
<<end generate>>


/* == Thread helper macros ============================================= */

/*
 * Definition of the entry function to the ReconOS thread. Every ReconOS
 * thread should be defined using this macro:
 *
 *   THREAD_ENTRY() {
 *     // thread code here
 *   }
 }
 */
#define THREAD_ENTRY(...)\
	void rt_<<NAME>>(void *data)

/*
 * Initializes the thread and reads from the osif the resume status.
 */
#define THREAD_INIT()\
 	struct reconos_thread  *__rt = (struct reconos_thread *)data;\
	int __run_id = reconos_thread_get_runid(__rt);

/*
 * Terminates the current ReconOS thread.
 */
#define THREAD_EXIT()\
	pthread_exit(0);

/*
 * Reads the signal value.
 */
#define THREAD_SIGNAL()\
	reconos_thread_swslot_signal(__rt)

#endif /* RECONOS_THREAD_H */