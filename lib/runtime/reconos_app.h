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

#ifndef RECONOS_APP_H
#define RECONOS_APP_H

#include "mbox.h"
#include "pipe.h"

#include <pthread.h>
#include <semaphore.h>

/* == Application resources ============================================ */

/*
 * Definition of different resources of the application.
 *
 *   mbox  - mailbox (struct mbox)
 *   sem   - semaphore (sem_t)
 *   mutex - mutex (pthread_mutex)
 *   cond  - condition variable (pthread_cond)
 *   pipe  - pipe (struct pipe)
 */
<<generate for RESOURCES(Type == "mbox" and Mode == "sw")>>
extern struct mbox <<NameLower>>_s;
extern struct mbox *<<NameLower>>_ptr;
<<end generate>>

<<generate for RESOURCES(Type == "sem" and Mode == "sw")>>
extern sem_t <<NameLower>>_s;
extern sem_t *<<NameLower>>_ptr;
<<end generate>>

<<generate for RESOURCES(Type == "mutex" and Mode == "sw")>>
extern pthread_mutex_t <<NameLower>>_s;
extern pthread_mutex_t *<<NameLower>>_ptr;
<<end generate>>

<<generate for RESOURCES(Type == "pipe" and Mode == "sw")>>
extern struct pipe <<NameLower>>_s;
extern struct pipe *<<NameLower>>_ptr;
<<end generate>>

<<generate for RESOURCES>>
extern struct reconos_resource *<<NameLower>>;
<<end generate>>


/* == Application functions ============================================ */

/*
 * Initializes the application by creating all resources.
 */
void reconos_app_init();

/*
 * Cleans up the application by destroying all resources.
 */
void reconos_app_cleanup();

<<generate for THREADS>>
<<=generate for HasHw=>>
/*
 * Creates a hardware thread in the specified slot with its associated
 * resources.
 *
 *   rt   - pointer to the ReconOS thread
 */
struct reconos_thread *reconos_thread_create_hwt_<<Name>>();

/*
 * Creates a hardware thread in the specified slot with its associated
 * resources and initializes it with init data.
 *
 *   rt   - pointer to the ReconOS thread
 *   init - initialization data
 */
struct reconos_thread *reconos_thread_createi_hwt_<<Name>>(void *init);
<<=end generate=>>

<<=generate for HasSw=>>
/*
 * Creates a software thread with its associated resources.
 *
 *   rt   - pointer to the ReconOS thread
 */
struct reconos_thread *reconos_thread_create_swt_<<Name>>();

/*
 * Creates a software thread with its associated resources and initializes
 * it with init data.
 *
 *   rt   - pointer to the ReconOS thread
 */
struct reconos_thread *reconos_thread_createi_swt_<<Name>>(void *init);
<<=end generate=>>

/*
 * Destroyes a hardware thread created.
 *
 *   rt   - pointer to the ReconOS thread
 */
void reconos_thread_destroy_<<Name>>(struct reconos_thread *rt);
<<end generate>>

#endif /* RECONOS_APP_H */