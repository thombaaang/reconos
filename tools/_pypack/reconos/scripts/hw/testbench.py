import reconos.utils.shutil2 as shutil2
import reconos.utils.template as template
import reconos.scripts.hw.export

import logging
import argparse

import tempfile

log = logging.getLogger(__name__)

def get_cmd(prj):
	return "export_sim"

def get_call(prj):
	return export_sim_cmd

def get_parser(prj):
	parser = argparse.ArgumentParser("export_sim", description="""
		Creates a testbench to simulate a single hardware thread.
		""")
	parser.add_argument("-t", "--thread", help="ReconOS thread to create testbench for")
	parser.add_argument("-l", "--link", help="link sources instead of copying", action="store_true")
	parser.add_argument("simdir", help="alternative export directory", nargs="?")
	return parser

def export_sim_cmd(args):
	if args.thread is None:
		export_sim(args, args.simdir, args.link)
	else:
		export_sim_thread(args, args.simdir, args.link, args.thread)

def export_sim(args, simdir, link):
	if args.prj.impinfo.xil[0] == "ise":
		_export_sim_ise(args.prj, simdir, link)
	else:
		log.error("Xilinx tool not supported")

def _export_sim_ise(prj, simdir, link):
	simdir = simdir if simdir is not None else prj.basedir + ".sim"

	log.info("Generating testbench")

	tmp = tempfile.TemporaryDirectory()
	reconos.scripts.hw.export._export_hw_ise(prj, tmp.name, False)

	dictionary = {}
	dictionary["NUM_SLOTS"] = len(prj.slots)
	dictionary["SLOTS"] = []
	for s in prj.slots:
		d = {}
		d["_e"] = s
		d["Id"] = s.id
		d["Name"] = s.name.lower()
		d["Thread"] = s.threads[0].name.lower()
		dictionary["SLOTS"].append(d)
	dictionary["THREADS"] = []
	for t in prj.threads:
		d = {}
		d["_e"] = t
		d["Name"] = t.name.lower()
		dictionary["THREADS"].append(d)
	dictionary["RESOURCES"] = []
	for r in prj.resources:
		d = {}
		d["_e"] = r
		d["Id"] = r.id
		d["Name"] = r.name.lower()
		d["Mode"] = r.mode
		dictionary["RESOURCES"].append(d)
	srcs = shutil2.join(tmp.name, "pcores")
	print("SRCS = " + str(srcs))
	dictionary["SOURCES"] = [srcs]
	incls = shutil2.listfiles(srcs, True, "vhd")
	print("INCLS = " + str(incls))
	dictionary["INCLUDES"] = [{"File": _, "Library": _.split("/")[0]} for _ in incls]

	tmpl = "sim_system_" + prj.impinfo.xil[1]
	prj.apply_template(tmpl, dictionary, simdir, link)

def export_sim_thread(args, simdir, link, thread):
	if args.prj.impinfo.xil[0] == "ise":
		_export_sim_thread_ise(args.prj, simdir, link, thread)
	else:
		log.error("Xilinx tool not supported")

def _export_sim_thread_ise(prj, simdir, link, thread):
	simdir = simdir if simdir is not None else prj.basedir + ".sim" + "." + thread.lower()

	log.info("Generating testbench for thread " + thread + " to '" + simdir + "'")

	threads = [_ for _ in prj.threads if _.name == thread]
	if (len(threads) == 1):
		thread = threads[0]

		if thread.hwsource is None:
			log.error("No hardware source specified")
	else:
		log.error("Thread '" + thread  + "' not found")
		return

	tmp = tempfile.TemporaryDirectory()
	reconos.scripts.hw.export._export_hw_thread_ise(prj, tmp.name, link, thread.name)

	dictionary = {}
	dictionary["THREAD"] = thread.name.lower()
	srcs = shutil2.join(tmp.name, "rt_" + thread.name.lower() + "_v1_00_a", "hdl", "vhdl")
	dictionary["SOURCES"] = [srcs]
	files = shutil2.listfiles(srcs, True)
	dictionary["FILES"] = [{"File": _} for _ in files]

	log.info("Generating export files ...")
	tmpl = "sim_thread_" + prj.impinfo.xil[1]
	prj.apply_template(tmpl, dictionary, simdir, link)