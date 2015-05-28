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
extern char *IEEE_P_2592010699;
extern char *IEEE_P_3620187407;

unsigned char ieee_p_2592010699_sub_3488768496604610246_503743352(char *, unsigned char , unsigned char );
char *ieee_p_3620187407_sub_2255506239096166994_3965413181(char *, char *, char *, char *, int );
char *ieee_p_3620187407_sub_2255506239096238868_3965413181(char *, char *, char *, char *, int );
unsigned char ieee_p_3620187407_sub_970019341842465249_3965413181(char *, char *, char *, int );


static void work_a_0415362039_1516540902_p_0(char *t0)
{
    char *t1;
    char *t2;
    unsigned char t3;
    char *t4;
    char *t5;
    char *t6;
    char *t7;
    char *t8;

LAB0:    xsi_set_current_line(91, ng0);

LAB3:    t1 = (t0 + 3432U);
    t2 = *((char **)t1);
    t3 = *((unsigned char *)t2);
    t1 = (t0 + 7224);
    t4 = (t1 + 56U);
    t5 = *((char **)t4);
    t6 = (t5 + 56U);
    t7 = *((char **)t6);
    *((unsigned char *)t7) = t3;
    xsi_driver_first_trans_fast_port(t1);

LAB2:    t8 = (t0 + 7048);
    *((int *)t8) = 1;

LAB1:    return;
LAB4:    goto LAB2;

}

static void work_a_0415362039_1516540902_p_1(char *t0)
{
    char *t1;
    char *t2;
    unsigned char t3;
    char *t4;
    char *t5;
    char *t6;
    char *t7;
    char *t8;

LAB0:    xsi_set_current_line(92, ng0);

LAB3:    t1 = (t0 + 3592U);
    t2 = *((char **)t1);
    t3 = *((unsigned char *)t2);
    t1 = (t0 + 7288);
    t4 = (t1 + 56U);
    t5 = *((char **)t4);
    t6 = (t5 + 56U);
    t7 = *((char **)t6);
    *((unsigned char *)t7) = t3;
    xsi_driver_first_trans_fast_port(t1);

LAB2:    t8 = (t0 + 7064);
    *((int *)t8) = 1;

LAB1:    return;
LAB4:    goto LAB2;

}

static void work_a_0415362039_1516540902_p_2(char *t0)
{
    char *t1;
    char *t2;
    char *t3;
    char *t4;
    char *t5;
    char *t6;
    char *t7;

LAB0:    xsi_set_current_line(93, ng0);

LAB3:    t1 = (t0 + 2472U);
    t2 = *((char **)t1);
    t1 = (t0 + 7352);
    t3 = (t1 + 56U);
    t4 = *((char **)t3);
    t5 = (t4 + 56U);
    t6 = *((char **)t5);
    memcpy(t6, t2, 8U);
    xsi_driver_first_trans_fast(t1);

LAB2:    t7 = (t0 + 7080);
    *((int *)t7) = 1;

LAB1:    return;
LAB4:    goto LAB2;

}

static void work_a_0415362039_1516540902_p_3(char *t0)
{
    char *t1;
    char *t2;
    char *t3;
    char *t4;
    char *t5;
    char *t6;
    char *t7;

LAB0:    xsi_set_current_line(94, ng0);

LAB3:    t1 = (t0 + 2952U);
    t2 = *((char **)t1);
    t1 = (t0 + 7416);
    t3 = (t1 + 56U);
    t4 = *((char **)t3);
    t5 = (t4 + 56U);
    t6 = *((char **)t5);
    memcpy(t6, t2, 8U);
    xsi_driver_first_trans_fast_port(t1);

LAB2:    t7 = (t0 + 7096);
    *((int *)t7) = 1;

LAB1:    return;
LAB4:    goto LAB2;

}

