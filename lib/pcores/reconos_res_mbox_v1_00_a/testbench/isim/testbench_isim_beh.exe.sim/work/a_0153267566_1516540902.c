/**********************************************************************/
/*   ____  ____                                                       */
/*  /   /\/   /                                                       */
/* /___/  \  /                                                        */
/* \   \   \/                                                       */
/*  \   \        Copyright (c) 2003-2009 Xilinx, Inc.                */
/*  /   /          All Right Reserved.                                 */
/* /---/   /\                                                         */
/* \   \  /  \                                                      */
/*  \___\/\___\                                                    */
/***********************************************************************/

/* This file is designed for use with ISim build 0xfbc00daa */

#define XSI_HIDE_SYMBOL_SPEC true
#include "xsi.h"
#include <memory.h>
#ifdef __GNUC__
#include <stdlib.h>
#else
#include <malloc.h>
#define alloca _alloca
#endif
static const char *ng0 = "/home/christoph/Documents/github/reconos/lib/pcores/reconos_res_mbox_v1_00_a/testbench/FIFO_top_queue_get_V_V.vhd";
extern char *IEEE_P_3620187407;

int ieee_p_3620187407_sub_5109402382352621412_3965413181(char *, char *, char *);


static void work_a_0153267566_1516540902_p_0(char *t0)
{
    char t17[16];
    char t19[32];
    unsigned char t1;
    char *t2;
    unsigned char t3;
    char *t4;
    char *t5;
    unsigned char t6;
    unsigned char t7;
    char *t8;
    unsigned char t9;
    unsigned char t10;
    char *t11;
    char *t12;
    unsigned int t13;
    unsigned int t14;
    unsigned int t15;
    char *t16;
    char *t18;
    char *t20;
    char *t21;
    int t22;
    unsigned int t23;
    char *t24;
    int t25;
    char *t26;
    char *t27;
    char *t28;
    char *t29;

LAB0:    xsi_set_current_line(34, ng0);
    t2 = (t0 + 992U);
    t3 = xsi_signal_has_event(t2);
    if (t3 == 1)
        goto LAB5;

LAB6:    t1 = (unsigned char)0;

LAB7:    if (t1 != 0)
        goto LAB2;

LAB4:
LAB3:    t2 = (t0 + 3912);
    *((int *)t2) = 1;

LAB1:    return;
LAB2:    xsi_set_current_line(35, ng0);
    t4 = (t0 + 1352U);
    t8 = *((char **)t4);
    t9 = *((unsigned char *)t8);
    t10 = (t9 == (unsigned char)3);
    if (t10 != 0)
        goto LAB8;

LAB10:
LAB9:    goto LAB3;

LAB5:    t4 = (t0 + 1032U);
    t5 = *((char **)t4);
    t6 = *((unsigned char *)t5);
    t7 = (t6 == (unsigned char)3);
    t1 = t7;
    goto LAB7;

LAB8:    xsi_set_current_line(36, ng0);
    t4 = (t0 + 1192U);
    t11 = *((char **)t4);
    t4 = (t0 + 1832U);
    t12 = *((char **)t4);
    t13 = (0 - 0);
    t14 = (t13 * 8U);
    t15 = (0 + t14);
    t4 = (t12 + t15);
    t18 = (t0 + 4544);
    t20 = (t19 + 0U);
    t21 = (t20 + 0U);
    *((int *)t21) = 0;
    t21 = (t20 + 4U);
    *((int *)t21) = 31;
    t21 = (t20 + 8U);
    *((int *)t21) = 1;
    t22 = (31 - 0);
    t23 = (t22 * 1);
    t23 = (t23 + 1);
    t21 = (t20 + 12U);
    *((unsigned int *)t21) = t23;
    t21 = (t19 + 16U);
    t24 = (t21 + 0U);
    *((int *)t24) = 7;
    t24 = (t21 + 4U);
    *((int *)t24) = 0;
    t24 = (t21 + 8U);
    *((int *)t24) = -1;
    t25 = (0 - 7);
    t23 = (t25 * -1);
    t23 = (t23 + 1);
    t24 = (t21 + 12U);
    *((unsigned int *)t24) = t23;
    t16 = xsi_base_array_concat(t16, t17, t18, (char)109, t11, (char)97, t4, t19, (char)101);
    t24 = (t0 + 4008);
    t26 = (t24 + 56U);
    t27 = *((char **)t26);
    t28 = (t27 + 56U);
    t29 = *((char **)t28);
    memcpy(t29, t16, 264U);
    xsi_driver_first_trans_fast(t24);
    goto LAB9;

}

static void work_a_0153267566_1516540902_p_1(char *t0)
{
    char *t1;
    char *t2;
    char *t3;
    int t4;
    int t5;
    unsigned int t6;
    unsigned int t7;
    unsigned int t8;
    char *t9;
    char *t10;
    char *t11;
    char *t12;
    char *t13;
    char *t14;
    char *t15;

LAB0:    xsi_set_current_line(41, ng0);

LAB3:    t1 = (t0 + 1832U);
    t2 = *((char **)t1);
    t1 = (t0 + 1512U);
    t3 = *((char **)t1);
    t1 = (t0 + 7264U);
    t4 = ieee_p_3620187407_sub_5109402382352621412_3965413181(IEEE_P_3620187407, t3, t1);
    t5 = (t4 - 0);
    t6 = (t5 * 1);
    xsi_vhdl_check_range_of_index(0, 32, 1, t4);
    t7 = (8U * t6);
    t8 = (0 + t7);
    t9 = (t2 + t8);
    t10 = (t0 + 4072);
    t11 = (t10 + 56U);
    t12 = *((char **)t11);
    t13 = (t12 + 56U);
    t14 = *((char **)t13);
    memcpy(t14, t9, 8U);
    xsi_driver_first_trans_fast_port(t10);

LAB2:    t15 = (t0 + 3928);
    *((int *)t15) = 1;

LAB1:    return;
LAB4:    goto LAB2;

}


extern void work_a_0153267566_1516540902_init()
{
	static char *pe[] = {(void *)work_a_0153267566_1516540902_p_0,(void *)work_a_0153267566_1516540902_p_1};
	xsi_register_didat("work_a_0153267566_1516540902", "isim/testbench_isim_beh.exe.sim/work/a_0153267566_1516540902.didat");
	xsi_register_executes(pe);
}
