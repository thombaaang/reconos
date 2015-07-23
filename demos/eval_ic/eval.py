import sys
import re
import reconos.utils.shutil2 as shutil2
import Gnuplot
import subprocess

def _istwo(a):
	return a == 1 or a == 8 or a == 16 or a == 24 or a == 32

MAX = 32

data_arbiter = "Num\tSlice Reg\tLUTs\n"
data_router = "Num\tSlice Reg\tLUTs\n"

for i in range(1, MAX + 1):
	rpt_path = shutil2.join("eval" + str(i), "build.hw", "implementation", "system_map.mrp")
	with open(rpt_path, "r") as file:
		rpt = file.read()

	#      | Module               |      | Slices*                        | Slice Reg                        | LUTs
	r = r"\| \+\+\+icout_slot0\s* \| \s* \| (?P<slices>[0-9]*)/[0-9]* \s* \| (?P<slicereg>[0-9]*)/[0-9]* \s* \| (?P<luts>[0-9]*)/[0-9]* \s* \|"
	m = re.search(r, rpt)
	data_arbiter += (str(i) if _istwo(i) else "") + "\t" + m.group("slicereg") + "\t" + m.group("luts") + "\n"

	r = r"\| \+\+\+icin_slot0\s* \| \s* \| (?P<slices>[0-9]*)/[0-9]* \s* \| (?P<slicereg>[0-9]*)/[0-9]* \s* \| (?P<luts>[0-9]*)/[0-9]* \s* \|"
	m = re.search(r, rpt)
	data_router += (str(i) if _istwo(i) else "") + " \t" + m.group("slicereg") + "\t" + m.group("luts") + "\n"

with open("data_arbiter.dat", "w") as file:
	file.write(data_arbiter)

with open("data_router.dat", "w") as file:
	file.write(data_router)

print(data_router)

gp = Gnuplot.Gnuplot()
gp("set terminal pdf font 'Arial,12' size 7.3cm,4cm linewidth 1")
gp("set datafile separator '\t'")
gp("set style data histograms")
gp("set style histogram rowstacked")
gp("set xlabel '# Ports'")
gp("set ylabel '# Resources'")
gp("set style fill solid border -1")
gp("set key autotitle columnheader")
gp("set boxwidth 0.5")

gp("set xtics 100")
gp("set yrange [0:900]")

gp("set output 'eval_arbiter.pdf'")
gp.plot("'data_arbiter.dat' using 2:xtic(1) linecolor rgb'black', '' using 3 linecolor rgb'white'")
gp("set output 'eval_router.pdf'")
gp.plot("'data_router.dat' using 2:xtic(1) linecolor rgb'black', '' using 3 linecolor rgb'white'")

#subprocess.call("epstopdf eval_arbiter.ps", shell=True)
#subprocess.call("epstopdf eval_router.ps", shell=True)
