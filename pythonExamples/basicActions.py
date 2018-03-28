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

# Wander around, avoiding obstacles, using some ArActions predefined in ARIA.

Aria_init()
robot = ArRobot()
parser = ArArgumentParser(sys.argv)
conn = ArRobotConnector(parser, robot) 
parser.loadDefaultArguments()

if not conn.connectRobot():
  print "Could not connect to robot, exiting"
  Aria_exit(1)
  
if not Aria_parseArgs():
  Aria_logOptions()
  Aria_exit(1)



# Most robots have sonar:
print "Creating sonar object..."
sonar = ArSonarDevice()
robot.addRangeDevice(sonar)

# Some robots have laser rangefinders (enabled in robot's parameter .p file or
# with -connectLaser command line argument):
laserConn = ArLaserConnector(parser, robot, conn)
if not laserConn.connectLasers():
  print "Warning: could not connect to laser(s)."

# Add actions to ArRobot.  While running the robot's action resolver will
# determine motion commands by evaluating the actions in order from lowest
# number to highest (so lower order actions' desired motion commands can
# supercede higher)

print "Adding actions..."
print "   1. StallRecover"
stallRecover = ArActionStallRecover()
robot.addAction(stallRecover, 1)
print "   2. AvoidFront"
avoid = ArActionAvoidFront()
robot.addAction(avoid, 2)
print "   3. LimitFront"
limitFront = ArActionLimiterForwards("limitFront", 300, 600, 250)
robot.addAction(limitFront, 3)
print "   4. LimitBack"
limitBack = ArActionLimiterBackwards()
robot.addAction(limitBack, 4)
print "   5. ConstantVelocity"
constVel = ArActionConstantVelocity()
robot.addAction(constVel, 5)


# Run robot thread here in the main thread. 
# 1 (=true) makes the function exit if the robot connection goes away unexpectedly.
print "Running robot..."
robot.enableMotors()
robot.run(True)

print "goodbye."
Aria_exit(0)
