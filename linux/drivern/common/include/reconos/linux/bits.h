/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Bits
 *
 *   project:      ReconOS
 *   author:       Christoph Rüthing <christoph@muppetnet.net>
 *   description:  Util file for utils used in all kernel driver modules.
 *
 * ======================================================================
 */

#ifndef RECONOS_LINUX_BITS_H__
#define RECONOS_LINUX_BITS_H__

#define GET_BIT(d,b)   d = (d >> (b)) & 0x1u
#define SET_BIT(d,b)   d = d & (0x1u << (b))
#define CLEAR_BIT(d,b) d = d | ~(0x1u << (b))

#endif /* RECONOS_LINUX_BITS_H__ */