static void work_a_0415362039_1516540902_p_4(char *t0)
{
    char t26[16];
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
    char *t13;
    char *t14;
    char *t15;
    char *t16;
    unsigned char t17;
    unsigned char t18;
    unsigned char t19;
    unsigned char t20;
    unsigned char t21;
    unsigned char t22;
    unsigned char t23;
    unsigned char t24;
    unsigned char t25;
    unsigned int t27;
    unsigned int t28;
    unsigned char t29;
    char *t30;
    char *t31;
    char *t32;
    char *t33;
    char *t34;
    int t35;

LAB0:    xsi_set_current_line(98, ng0);
    t2 = (t0 + 992U);
    t3 = xsi_signal_has_event(t2);
    if (t3 == 1)
        goto LAB5;

LAB6:    t1 = (unsigned char)0;

LAB7:    if (t1 != 0)
        goto LAB2;

LAB4:
LAB3:    t2 = (t0 + 7112);
    *((int *)t2) = 1;

LAB1:    return;
LAB2:    xsi_set_current_line(99, ng0);
    t4 = (t0 + 1192U);
    t8 = *((char **)t4);
    t9 = *((unsigned char *)t8);
    t10 = (t9 == (unsigned char)3);
    if (t10 != 0)
        goto LAB8;

LAB10:    xsi_set_current_line(104, ng0);
    t2 = (t0 + 1672U);
    t4 = *((char **)t2);
    t6 = *((unsigned char *)t4);
    t2 = (t0 + 1512U);
    t5 = *((char **)t2);
    t7 = *((unsigned char *)t5);
    t9 = ieee_p_2592010699_sub_3488768496604610246_503743352(IEEE_P_2592010699, t6, t7);
    t10 = (t9 == (unsigned char)3);
    if (t10 == 1)
        goto LAB17;

LAB18:    t3 = (unsigned char)0;

LAB19:    if (t3 == 1)
        goto LAB14;

LAB15:    t1 = (unsigned char)0;

LAB16:    if (t1 != 0)
        goto LAB11;

LAB13:    t2 = (t0 + 1672U);
    t4 = *((char **)t2);
    t6 = *((unsigned char *)t4);
    t2 = (t0 + 1512U);
    t5 = *((char **)t2);
    t7 = *((unsigned char *)t5);
    t9 = ieee_p_2592010699_sub_3488768496604610246_503743352(IEEE_P_2592010699, t6, t7);
    t10 = (t9 == (unsigned char)2);
    if (t10 == 1)
        goto LAB33;

LAB34:    t2 = (t0 + 3432U);
    t8 = *((char **)t2);
    t17 = *((unsigned char *)t8);
    t18 = (t17 == (unsigned char)2);
    t3 = t18;

LAB35:    if (t3 == 1)
        goto LAB30;

LAB31:    t1 = (unsigned char)0;

LAB32:    if (t1 != 0)
        goto LAB28;

LAB29:
LAB12:
LAB9:    goto LAB3;

LAB5:    t4 = (t0 + 1032U);
    t5 = *((char **)t4);
    t6 = *((unsigned char *)t5);
    t7 = (t6 == (unsigned char)3);
    t1 = t7;
    goto LAB7;

LAB8:    xsi_set_current_line(100, ng0);
    t4 = xsi_get_transient_memory(7U);
    memset(t4, 0, 7U);
    t11 = t4;
    memset(t11, (unsigned char)3, 7U);
    t12 = (t0 + 7480);
    t13 = (t12 + 56U);
    t14 = *((char **)t13);
    t15 = (t14 + 56U);
    t16 = *((char **)t15);
    memcpy(t16, t4, 7U);
    xsi_driver_first_trans_fast(t12);
    xsi_set_current_line(101, ng0);
    t2 = (t0 + 7544);
    t4 = (t2 + 56U);
    t5 = *((char **)t4);
    t8 = (t5 + 56U);
    t11 = *((char **)t8);
    *((unsigned char *)t11) = (unsigned char)2;
    xsi_driver_first_trans_fast(t2);
    xsi_set_current_line(102, ng0);
    t2 = (t0 + 7608);
    t4 = (t2 + 56U);
    t5 = *((char **)t4);
    t8 = (t5 + 56U);
    t11 = *((char **)t8);
    *((unsigned char *)t11) = (unsigned char)3;
    xsi_driver_first_trans_fast(t2);
    goto LAB9;

LAB11:    xsi_set_current_line(106, ng0);
    t2 = (t0 + 3272U);
    t14 = *((char **)t2);
    t2 = (t0 + 13024U);
    t15 = ieee_p_3620187407_sub_2255506239096238868_3965413181(IEEE_P_3620187407, t26, t14, t2, 1);
    t16 = (t26 + 12U);
    t27 = *((unsigned int *)t16);
    t28 = (1U * t27);
    t29 = (7U != t28);
    if (t29 == 1)
        goto LAB23;

LAB24:    t30 = (t0 + 7480);
    t31 = (t30 + 56U);
    t32 = *((char **)t31);
    t33 = (t32 + 56U);
    t34 = *((char **)t33);
    memcpy(t34, t15, 7U);
    xsi_driver_first_trans_fast(t30);
    xsi_set_current_line(107, ng0);
    t2 = (t0 + 3272U);
    t4 = *((char **)t2);
    t2 = (t0 + 13024U);
    t1 = ieee_p_3620187407_sub_970019341842465249_3965413181(IEEE_P_3620187407, t4, t2, 0);
    if (t1 != 0)
        goto LAB25;

LAB27:
LAB26:    xsi_set_current_line(110, ng0);
    t2 = (t0 + 7608);
    t4 = (t2 + 56U);
    t5 = *((char **)t4);
    t8 = (t5 + 56U);
    t11 = *((char **)t8);
    *((unsigned char *)t11) = (unsigned char)3;
    xsi_driver_first_trans_fast(t2);
    goto LAB12;

LAB14:    t2 = (t0 + 2312U);
    t11 = *((char **)t2);
    t20 = *((unsigned char *)t11);
    t2 = (t0 + 2152U);
    t12 = *((char **)t2);
    t21 = *((unsigned char *)t12);
    t22 = ieee_p_2592010699_sub_3488768496604610246_503743352(IEEE_P_2592010699, t20, t21);
    t23 = (t22 == (unsigned char)2);
    if (t23 == 1)
        goto LAB20;

LAB21:    t2 = (t0 + 3592U);
    t13 = *((char **)t2);
    t24 = *((unsigned char *)t13);
    t25 = (t24 == (unsigned char)2);
    t19 = t25;

LAB22:    t1 = t19;
    goto LAB16;

LAB17:    t2 = (t0 + 3432U);
    t8 = *((char **)t2);
    t17 = *((unsigned char *)t8);
    t18 = (t17 == (unsigned char)3);
    t3 = t18;
    goto LAB19;

LAB20:    t19 = (unsigned char)1;
    goto LAB22;

LAB23:    xsi_size_not_matching(7U, t28, 0);
    goto LAB24;

LAB25:    xsi_set_current_line(108, ng0);
    t5 = (t0 + 7544);
    t8 = (t5 + 56U);
    t11 = *((char **)t8);
    t12 = (t11 + 56U);
    t13 = *((char **)t12);
    *((unsigned char *)t13) = (unsigned char)2;
    xsi_driver_first_trans_fast(t5);
    goto LAB26;

LAB28:    xsi_set_current_line(113, ng0);
    t2 = (t0 + 3272U);
    t14 = *((char **)t2);
    t2 = (t0 + 13024U);
    t15 = ieee_p_3620187407_sub_2255506239096166994_3965413181(IEEE_P_3620187407, t26, t14, t2, 1);
    t16 = (t26 + 12U);
    t27 = *((unsigned int *)t16);
    t28 = (1U * t27);
    t29 = (7U != t28);
    if (t29 == 1)
        goto LAB39;

LAB40:    t30 = (t0 + 7480);
    t31 = (t30 + 56U);
    t32 = *((char **)t31);
    t33 = (t32 + 56U);
    t34 = *((char **)t33);
    memcpy(t34, t15, 7U);
    xsi_driver_first_trans_fast(t30);
    xsi_set_current_line(114, ng0);
    t2 = (t0 + 7544);
    t4 = (t2 + 56U);
    t5 = *((char **)t4);
    t8 = (t5 + 56U);
    t11 = *((char **)t8);
    *((unsigned char *)t11) = (unsigned char)3;
    xsi_driver_first_trans_fast(t2);
    xsi_set_current_line(115, ng0);
    t2 = (t0 + 3272U);
    t4 = *((char **)t2);
    t2 = (t0 + 13024U);
    t35 = (33 - 2);
    t1 = ieee_p_3620187407_sub_970019341842465249_3965413181(IEEE_P_3620187407, t4, t2, t35);
    if (t1 != 0)
        goto LAB41;

LAB43:
LAB42:    goto LAB12;

LAB30:    t2 = (t0 + 2312U);
    t11 = *((char **)t2);
    t20 = *((unsigned char *)t11);
    t2 = (t0 + 2152U);
    t12 = *((char **)t2);
    t21 = *((unsigned char *)t12);
    t22 = ieee_p_2592010699_sub_3488768496604610246_503743352(IEEE_P_2592010699, t20, t21);
    t23 = (t22 == (unsigned char)3);
    if (t23 == 1)
        goto LAB36;

LAB37:    t19 = (unsigned char)0;

LAB38:    t1 = t19;
    goto LAB32;

LAB33:    t3 = (unsigned char)1;
    goto LAB35;

LAB36:    t2 = (t0 + 3592U);
    t13 = *((char **)t2);
    t24 = *((unsigned char *)t13);
    t25 = (t24 == (unsigned char)3);
    t19 = t25;
    goto LAB38;

LAB39:    xsi_size_not_matching(7U, t28, 0);
    goto LAB40;

LAB41:    xsi_set_current_line(116, ng0);
    t5 = (t0 + 7608);
    t8 = (t5 + 56U);
    t11 = *((char **)t8);
    t12 = (t11 + 56U);
    t13 = *((char **)t12);
    *((unsigned char *)t13) = (unsigned char)2;
    xsi_driver_first_trans_fast(t5);
    goto LAB42;

}

