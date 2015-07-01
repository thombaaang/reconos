import reconos.utils.shutil2 as shutil2
import reconos.utils.template as template

import logging
import argparse

import tempfile
import subprocess

log = logging.getLogger(__name__)

def get_cmd(prj):
	return "export_hw"

def get_call(prj):
	return export_hw_cmd

def get_parser(prj):
	parser = argparse.ArgumentParser("export_hw", description="""
		Exports the hardware project and generates all necessary files.
		""")
	parser.add_argument("-l", "--link", help="link sources instead of copying", action="store_true")
	parser.add_argument("-t", "--thread", help="export only single thread")
	parser.add_argument("-r", "--resource", help="export only single resource")
	parser.add_argument("hwdir", help="alternative export directory", nargs="?")
	return parser

def export_hw_cmd(args):
	if args.thread is not None and args.resource is not None:
		log.error("-t and -r are exclusive")
		return

	if args.thread is not None:
		export_hw_thread(args, args.hwdir, args.link, args.thread)
	elif args.resource is not None:
		resource = args.resource.split("_")
		export_hw_res(args, args.hwdir, resource[0], resource[1])
	else:
		export_hw(args, args.hwdir, args.link)

def export_hw(args, hwdir, link):
	if args.prj.impinfo.xil[0] == "ise":
		_export_hw_ise(args.prj, hwdir, link)
	else:
		log.error("Xilinx tool not supported")

def export_hw_thread(args, hwdir, link, thread):
	if args.prj.impinfo.xil[0] == "ise":
		_export_hw_thread_ise(args.prj, hwdir, link, thread)
	else:
		log.error("Xilinx tool not supported")

def export_hw_res(args, hwdir, resource, group):
	if args.prj.impinfo.xil[0] == "ise":
		_export_hw_res_ise(args.prj, hwdir, resource, group)
	else:
		log.error("Xilinx tool not supported")

def _export_hw_ise(prj, hwdir, link):
	hwdir = hwdir if hwdir is not None else prj.basedir + ".hw"

	log.info("Export hardware to directory '" + hwdir + "'")

	dictionary = {}
	dictionary["NUM_SLOTS"] = len(prj.slots)
	dictionary["NUM_CLOCKS"] = len(prj.clocks)
	dictionary["NUM_RESOURCES"] = len(prj.resources)
	dictionary["NUM_HWRESOURCES"] = len([_ for _ in prj.resources if _.mode == "hw"])
	dictionary["SYSCLK"] = prj.clock.id
	dictionary["SYSRST"] = "SYSRESET"
	dictionary["SLOTS"] = []
	for s in prj.slots:
		if s.threads:
			d = {}
			d["_e"] = s
			d["HwtCoreName"] = s.threads[0].get_corename()
			d["HwtCoreVersion"] = s.threads[0].get_coreversion()
			d["Id"] = s.id
			d["SlotId"] = s.id
			d["Name"] = s.name.lower()
			d["Clk"] = s.clock.id
			d["Async"] = "sync" if s.clock == prj.clock else "async"
			d["Ics"] = []
			d["NumIcs"] = 0
			d["HasIcs"] = False
			d["Ports"] = s.ports
			dictionary["SLOTS"].append(d)
	dictionary["CLOCKS"] = []
	for c in prj.clocks:
		d = {}
		d["Id"] = c.id
		d["ReqFreqKHz"] = c.freq / 1000
		param = c.get_pllparam(800000000, 1600000000, 100000000)
		d["ActFreqKHz"] = param[2] / 1000
		d["M"] = param[0]
		d["O"] = param[1]
		dictionary["CLOCKS"].append(d)
	dictionary["RESOURCES"] = []
	for r in prj.resources:
		d = {}
		d["_e"] = r
		d["Id"] = r.id
		d["ResId"] = r.id
		d["Name"] = r.name.lower()
		d["Mode"] = r.mode
		d["Ics"] = []
		d["NumIcs"] = 0
		d["HasIcs"] = False
		dictionary["RESOURCES"].append(d)

	num_ics = 0;

	# interconnect between resources and threads
	ics = []
	for s in prj.slots:
		sd = [_ for _ in dictionary["SLOTS"] if _["_e"] == s]
		if not sd:
			continue
		else:
			sd = sd[0]

		for r in [_ for _ in s.acc_resources() if _.mode == "hw"]:
			rd = [_ for _ in dictionary["RESOURCES"] if _["_e"] == r][0]
			sd["Ics"].append({"Id": num_ics, "Type": "arbiter"})
			sd["Ics"].append({"Id": num_ics + 1, "DstId": r.id, "Type": "router"})
			sd["HasIcs"] = True
			sd["NumIcs"] = len(sd["Ics"]) // 2
			rd["Ics"].append({"Id": num_ics + 1, "Type": "arbiter"})
			rd["Ics"].append({"Id": num_ics, "DstId": s.id, "Type": "router"})
			rd["HasIcs"] = True
			rd["NumIcs"] = len(rd["Ics"]) // 2
			num_ics += 2

	dictionary["NUM_ICS"] = num_ics;

	log.info("Generating export files ...")
	tmpl = "ref_" + prj.impinfo.os + "_" + "_".join(prj.impinfo.board) + "_" + prj.impinfo.design + "_" + prj.impinfo.xil[1]
	prj.apply_template(tmpl, dictionary, hwdir, link)

	log.info("Generating threads ...")
	for t in prj.threads:
		_export_hw_thread_ise(prj, shutil2.join(hwdir, "pcores"), link, t.name)

	log.info("Generating resources ...")
	for r in [_ for _ in prj.resources if _.mode == "hw"]:
		_export_hw_res_ise(prj, shutil2.join(hwdir, "pcores"), r.name, r.group)

