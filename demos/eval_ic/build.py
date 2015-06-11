import sys
import reconos.utils.shutil2 as shutil2
import reconos.utils.template as template
import reconos.runtime.project
import reconos.scripts.hw.export
import reconos.scripts.hw.build

MAX = 32

for i in range(1, MAX + 1):
	wd = "eval" + str(i)

	shutil2.mkdir(wd)
	shutil2.copytree("build.cfg", wd)
	shutil2.copytree("testbench.vhd", wd)
	shutil2.copytree("testbench.xise", wd)
	shutil2.symlink("src", shutil2.join(wd, "src"))

	d = {"RESOURCES": [{} for _ in range(0, i)]}
	template.preproc(shutil2.join(wd, "build.cfg"), d, "overwrite")

	prj = reconos.runtime.project.Project()
	prj.open(shutil2.join(wd, "build.cfg"))
	reconos.scripts.hw.export._export_hw_ise(prj, None, False)

	d = {}
	d["THREAD"] = "dummy"
	d["RESOURCES"] = []
	for j in range(0, i):
		d["RESOURCES"].append({"Name": "dummy" + str(j), "Id": 128 + j})
	template.preproc(shutil2.join(wd, "testbench.vhd"), d, "overwrite")
	template.preproc(shutil2.join(wd, "testbench.xise"), d, "overwrite")

	reconos.scripts.hw.build._build_ise(prj, None)
	shutil2.chdir("..")