static void work_a_0415362039_1516540902_p_5(char *t0)
{
    char *t1;
    char *t2;
    int t3;
    unsigned int t4;
    unsigned int t5;
    unsigned int t6;
    unsigned char t7;
    unsigned char t8;
    char *t9;
    char *t10;
    char *t11;
    char *t12;
    char *t13;
    char *t14;
    char *t15;
    char *t16;
    char *t17;
    int t18;
    unsigned int t19;
    unsigned int t20;
    unsigned int t21;
    char *t22;
    char *t23;
    char *t24;
    char *t25;
    char *t26;
    char *t27;

LAB0:    xsi_set_current_line(123, ng0);
    t1 = (t0 + 3272U);
    t2 = *((char **)t1);
    t3 = (6 - 6);
    t4 = (t3 * -1);
    t5 = (1U * t4);
    t6 = (0 + t5);
    t1 = (t2 + t6);
    t7 = *((unsigned char *)t1);
    t8 = (t7 == (unsigned char)3);
    if (t8 != 0)
        goto LAB3;

LAB4:
LAB5:    t16 = (t0 + 3272U);
    t17 = *((char **)t16);
    t18 = (6 - 1);
    t19 = (6 - t18);
    t20 = (t19 * 1U);
    t21 = (0 + t20);
    t16 = (t17 + t21);
    t22 = (t0 + 7672);
    t23 = (t22 + 56U);
    t24 = *((char **)t23);
    t25 = (t24 + 56U);
    t26 = *((char **)t25);
    memcpy(t26, t16, 6U);
    xsi_driver_first_trans_fast(t22);

LAB2:    t27 = (t0 + 7128);
    *((int *)t27) = 1;

LAB1:    return;
LAB3:    t9 = xsi_get_transient_memory(6U);
    memset(t9, 0, 6U);
    t10 = t9;
    memset(t10, (unsigned char)2, 6U);
    t11 = (t0 + 7672);
    t12 = (t11 + 56U);
    t13 = *((char **)t12);
    t14 = (t13 + 56U);
    t15 = *((char **)t14);
    memcpy(t15, t9, 6U);
    xsi_driver_first_trans_fast(t11);
    goto LAB2;

LAB6:    goto LAB2;

}

