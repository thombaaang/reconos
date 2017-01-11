/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Linux Driver - ReconOS
 *
 *   project:      ReconOS
 *   author:       Christoph Rüthing <christoph@muppetnet.net>
 *   description:  Public definitions for the osif driver module.
 *
 * ======================================================================
 */

#ifndef RECONOS_LINUX_OSIF_H__
#define RECONOS_LINUX_OSIF_H__

#include <linux/ioctl.h>

#define RECONOS_LINUX_OSIF_IOC_MAGIC 'k'

/*
 * IOCTL definitions for osif
 *
 *   set_mask - set mask for arbitration
 *   set_bits - set ctrl for arbitration
 */
#define RECONOS_OSIF_SET_MASK  _IOW(RECONOS_LINUX_OSIF_IOC_MAGIC, 20, uint32_t)
#define RECONOS_OSIF_SET_BITS  _IOW(RECONOS_LINUX_OSIF_IOC_MAGIC, 21, uint32_t)

#endif /* RECONOS_LINUX_OSIF_H__ */
