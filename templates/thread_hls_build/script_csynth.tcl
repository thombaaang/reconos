<<reconos_preproc>>

open_project hls
set_top rt_imp
add_files reconos_calls.h -cflags "-D__SYNTHESIS__ -D__RECONOS__"
add_files reconos_thread.h -cflags "-D__SYNTHESIS__ -D__RECONOS__"
add_files [ glob *.cpp ] -cflags "-D__SYNTHESIS__ -D__RECONOS__"
open_solution sol
set_part {<<PART>>}
create_clock -period <<CLKPRD>> -name default
source directives.tcl
csynth_design
exit