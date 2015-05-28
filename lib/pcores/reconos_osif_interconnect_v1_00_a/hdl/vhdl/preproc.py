import sys
import reconos.utils.shutil2 as shutil2
import reconos.utils.template as template

slots = []
res = []

slots.append({"Id": 0, "SlotId": 0, "IcRes": res, "NumIcRes": 2})
slots.append({"Id": 1, "SlotId": 1, "IcRes": res, "NumIcRes": 2})
slots.append({"Id": 2, "SlotId": 2, "IcRes": res, "NumIcRes": 2})

res.append({"Id": 1, "ResId": 1, "Mode": "hw", "IcSlots": slots, "NumIcSlots": 3})
res.append({"Id": 2, "ResId": 2, "Mode": "hw", "IcSlots": slots, "NumIcSlots": 3})

d = {}
d["SLOTS"] = slots
d["RESOURCES"] = res

if len(sys.argv) == 3:
	shutil2.copytree(sys.argv[1], sys.argv[2])
	template.preproc(sys.argv[2], d, "overwrite")
elif len(sys.argv) == 2:
	template.preproc(sys.argv[1], d, "print")
else:
	print("Wrong number of arguments")