def _export_hw_res_ise(prj, hwdir, resource, group):
	hwdir = hwdir if hwdir is not None else prj.basedir + ".hw." + resource.lower()

	log.info("Exporting resource " + resource + " to directory '" + hwdir + "'")

	resources = [_ for _ in prj.resources if _.name == resource and _.group == group]
	if (len(resources) == 1):
		resource = resources[0]

		if resource.mode != "hw":
			log.info("Resource implemented in software")
			return
	else:
		log.error("Resource " + resource + " not found")
		return

	tmp = tempfile.TemporaryDirectory()

	dictionary = {}
	dictionary["PART"] = prj.impinfo.part
	dictionary["CLKPRD"] = prj.clock.get_periodns()
	dictionary["ID"] = resource.id
	dictionary["NAME"] = resource.name.lower()
	dictionary["TYPE"] = resource.type
	dictionary["ARGS"] = resource.args
	srcs = shutil2.join(prj.impinfo.repo, "lib", "hwres", resource.type)
	dictionary["SOURCES"] = [srcs]

	log.info("Generating temporary HLS project in " + tmp.name + " ...")
	prj.apply_template("hwres_hls_build", dictionary, tmp.name)

	subprocess.call("""
	  source /opt/Xilinx/Vivado/2014.4/settings64.sh;
	  cd {0};
	  vivado_hls -f script_csynth.tcl;""".format(tmp.name),
	  shell=True)

	dictionary = {}
	dictionary["ID"] = resource.id
	dictionary["NAME"] = resource.name.lower()
	dictionary["TYPE"] = resource.type
	srcs = shutil2.join(tmp.name, "hls", "sol", "syn", "vhdl")
	dictionary["SOURCES"] = [srcs]
	incls = shutil2.listfiles(srcs, True)
	dictionary["INCLUDES"] = [{"File": shutil2.trimext(_)} for _ in incls]

	log.info("Generating export files ...")
	prj.apply_template("hwres_hls_pcore_vhdl", dictionary, hwdir)

	shutil2.rmtree("/tmp/test2")
	shutil2.mkdir("/tmp/test2")
	shutil2.copytree(tmp.name, "/tmp/test2")

