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

from AriaPy import *
import sys


robot = Aria.connectToRobot()

print('Running...')
robot.enableMotors()
robot.runAsync()

def printRobotPos():
  print(robot.getPose())

robot.addSensorInterpTask(printRobotPos)

# Drive the robot in a circle by requesting
# Constant rotation and forward velocity 
# components.
def driveRobot():
  print('driving forward 200mm/s, turn 15 deg/s')
  robot.setVel(200)   # forward mm/s
  robot.setRotVel(15) # turn deg/s


robot.addUserTask(driveRobot, 'drive')


# After 5 seconds, replace the user task with a new one.  In Python, you can
# define a new function with the same name. The old function still exists, and
# ArRobot will continue to call it.  So use replaceUserTask() to remove the old
# task named 'drive' and then add the new task named 'drive' which will call the
# new function.  (You can use this in an interactive Python environment such as
# Jupyter, the Python interpreter, or an IDE such as Idle.)
ArUtil.sleep(6000);

def driveRobot():
  print 'drive redefined. stopping robot.'
  robot.setVel(0)
  robot.setRotVel(0)

robot.replaceUserTask(driveRobot, 'drive')

print 'Press CTRL-C to exit'
robot.waitForRunExit()

print('Exiting.')
Aria.exit(0)
