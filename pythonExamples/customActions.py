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


"""
An example program demonstrating how to make and use new actions.

This example program creates two new actions, Go and Turn. Go will drive the robot forward safely,
while Turn will aobstacles detected by the sonar by turning. 
This program also adds a predefined
action from which = Aria() tries to recover from stalls (hit something and 
can't move forward) by backing and turning.

Each of these actions have the normal constructor and destructor, note that 
the constructors also call ArAction.__init__ to call the parent ArAction
constructor.
Each action then also implements the essential method fire(). This fire 
function is called by the action resolver, and returns values that, in 
combination with other actions' desired behavior, determine the driving 
commands sent to the robot.

Also note that each of these actions override the ArAction::setRobot function; these 
implementations obtain the sonar device from the robot in addition to doing the
needed caching of the robot pointer.  This is what you should do if you
care about the presence or absence of a particular sensor.  If you don't
care about any particular sensor you could just use the checkRangeDevice
methods from ArRobot (there are four of them) in your fire() method.
Also note that these are very naive actions, they are simply an example
of how to use actions.

See the Actions section of the Aria reference manual for more details about
actions, and see the Python README.txt for notes on the tricky aspects
of implementing an ArAction or other subclass.

Also, as a general note, remember actions must take a small amount of time to execute, to avoid
delaying the robot synchronization cycle.

"""



# Action that drives the robot forward, but stops if obstacles are
# detected by sonar. 
class ActionGo(ArAction):

  # constructor, sets myMaxSpeed and myStopDistance
  def __init__(self, maxSpeed, stopDistance):
    ArAction.__init__(self, "Go")
    self.myMaxSpeed = maxSpeed
    self.myStopDistance = stopDistance
    self.myDesired = ArActionDesired()
    self.mySonar = None
    # Swig doesn't wrap protected methods yet # self.setNextArgument(ArArg("maximum speed", self.myMaxSpeed, "Maximum speed to go."))
    # Swig doesn't wrap protected methods yet # self.setNextArgument(ArArg("stop distance", self.myStopDistance, "Distance at which to stop."))

  # This fire method is where the real work of the action happens.
  # currentDesired is the combined desired action from other actions
  # previously processed by the action resolver.  In this case, we're
  # not interested in that, we will set our desired 
  # forward velocity in the myDesired member, and return it.
  # Note that myDesired must be a class member:, since this method
  # will return a pointer to myDesired to the caller. If we had
  # declared the desired action as a local variable in this method,
  # the pointer we returned would be invalid after this method
  # returned.
  def fire(self, currentDesired):
    # reset the actionDesired (must be done), to clear
    # its previous values.
    self.myDesired.reset()

    # if the sonar is null we can't do anything, so deactivate
    if self.mySonar == None:
      deactivate()
      return None

    # get the range of the sonar
    range = self.mySonar.currentReadingPolar(-70, 70) - self.getRobot().getRobotRadius()

    # if the range is greater than the stop distance, find some speed to go
    if (range > self.myStopDistance):
      # just an arbitrary speed based on the range
      speed = range * .3
      # if that speed is greater than our max, cap it
      if (speed > self.myMaxSpeed):
        speed = self.myMaxSpeed
      # now set the velocity
      self.myDesired.setVel(speed)
    else:
      # the range was less than the stop distance, so request stop
      self.myDesired.setVel(0)

    # return a reference to our actionDesired to the resolver to make our request
    return self.myDesired


  # Override setRobot() to get a reference to the sonar device
  def setRobot(self, robot):
  
    # Set myRobot object in parent ArAction class (must be done if
    # you overload setRobot):
    #self.myRobot = robot
    print "ActionGo: setting robot on ArAction..."
    self.setActionRobot(robot)

    # Find sonar device for use in fire():
    self.mySonar = robot.findRangeDevice("sonar")
    if (self.mySonar == None):
      ArLog.log(ArLog.Terse, "actionExample: ActionGo: Warning: The robot had no sonar range device, deactivating!")
      deactivate()


# Action that turns the robot away from obstacles detected by the 
# sonar.

