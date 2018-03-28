"""
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016-2018 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
"""

import numpy
import pylab

# To install numpy and matplotlib on Ubuntu linux:
#   sudo apt-get install python-numpy python-matplotlib
#
# To install on Windows, open a Windows command prompt window and use pip:
#   python -m pip install numpy
#   python -m pip install matplotlib
# (Python must be in your path)


try:
  data = numpy.loadtxt('dataLog.txt', comments=';')
  #data = numpy.genfromtxt(fname='dataLog.txt', comments=';', names=True, invalid_raise=True)
  print data
  #print data.dtype
except Exception as e:
  print e
  raise e

# dataLog.txt must contain these columns:
# column# 0     1  2   3   4   ...     11    12
# ;    Time  Volt  X   Y  Th   ... TransV  RotV
# TODO find values by looking at data.dtype if using numpy.getfromtxt
# Extract arrays of values using Python slice array operations. 
# Change the column index in the second part of the array slice subscript
# if your data log has these values in different columns than
# the example above.
times = data[:,0]
volts = data[:,1]
xs = data[:,2]
ys = data[:,3]
ths = data[:,4]
vels = data[:,11]
rotvels = data[:,12]

fig = pylab.figure(1)

pylab.subplot(121)
pylab.plot(xs, ys, 'ro')
pylab.xlabel('X')
pylab.ylabel('Y')
pylab.title('Robot position')

pylab.subplot(122)
vp = pylab.plot(times, vels, 'r', label='TransVel')
rp = pylab.plot(times, rotvels, 'b', label='RotVel')
pylab.xlabel('Time')
pylab.ylabel('Velocity (Trans, Rot)')
pylab.legend([vp, rp], ['TransVel', 'RotVel'])
pylab.title('Robot velocity')

pylab.show()

