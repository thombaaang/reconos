/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Linux Driver - ReconOS - OSIF (AXI FIFO)
 *
 *   project:      ReconOS
 *   author:       Andreas Agne, University of Paderborn
 *                 Daniel Borkmann, ETH Zürich
 *                 Sebastian Meisner, University of Paderborn
 *                 Christoph Rüthing, University of Paderborn

 *   description:  Driver for the AXI-FIFO used to communicate to the
 *                 hardware-threads (successor of fsl_driver).
 *
 * ======================================================================
 */

#ifndef RECONOS_DRV_OSIF_H
#define RECONOS_DRV_OSIF_H

#include "reconos.h"

/*
 * Initialization function called by module loading
 *
 *   no parameters
 *
 *   @returns -1 on failure, otherwise 0
 */
extern int osif_init(void);

/*
 * Exit function called by module unloading
 *
 *   no parameters
 *
 *   @returns always 0
 */
extern int osif_exit(void);

/*
 * Resets the internal state of the osif in case of a system reset.
 *
 *   no parameters
 */
extern void osif_reset(void);

#endif /* RECONOS_DRV_OSIF_H */