class ActionTurn(ArAction):
  def __init__(self, turnThreshold, turnAmount):
    ArAction.__init__(self, "Turn")
    self.myDesired = ArActionDesired()
    self.myTurnThreshold = turnThreshold
    self.myTurnAmount = turnAmount

    # Swig doesn't wrap protected methods yet # self.setNextArgument(ArArg("turn threshold (mm)", self.myTurnThreshold, "The number of mm away from obstacle to begin turnning."))
    # Swig doesn't wrap protected methods yet # self.setNextArgument(ArArg("turn amount (deg)", self.myTurnAmount, "The number of degress to turn if turning."))

    # remember which turn direction we requested, to help keep turns smooth
    self.myTurning = 0 # -1 == left, 1 == right, 0 == none


  def setRobot(self, robot):
    # Sets myRobot in the parent ArAction class (must be done):
    print "ActionTurn: calling ArAction.setActionRobot..."
    self.setActionRobot(robot)
    #self.myRobot = robot


    # Find sonar object for use in fire():
    self.mySonar = robot.findRangeDevice("sonar")
    if (self.mySonar == None):
      ArLog.log(ArLog.Terse, "actionExample: ActionTurn: Warning: I found no sonar, deactivating.")
      self.deactivate()

  def fire(self, currentDesired):

    # reset the actionDesired (must be done)
    self.myDesired.reset()

    # if the sonar is null we can't do anything, so deactivate
    if self.mySonar == None:
      self.deactivate()
      return None

    # Get the left readings and right readings off of the sonar
    leftRange = (self.mySonar.currentReadingPolar(0, 100) - 
          self.getRobot().getRobotRadius())
    rightRange = (self.mySonar.currentReadingPolar(-100, 0) - 
          self.getRobot().getRobotRadius())

    # if neither left nor right range is within the turn threshold,
    # reset the turning variable and don't turn
    if (leftRange > self.myTurnThreshold  and  rightRange > self.myTurnThreshold):
      self.myTurning = 0
      self.myDesired.setDeltaHeading(0)

    # if we're already turning some direction, keep turning that direction
    elif (self.myTurning != 0):
      self.myDesired.setDeltaHeading(self.myTurnAmount * self.myTurning)

    # if we're not turning already, but need to, and left is closer, turn right
    # and set the turning variable so we turn the same direction for as long as
    # we need to
    elif (leftRange < rightRange):
      self.myTurning = -1
      self.myDesired.setDeltaHeading(self.myTurnAmount * self.myTurning)

    # if we're not turning already, but need to, and right is closer, turn left
    # and set the turning variable so we turn the same direction for as long as
    # we need to
    else :
      self.myTurning = 1
      self.myDesired.setDeltaHeading(self.myTurnAmount * self.myTurning)

    # return the actionDesired, so resolver knows what to do
    return self.myDesired



Aria_init()
parser = ArArgumentParser(sys.argv)
parser.loadDefaultArguments()
robot = ArRobot()
conn = ArRobotConnector(parser, robot)
sonar = ArSonarDevice()

# Create instances of the actions defined above, plus ArActionStallRecover, 
# a predefined action from Aria_
go = ActionGo(500, 350)
turn = ActionTurn(400, 10)
recover = ArActionStallRecover()

  
  
# Connect to the robot
if not conn.connectRobot():
  ArLog.log(ArLog.Terse, "actionExample: Could not connect to robotnot  Exiting.")
  Aria_exit(1)
  
# Parse all command-line arguments
if not Aria_parseArgs():
  Aria_logOptions()
  Aria_exit(1)


# Add the range device to the robot. You should add all the range 
# devices and such before you add actions
robot.addRangeDevice(sonar)


# Add our actions in order. The second argument is the priority, 
# with higher priority actions going first, and possibly pre-empting lower
# priority actions.
robot.addAction(recover, 100)
robot.addAction(go, 50)
robot.addAction(turn, 49)

# Enable the motors
robot.enableMotors()

# Run the robot processing cycle.
# 'true' means to return if it loses connection,
# after which we exit the program.
robot.run(True)

Aria_exit(0)
