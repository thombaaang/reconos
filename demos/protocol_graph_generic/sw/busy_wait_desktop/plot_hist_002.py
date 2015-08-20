#!/usr/bin/env python
import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt 
import sys

x = []

path = ""

if sys.argc >= 2:
  path = sys.argv[1]
  
print sys.argv

with open(path, 'r') as f:
  x = np.fromstring(f.read(), dtype=float, sep=';')

# the histogram of the data
n, bins, patches = plt.hist(x, bins=400, facecolor='green', alpha=0.75)

# add a 'best fit' line
#y = mlab.normpdf( bins, mu, sigma)
#l = plt.plot(bins, y, 'r--', linewidth=1)

plt.xlabel('Execution time per dummy operation [usec]')
plt.ylabel('Count')
plt.title(r'Distribution of execution time per dummy operation')
#plt.axis([40, 160, 0, 0.03])
plt.grid(True)

plt.show()
