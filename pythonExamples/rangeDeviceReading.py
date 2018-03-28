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

from __future__ import division # For correct float division in Python 2
from AriaPy import *
import sys

# This Python script connects to the robot and prints out the current
# Sonar and Laser range readings.

chooseWiBox()

Aria.init()
argparser = ArArgumentParser(sys.argv)
argparser.loadDefaultArguments()
robot = ArRobot()
conn = ArRobotConnector(argparser, robot)
laserCon = ArLaserConnector(argparser, robot, conn)

if (not conn.connectRobot(robot)):
  print 'Error connecting to robot'
  Aria.logOptions()
  print 'Could not connect to robot, exiting.'
  Aria.exit(1)

	
print 'Connected to robot'
sonar = ArSonarDevice()
robot.addRangeDevice(sonar)
robot.runAsync(True)

if not Aria_parseArgs():
  Aria.logOptions()
  Aria.exit(1)
  

print 'Connecting to laser and waiting 1 sec...'
laser = None
if(laserCon.connectLasers()):
  print 'Connected to lasers as configured in parameters'
  laser = robot.findLaser(1)
else:
  print 'Warning: unable to connect to lasers. Continuing anyway!'

	
ArUtil.sleep(1000)
robot.lock()
poses = sonar.getCurrentBufferAsVector()
print 'Sonar readings (%d) (Point coordinates in space):' % (len(poses))
for p in poses:
  print '    sonar sensed something at point ', p

print ''
left = sonar.currentReadingPolar(-135, -45)
front = sonar.currentReadingPolar(-45, 45)
right = sonar.currentReadingPolar(45, 135)
back = sonar.currentReadingPolar(135, -135)
print 'Closest sonar range: left sector: %d front: %d right: %d back: %d mm' % (left, front, right, back)
  
robot.unlock()

print ''
print ''

if laser:
  laser.lockDevice()
  readings = laser.getRawReadingsAsVector()
  print 'Laser readings (%d):' % (len(readings))
  for r in readings:
    print '   from laser placed at pose (X:%.1g, Y:%.1g) on robot, have a sensed point %s, distance is %d' % (r.getSensorX(), r.getSensorY(), r.getPose(), r.getRange())
  laser.unlockDevice()

print ''
left = robot.checkRangeDevicesCurrentPolar(-135, -45)
front = robot.checkRangeDevicesCurrentPolar(-45, 45)
right = robot.checkRangeDevicesCurrentPolar(45, 135)
back = robot.checkRangeDevicesCurrentPolar(135, -135)
print 'Closest sensor range: left sector: %d front: %d right: %d back: %d mm' % (left, front, right, back)
  
robot.unlock()

print 'goodbye.'
Aria.exit(0)
