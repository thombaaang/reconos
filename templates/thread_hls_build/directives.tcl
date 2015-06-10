<<reconos_preproc>>

set_directive_interface -mode ap_fifo "rt_imp" osif_sw2hw
set_directive_interface -mode ap_fifo "rt_imp" osif_hw2sw
<<generate for MEM>>
set_directive_interface -mode ap_fifo "rt_imp" memif_hwt2mem
set_directive_interface -mode ap_fifo "rt_imp" memif_mem2hwt
<<end generate>>
set_directive_interface -mode ap_ctrl_none "rt_imp"