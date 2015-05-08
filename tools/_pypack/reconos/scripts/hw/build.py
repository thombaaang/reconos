import reconos.utils.shutil2 as shutil2
import reconos.utils.template as template

import logging
import argparse
import subprocess

log = logging.getLogger(__name__)

def get_cmd(prj):
	return "build_hw"

def get_call(prj):
	return build_cmd

def get_parser(prj):
	parser = argparse.ArgumentParser("build_hw", description="""
		Builds the hardware project and generates a bitstream to
		be downloaded to the FPGA.
		""")
	parser.add_argument("hwdir", help="alternative export directory", nargs="?")
	return parser

def build_cmd(args):
	build(args, args.hwdir)

def build(args, hwdir):
	if args.prj.impinfo.xil[0] == "ise":
		build_ise(args, hwdir)
	else:
		log.error("Xilinx tool not supported")

def build_ise(args, hwdir):
	prj = args.prj
	hwdir = hwdir if hwdir is not None else prj.basedir + ".hw"

	try:
		shutil2.chdir(hwdir)
	except:
		log.error("hardware directory '" + hwdir + "' not found")
		return
	
	subprocess.call("""
	  source /opt/Xilinx/""" + prj.impinfo.xil[1] + """/ISE_DS/settings64.sh;
	  echo -e "run hwclean\nrun netlist\nexit\n" | xps -nw system""",
	  shell=True)

	print()
	shutil2.chdir(prj.dir)