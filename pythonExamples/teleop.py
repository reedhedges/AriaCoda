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

# This is an example of how to use the limiting behaviors.
#  
#  The way it works is that it has a limiting behavior higher priority
#  than the joydrive action behavior.  So the joydrive action can try
#  to do whatever it wants, but it won't work.

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
  Aria.exit(2)
  

print 'Connecting to laser and waiting 1 sec...'
laser = None
if(laserCon.connectLasers()):
  print 'Connected to lasers as configured in parameters'
  laser = robot.findLaser(1)
else:
  print 'Warning: unable to connect to lasers. Continuing anyway!'


# the joydrive action
jdAct = ArActionJoydrive()

# a keyboard drive action
kdAct = ArActionKeydrive()

# limiter for close obstacles
limiter = ArActionLimiterForwards("speed limiter near", 300, 600, 250)

# limiter for far away obstacles
limiterFar = ArActionLimiterForwards("speed limiter far", 300, 1100, 400)

# if the robot has upward facing IR sensors ("under the table" sensors), this 
# stops us too
tableLimiter = ArActionLimiterTableSensor()

# limiter so we don't bump things backwards
backwardsLimiter = ArActionLimiterBackwards()

# add the actions, put the limiters on top, then have the joydrive action,
# this will keep the action from being able to drive too fast and hit
# something
robot.lock()
robot.addAction(tableLimiter, 100)
robot.addAction(limiter, 95)
robot.addAction(limiterFar, 90)
robot.addAction(backwardsLimiter, 85)
robot.addAction(kdAct, 51)
robot.addAction(jdAct, 50)
robot.unlock()


print "Connected to the robot.  Use arrow keys or joystick to move. Spacebar to stop."


# enable motors and run the robot until connection is lost
robot.enableMotors()

while 1:
  ArUtil.sleep(500)

Aria.exit(0)
