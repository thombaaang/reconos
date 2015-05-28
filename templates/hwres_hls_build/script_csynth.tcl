<<reconos_preproc>>

open_project hls
set_top hwres_imp
add_files [ glob *.cpp ]
open_solution sol
set_part {<<PART>>}
create_clock -period <<CLKPRD>> -name default
csynth_design
exit