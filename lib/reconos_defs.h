/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Constants
 *
 *   project:      ReconOS
 *   author:       Andreas Agne, University of Paderborn
 *                 Christoph Rüthing, University of Paderborn
 *   description:  Global header file with constant definitions.
 *
 * ======================================================================
 */

#ifndef RECONOS_DEFS_H
#define RECONOS_DEFS_H

/* == ReconOS thread =================================================== */

/*
 * Definition of thread types
 *
 *   RECONOS_THREAD_SW - software thread
 *   RECONOS_THREAD_HW - hardware thread
 */
#define RECONOS_THREAD_MODE_SW 0x01
#define RECONOS_THREAD_MODE_HW 0x02

/*
 * Definition of the thread states
 *
 *   RECONOS_THREAD_STATE_STOPED     - not created yet
 *   RECONOS_THREAD_STATE_RUNNING_HW - executing as a hardware thread
 *   RECONOS_THREAD_STATE_RUNNING_SW - executing as a software thread
 *   RECONOS_THREAD_STATE_SUSPENDED  - suspendend and ready for sheduling
 *   RECONOS_THREAD_STATE_SUSPENDING - currently suspending and saving state
 */
#define RECONOS_THREAD_STATE_INIT         0x01
#define RECONOS_THREAD_STATE_STOPED       0x02
#define RECONOS_THREAD_STATE_RUNNING_HW   0x04
#define RECONOS_THREAD_STATE_RUNNING_SW   0x08
#define RECONOS_THREAD_STATE_SUSPENDED    0x10
#define RECONOS_THREAD_STATE_SUSPENDING   0x20


/* == ReconOS resources ================================================== */

/*
 * Definition of the different resource parameters.
 *
 *   RECONOS_RESOURCE_TYPE_MBOX  - mailbox (struct mbox)
 *   RECONOS_RESOURCE_TYPE_SEM   - semaphore (sem_t)
 *   RECONOS_RESOURCE_TYPE_MUTEX - mutex (pthread_mutex)
 *   RECONOS_RESOURCE_TYPE_COND  - condition variable (pthread_cond)
 *
 *   RECONOS_RESOURCE_MODE_SW - software resource
 *   RECONOS_RESOURCE_MODE_HW - hardwrae resource
 */
#define RECONOS_RESOURCE_TYPE_MBOX  0x00000001
#define RECONOS_RESOURCE_TYPE_SEM   0x00000002
#define RECONOS_RESOURCE_TYPE_MUTEX 0x00000004
#define RECONOS_RESOURCE_TYPE_COND  0x00000008

#define RECONOS_RESOURCE_MODE_SW 0x00000001
#define RECONOS_RESOURCE_MODE_HW 0x00000002


/* == ReconOS osif ===================================================== */

/*
 * Definition of the osif commands
 *
 *   self-describing
 */
#define OSIF_CMD_THREAD_GET_INIT_DATA  0x000000A0
#define OSIF_CMD_THREAD_GET_STATE_ADDR 0x000000A1
#define OSIF_CMD_THREAD_EXIT           0x000000A2
#define OSIF_CMD_THREAD_YIELD          0x000000A3
#define OSIF_CMD_THREAD_CLEAR_SIGNAL   0x000000A4
#define OSIF_CMD_THREAD_GET_RUNID      0x000000A5
#define OSIF_CMD_SEM_POST              0x000000B0
#define OSIF_CMD_SEM_WAIT              0x000000B1
#define OSIF_CMD_MUTEX_LOCK            0x000000C0
#define OSIF_CMD_MUTEX_UNLOCK          0x000000C1
#define OSIF_CMD_MUTEX_TRYLOCK         0x000000C2
#define OSIF_CMD_COND_WAIT             0x000000D0
#define OSIF_CMD_COND_SIGNAL           0x000000D1
#define OSIF_CMD_COND_BROADCAST        0x000000D2
#define OSIF_CMD_MBOX_GET              0x000000F0
#define OSIF_CMD_MBOX_PUT              0x000000F1
#define OSIF_CMD_MBOX_TRYGET           0x000000F2
#define OSIF_CMD_MBOX_TRYPUT           0x000000F3
#define OSIF_CMD_MASK                  0x000000FF
#define OSIF_CMD_YIELD_MASK            0x80000000

#define OSIF_SIGNAL_THREAD_START       0x01000000
#define OSIF_SIGNAL_THREAD_RESUME      0x01000001

#define OSIF_INTERRUPTED               0x000000FF

/*
 * Definition of osif control words
 *
 *   RECONOS_OSIF_DATA_WIDTH - width of the osif
 *
 *   RECONOS_OSIF_SRC_MASK    - mask for the source in command word
 *   RECONOS_OSIF_DST_MASK    - mask for the destination in command word
 *   RECONOS_OSIF_LENGTH_MASK - mask for the length in command word
 *
 *   RECONOS_OSIF_SRC    - returns source id of control word
 *   RECONOS_OSIF_DST    - returns destination id of control word
 *   RECONOS_OSIF_LENGTH - returns length id of control word
 */
#define RECONOS_OSIF_DATA_WIDTH  32

#define RECONOS_OSIF_SRC_MASK    0xFF000000
#define RECONOS_OSIF_DST_MASK    0x00FF0000
#define RECONOS_OSIF_LENGTH_MASK 0x0000FFFF

#define RECONOS_OSIF_SRC(ctrl)    ((ctrl & RECONOS_OSIF_SRC_MASK) >> 24)
#define RECONOS_OSIF_DST(ctrl)    ((ctrl & RECONOS_OSIF_DST_MASK) >> 16)
#define RECONOS_OSIF_LENGTH(ctrl) ((ctrl & RECONOS_OSIF_LENGTH_MASK >> 0))


/* == ReconOS hardware ================================================= */

/*
 * Definition of the memif parameters
 *
 *   RECONOS MEMIF_CHUNK_WORDS - size of one memory request in words
 *                               (a request might be split up to meet this)
 *   RECONOS_MEMIF_CHUNK_BYTES - size of one memory reques in bytes
 *   RECONOS_MEMIF_CHUNK_MASK  - mask for chunk size
 */
#define RECONOS_MEMIF_CHUNK_WORDS 64
#define RECONOS_MEMIF_CHUNK_BYTES (MEMIF_CHUNK_WORDS * 4)
#define RECONOS_MEMIF_CHUNK_MASK  0x000000FF


/* Definition of memif control words
 *
 *   RECONOS_MEMIF_DATA_WIDTH  - width of the memif
 *
 *   RECONOS_MEMIF_OP_MASK     - mask of operation in control word
 *   RECONOS_MEMIF_LENGTH_MASK - mask of length in control word
 *
 *   RECONOS_MEMIF_OP     - returns operation if control word
 *   RECONOS_MEMIF_LENGTH - returns length if control word
 */
#define RECONOS_MEMIF_DATA_WIDTH  32

#define RECONOS_MEMIF_OP_MASK     0xFF000000
#define RECONOS_MEMIF_LENGTH_MASK 0x00FFFFFF

#define RECONOS_MEMIF_OP(ctrl)     (ctrl & RECONOS_MEMIF_OP_MASK >> 24)
#define RECONOS_MEMIF_LENGTH(ctrl) (ctrl & RECONOS_MEMIF_LENGTH_MASK)

/*
 * Definition of the memif commands
 *
 *   self-describing
 */
#define RECONOS_MEMIF_CMD_READ 0x00
#define RECONOS_MEMIF_CMD_WRITE 0xF0

#endif /* RECONOS_DEFS_H */