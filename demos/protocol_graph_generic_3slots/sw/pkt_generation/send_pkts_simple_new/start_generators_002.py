#!/usr/bin/env python

# NOTE: execute this script with sudo!

import os
import subprocess
from time import *
import atexit
import sys

#################################

# get file descriptor of /dev/null to discard any output
DEVNULL = open(os.devnull, 'wb')
oldstdout = sys.stdout

# not necessary if the script is killed with ctrl-c
def cleanup():
  print "cleanup"
  sys.__stdout__ = oldstdout
  for p in processes:
    p.kill() 
  
#################################

offset = 10 # time between the first and second pkt generator is started
upper_limit_1 = str(400)
upper_limit_2 = str(400)
interval1 = str(20)
interval2 = str(20)

if len(sys.argv) == 2:
  offset = float(sys.argv[1])
elif len(sys.argv) == 4:
  offset = float(sys.argv[1])
  upper_limit_1 = str(sys.argv[2])
  upper_limit_2 = str(sys.argv[3])
elif len(sys.argv) == 6:
  offset = float(sys.argv[1])
  upper_limit_1 = str(sys.argv[2])
  upper_limit_2 = str(sys.argv[3])
  interval1 = str(sys.argv[4])
  interval2 = str(sys.argv[5])
  
print "offset:", offset, "secs"
print "upper limit idp 8:", upper_limit_1
print "upper limit idp 9:", upper_limit_2
print "interval idp 8:", interval1
print "interval idp 9:", interval2

print "----------------"

# command for the pkt generation
args_gen1 = ["./send_pkts_simple", "-p64", "-saaaaaaaaaaaaaaa8", "-l1", "-u"+upper_limit_1, "-i"+interval1]
args_gen2 = ["./send_pkts_simple", "-p64", "-saaaaaaaaaaaaaaa9", "-l1", "-u"+upper_limit_2, "-i"+interval2]

processes = []
atexit.register(cleanup)


processes.append(subprocess.Popen(args_gen1, stdout=DEVNULL))
print " ".join(args_gen1)

sleep(offset)

processes.append(subprocess.Popen(args_gen2, stdout=DEVNULL))
print " ".join(args_gen2)

print "all processes started"

# wait forever (in order to not kill the subprocesses)
while True:
  sleep(10)



