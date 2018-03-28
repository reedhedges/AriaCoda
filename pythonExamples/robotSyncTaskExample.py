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

#  Shows how to add a task callback to ArRobot's synchronization/processing cycle
#
#  This program will just have the robot wander around, it uses some aance 
#  routines, then just has a constant velocity.  A sensor interpretation task callback is invoked
#  by the ArRobot object every cycle as it runs, which records the robot's current 
#  pose and velocity.
#
#  Note that tasks must take a small amount of time to execute, to adelaying the
#  robot cycle.
#
#  Callbacks are done differently in Python than in C++.  In the Aria C++
#  library, a class called ArFunctor is used to store 
#  a function pointer, and an instance of an object.  Different ArFunctor
#  subclasses are used for functions with different arguments, and for global
#  functions which are not object methods.  Instead, in the Python wrapper, you
#  can simply pass a function (whether defined globally, or bound to an object
#  if defined as a method, or created on the fly as a 'lambda' expression)
#  whenever the Aria API calls for an ArFunctor*.  The most common use is shown
#  below:  a method bound to an object is passed directly as a sensor interpretation
#  task.


class PrintingTask:

  # Constructor. Adds a sensor interpretation task to the given robot object.
  def __init__(self, robot):
    self.myRobot = robot
    robot.addSensorInterpTask("PrintingTask", 50, self.doTask)
  
  # This method will be called by ArRobot as a sensor interpretation task
  def doTask(self):
    print "x %6.1f  y %6.1f  th  %6.1f vel %7.1f mpacs %3d"  % (self.myRobot.getX(), self.myRobot.getY(), self.myRobot.getTh(), self.myRobot.getVel(), self.myRobot.getMotorPacCount())



Aria_init()

parser = ArArgumentParser(sys.argv)
robot = ArRobot()
con = ArRobotConnector(parser, robot)

if not con.connectRobot():
  print "Could not connect to robot, exiting"
  Aria_logOptions()
  Aria_exit(1)


sonar = ArSonarDevice()

# This object encapsulates the task we want to do every cycle. 
# Upon creation, it puts a callback functor in the ArRobot object
# as a 'user task'.
pt = PrintingTask(robot)

# actions used to wander
recover = ArActionStallRecover()
aFront = ArActionAvoidFront()
constantVelocity = ArActionConstantVelocity("Constant Velocity", 400)


robot.addRangeDevice(sonar)


print "Connected to the robot. (Press Ctrl-C to exit)"


# add the wander actions
robot.addAction(recover, 100)
robot.addAction(aFront, 50)
robot.addAction(constantVelocity, 25)

robot.enableMotors()

# Start the robot process cycle running. Each cycle, it calls the robot's
# tasks. When the PrintingTask was created above, it added a new
# task to the robot. '1' means that if the robot connection
# is lost, then ArRobot's processing cycle ends and self call returns.
robot.run(True)

print "Disconnected. Goodbye."

Aria.exit(0)
