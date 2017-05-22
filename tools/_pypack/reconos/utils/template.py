#                                                        ____  _____
#                            ________  _________  ____  / __ \/ ___/
#                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
#                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
#                         /_/   \___/\___/\____/_/ /_/\____//____/
# 
# ======================================================================
# 
#   project:      ReconOS - Toolchain
#   author:       Christoph Rüthing, University of Paderborn
#   description:  A preprocessor parsing source files.
# 
# ======================================================================

import re
import reconos.utils.shutil2 as shutil2

#
# Internal method used for replacing each occurence of a generate
# statement in the source file.
#
#   d - dictionary including the used keys
#
def _gen_preproc(d):
	def _gen_(m):
		if m.group("key") not in d:
			return m.string[m.start():m.end()]

		if type(d[m.group("key")]) is bool:
			return m.group("data")

		if type(d[m.group("key")]) is not list:
			return m.string[m.start():m.end()]

		data = ""
		for i, r in enumerate(d[m.group("key")]):
			l = {"_i" : i}
			l.update(r)

			if m.group("cond") is not None and not eval(m.group("cond"), l):
				continue

			ndata = m.group("data")

			reg = r"<<(?P<key>[A-Za-z0-9_]+)(?:\((?P<join>.*?)\))?>>"
			def repl(m):
				if m.group("key") in l:
					return str(l[m.group("key")]) 
				else:
					return "<<" + m.group("key") + ">>"
			ndata = re.sub(reg, repl, ndata)

			reg = r"<<c(?P<data>.)>>"
			if i < len(d[m.group("key")]) - 1:
				ndata = re.sub(reg, "\g<data>", ndata)
			else:
				ndata = re.sub(reg, "", ndata)

			if ndata.count("\n") > 1:
				ndata += "\n"

			data += ndata

		return data

	return _gen_

#
# Internal method used for replacing each occurence of an if
# statement in the source file.
#
#   d - dictionary including the used keys
#
def _if_preproc(d):
	def _gen_(m):
		eval_string="{} {} {}".format(m.group("key"), m.group("comp"), m.group("value"))
		
		if eval(eval_string, d):
			return m.group("data")
		else:
			return ""
	
	return _gen_

#
# Preprocesses the given file using a dictionary containing keys and
# a list of values.
#
#   filepath   - path to the file to preprocess
#   dictionary - dictionary containing all keys
#   mode       - print or overwrite to print to stdout or overwrite
#
def preproc(filepath, dictionary, mode, force=False):
	with open(filepath, "r") as file:
		try:
			data = file.read()
		except:
			return

	if "<<reconos_preproc>>" not in data and not force:
		return
	else:
		data = re.sub(r"<<reconos_preproc>>", "", data)

	# Generate syntax: <<generate for KEY(OPTIONAL = CONDITION)>> ... <<end generate>>
	# Used to automatically generate several lines of code.
	reg = r"<<generate for (?P<key>[A-Za-z0-9_]*?)(?:\((?P<cond>.*?)\))?>>\n?(?P<data>.*?)<<end generate>>"
	data = re.sub(reg, _gen_preproc(dictionary), data, 0, re.DOTALL)

	# If syntax: <<if KEY OPERATOR VALUE>> ... <<end if>>
	# Used to conditionally include or exclude code fragments
	reg = r"<<if (?P<key>[A-Za-z0-9_]*?)(?P<comp>[<>=!]*?)(?P<value>[A-Za-z0-9_\"]*?)>>\n?(?P<data>.*?)<<end if>>"
	data = re.sub(reg, _if_preproc(dictionary), data, 0, re.DOTALL)

	reg = r"<<(?P<key>[A-Za-z0-9_]+)>>"
	def repl(m): 
		if m.group("key") in dictionary:
			return str(dictionary[m.group("key")])
		else:
			return m.string[m.start():m.end()]
	data = re.sub(reg, repl, data)

	if mode == "print":
		print(data)
	elif mode == "overwrite":
		with open(filepath, "w") as file:
			file.write(data)

#
# Preprocesses the given file by replacing the file by its sources.
#
#   filepath   - path to the file to preprocess
#   dictionary - dictionary containing all keys
#
def precopy(filepath, dictionary, link):
	reg = r"^<<generate_for_(?P<key>[A-Za-z0-9_]+)>>"
	m = re.match(reg, shutil2.basename(filepath))

	if m is None:
		return
	else:
		shutil2.remove(filepath)

	if m.group("key") in dictionary:
		for f in dictionary[m.group("key")]:
			if link:
				shutil2.linktree(f, shutil2.dirname(filepath))
			else:
				shutil2.copytree(f, shutil2.dirname(filepath), True)

#
# Preprocesses the given file by renaming it.
#
#   filepath   - path to the file to preprocess
#   dictionary - dictionary containing all keys
#
def prefile(filepath, dictionary):
	old = shutil2.basename(filepath)

	reg = r"<<(?P<key>[A-Za-z0-9_]+)>>"
	def repl(m): 
		if m.group("key") in dictionary:
			return str(dictionary[m.group("key")])
		else:
			return m.string[m.start():m.end()]
	new = re.sub(reg, repl, old)

	if (new != old):
		old = shutil2.join(shutil2.dirname(filepath), old)
		new = shutil2.join(shutil2.dirname(filepath), new)
		if shutil2.exists(new):
			shutil2.rmtree(new)
		shutil2.rename(old, new)

#
# Preprocesses the given directory by renaming it.
#
#   dirpath    - path to the file to preprocess
#   dirs       - dirs of walk function
#   dictionary - dictionary containing all keys
#
def predirectory(dirpath, dirs, dictionary):
	old = shutil2.basename(dirpath)

	reg = r"<<(?P<key>[A-Za-z0-9_]+)>>"
	def repl(m): 
		if m.group("key") in dictionary:
			return str(dictionary[m.group("key")])
		else:
			return m.string[m.start():m.end()]
	new = re.sub(reg, repl, old)

	if (new != old):
		dirs.remove(old)
		dirs.append(new)
		old = shutil2.join(shutil2.dirname(dirpath), old)
		new = shutil2.join(shutil2.dirname(dirpath), new)
		if shutil2.exists(new):
			shutil2.rmtree(new)
		shutil2.rename(old, new)

#
# Generates the template given using a dictionary containing keys and
# a list of values.
#
#   filepath   - path to template directory
#   dictionary - dictionary containing all keys
#   mode       - printe or overwrite to print to stdout or overwrite
#
def generate(filepath, dictionary, mode, link = False):
	def pc(f): return precopy(f, dictionary, link)
	shutil2.walk(filepath, pc)

	def pf(f): return prefile(f, dictionary)
	def pd(d, dirs): return predirectory(d, dirs, dictionary)
	shutil2.walk(filepath, pf, pd)

	def pp(f): return preproc(f, dictionary, mode)
	shutil2.walk(filepath, pp)