static void work_a_0415362039_1516540902_p_6(char *t0)
{
    char *t1;
    char *t2;
    unsigned char t3;
    char *t4;
    unsigned char t5;
    unsigned char t6;
    char *t7;
    unsigned char t8;
    unsigned char t9;
    char *t10;
    char *t11;
    char *t12;
    char *t13;
    char *t14;

LAB0:    xsi_set_current_line(124, ng0);

LAB3:    t1 = (t0 + 2312U);
    t2 = *((char **)t1);
    t3 = *((unsigned char *)t2);
    t1 = (t0 + 2152U);
    t4 = *((char **)t1);
    t5 = *((unsigned char *)t4);
    t6 = ieee_p_2592010699_sub_3488768496604610246_503743352(IEEE_P_2592010699, t3, t5);
    t1 = (t0 + 3592U);
    t7 = *((char **)t1);
    t8 = *((unsigned char *)t7);
    t9 = ieee_p_2592010699_sub_3488768496604610246_503743352(IEEE_P_2592010699, t6, t8);
    t1 = (t0 + 7736);
    t10 = (t1 + 56U);
    t11 = *((char **)t10);
    t12 = (t11 + 56U);
    t13 = *((char **)t12);
    *((unsigned char *)t13) = t9;
    xsi_driver_first_trans_fast(t1);

LAB2:    t14 = (t0 + 7144);
    *((int *)t14) = 1;

LAB1:    return;
LAB4:    goto LAB2;

}


extern void work_a_0415362039_1516540902_init()
{
	static char *pe[] = {(void *)work_a_0415362039_1516540902_p_0,(void *)work_a_0415362039_1516540902_p_1,(void *)work_a_0415362039_1516540902_p_2,(void *)work_a_0415362039_1516540902_p_3,(void *)work_a_0415362039_1516540902_p_4,(void *)work_a_0415362039_1516540902_p_5,(void *)work_a_0415362039_1516540902_p_6};
	xsi_register_didat("work_a_0415362039_1516540902", "isim/testbench_isim_beh.exe.sim/work/a_0415362039_1516540902.didat");
	xsi_register_executes(pe);
}