def _export_hw_thread_ise(prj, hwdir, link, thread):
	hwdir = hwdir if hwdir is not None else prj.basedir + ".hw." + thread.lower()

	log.info("Exporting thread " + thread + " to directory '" + hwdir + "'")

	threads = [_ for _ in prj.threads if _.name == thread]
	if (len(threads) == 1):
		thread = threads[0]

		if thread.hwsource is None:
			log.info("No hardware source specified")
			return
	else:
		log.error("Thread " + thread  + " not found")
		return

	if thread.hwsource == "vhdl":
		dictionary = {}
		dictionary["ID"] = thread.id;
		dictionary["NAME"] = thread.name.lower();
		srcs = shutil2.join(prj.dir, "src", "rt_" + thread.name.lower(), thread.hwsource)
		dictionary["SOURCES"] = [srcs]
		incls = shutil2.listfiles(srcs, True)
		dictionary["INCLUDES"] = [{"File": shutil2.trimext(_)} for _ in incls]
		dictionary["RESOURCES"] = []
		dictionary["PORTS"] = thread.ports
		for i, r in enumerate(thread.resources):
			d = {}
			d["NameUpper"] = (r.group + "_" + r.name).upper()
			d["NameLower"] = (r.group + "_" + r.name).lower()
			d["Id"] = r.id
			d["HexId"] = "%08x" % r.id
			d["LocalId"] = i
			d["HexLocalId"] =  "%08x" % i
			d["Mode"] = r.mode
			dictionary["RESOURCES"].append(d)

		log.info("Generating export files ...")
		prj.apply_template("thread_vhdl_pcore", dictionary, hwdir, link)

	elif thread.hwsource == "hls":
		tmp = tempfile.TemporaryDirectory()

		dictionary = {}
		dictionary["PART"] = prj.impinfo.part
		dictionary["NAME"] = thread.name.lower()
		dictionary["MEM"] = thread.mem
		dictionary["MEM_N"] = not thread.mem
		dictionary["CLKPRD"] = min([_.clock.get_periodns() for _ in thread.slots])
		srcs = shutil2.join(prj.dir, "src", "rt_" + thread.name.lower(), thread.hwsource)
		dictionary["SOURCES"] = [srcs]
		files = shutil2.listfiles(srcs, True)
		dictionary["FILES"] = [{"File": _} for _ in files]
		dictionary["RESOURCES"] = []
		for i, r in enumerate(thread.resources):
			d = {}
			d["NameUpper"] = (r.group + "_" + r.name).upper()
			d["NameLower"] = (r.group + "_" + r.name).lower()
			d["Id"] = r.id
			d["HexId"] = "%08x" % r.id
			d["LocalId"] = i
			d["HexLocalId"] =  "%08x" % i
			d["Type"] = r.type
			d["TypeUpper"] = r.type.upper()
			dictionary["RESOURCES"].append(d)

		log.info("Generating temporary HLS project in " + tmp.name + " ...")
		prj.apply_template("thread_hls_build", dictionary, tmp.name)

		log.info("Starting Vivado HLS ...")
		if "bbd" in thread.hwoptions:
			subprocess.call("""
			  source /opt/Xilinx/14.7/ISE_DS/settings64.sh;
			  source /opt/Xilinx/Vivado/2014.4/settings64.sh;
			  cd {0};
			  vivado_hls -f script_export.tcl;""".format(tmp.name),
			  shell=True)

			dictionary = {}
			dictionary["NAME"] = thread.name.lower()
			dictionary["MEM"] = thread.mem
			dictionary["MEM_N"] = not thread.mem
			srcs = shutil2.join(tmp.name, "hls", "sol", "impl", "vhdl")
			dictionary["SOURCES"] = shutil2.listfiles(srcs, True, "ngc", False)
			incls = shutil2.listfiles(srcs, True, "ngc")
			dictionary["INCLUDES"] = [{"File": _} for _ in incls]

			log.info("Generating export files ...")
			prj.apply_template("thread_hls_pcore_bbd", dictionary, hwdir)

		else:
			subprocess.call("""
			  source /opt/Xilinx/Vivado/2014.4/settings64.sh;
			  cd {0};
			  vivado_hls -f script_csynth.tcl;""".format(tmp.name),
			  shell=True)

			dictionary = {}
			dictionary["NAME"] = thread.name.lower()
			dictionary["MEM"] = thread.mem
			dictionary["MEM_N"] = not thread.mem
			srcs = shutil2.join(tmp.name, "hls", "sol", "syn", "vhdl")
			dictionary["SOURCES"] = [srcs]
			incls = shutil2.listfiles(srcs, True, "vhd")
			dictionary["INCLUDES"] = [{"File": shutil2.trimext(_)} for _ in incls]

			log.info("Generating export files ...")
			prj.apply_template("thread_hls_pcore_vhdl", dictionary, hwdir)

		shutil2.rmtree("/tmp/test")
		shutil2.mkdir("/tmp/test")
		shutil2.copytree(tmp.name, "/tmp/test")