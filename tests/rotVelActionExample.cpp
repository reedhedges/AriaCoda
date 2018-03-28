/*
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
*/
#include "Aria.h"

/*
This demonstrates how to make actions using rot vel and how to use them.
It creates two actions, Go and Turn... Go will drive the robot forward safely,
while Turn will avoid obstacles by turning.

Each of these actions have the normal constructor and destructor, note that 
the constructors use constructor chaining to create their ArAction part
correctly.  Each action then also implements the needed fire function, this
fire function is where the robot is driven from.  

Also note that each of these actions override the setRobot function, their
implementation grabs the sonar device from the robot in addition to doing the
needed caching of the robot pointer.  This is what you should do if you
care about the presence or absence of a particular sensor.  If you don't
care about any particular sensor you could just use one of the 
ArRobot::checkRangeDevice functions (there are four of them).

Also note that these are very naive actions, they are simply an example
of how to use actions.
*/

class ActionGo : public ArAction
{
public:
  // constructor, sets myMaxSpeed and myStopDistance
  ActionGo(double maxSpeed, double stopDistance);
  // destructor, its just empty, we don't need to do anything
  virtual ~ActionGo(void) {};
  // fire, this is what the resolver calls to figure out what this action wants
  virtual ArActionDesired *fire(ArActionDesired currentDesired);
  // sets the robot pointer, also gets the sonar device
  virtual void setRobot(ArRobot *robot);
protected:
  // this is to hold the sonar device form the robot
  ArRangeDevice *mySonar;
  // what the action wants to do
  ArActionDesired myDesired;
  // maximum speed
  double myMaxSpeed;
  // distance to stop at
  double myStopDistance;
};


class ActionTurn : public ArAction
{
public:
  // constructor, sets the turnThreshold, and rotVel
  ActionTurn(double turnThreshold, double rotVel);
  // destructor, its just empty, we don't need to do anything
  virtual ~ActionTurn(void) {};
  // fire, this is what the resolver calls to figure out what this action wants
  virtual ArActionDesired *fire(ArActionDesired currentDesired);
  // sets the robot pointer, also gets the sonar device
  virtual void setRobot(ArRobot *robot);
protected:
  // this is to hold the sonar device form the robot
  ArRangeDevice *mySonar;
  // what the action wants to do
  ArActionDesired myDesired;
  // distance at which to start turning
  double myTurnThreshold;
  // amount to turn when turning is needed
  double myTurnAmount;
  // value ot hold onto so turns are smooth, which direction its turning
  int myTurning; // -1 == left, 1 == right, 0 == none
};


/*
  This is the constructor, note the use of constructor chaining with the
  ArAction... also note how it uses setNextArgument, which makes it so that 
  other things can see what parameters this action has, and set them.
  It also initializes the classes variables.
*/
ActionGo::ActionGo(double maxSpeed, double stopDistance) :
  ArAction("Go")
{
  mySonar = NULL;
  myMaxSpeed = maxSpeed;
  myStopDistance = stopDistance;
  setNextArgument(ArArg("maximum speed", &myMaxSpeed, "Maximum speed to go."));
  setNextArgument(ArArg("stop distance", &myStopDistance, "Distance at which to stop."));
}

/*
  Sets the myRobot pointer (all setRobot overloaded functions must do this),
  finds the sonar device from the robot, and if the sonar isn't there, 
  then it deactivates itself.
*/
void ActionGo::setRobot(ArRobot *robot)
{
  myRobot = robot;
  mySonar = myRobot->findRangeDevice("sonar");
  if (mySonar == NULL)
    deactivate();
}

/*
  This fire is the whole point of the action.
*/
ArActionDesired *ActionGo::fire(ArActionDesired currentDesired)
{
  double range;
  double speed;

  // reset the actionDesired (must be done)
  myDesired.reset();

  // if the sonar is null we can't do anything, so deactivate
  if (mySonar == NULL)
  {
    deactivate();
    return NULL;
  }

  // get the range off the sonar
  range = mySonar->currentReadingPolar(-70, 70) - myRobot->getRobotRadius();
  // if the range is greater than the stop distance, find some speed to go
  if (range > myStopDistance)
  {
    // just an arbitrary speed based on the range
    speed = range * .3;
    // if that speed is greater than our max, cap it
    if (speed > myMaxSpeed)
      speed = myMaxSpeed;
    // now set the velocity
    myDesired.setVel(speed);
  }
  // the range was less than the sop distance, so just stop
  else
  {
    myDesired.setVel(0);
  }
  // return a pointer to the actionDesired, so resolver knows what to do
  return &myDesired;
}

/*
  This is the constructor, note the use of constructor chaining with the
  ArAction... also note how it uses setNextArgument, which makes it so that 
  other things can see what parameters this action has, and set them.
  It also initializes the classes variables.
*/
ActionTurn::ActionTurn(double turnThreshold, double rotVel) :
  ArAction("Turn")
{
  myTurnThreshold = turnThreshold;
  myTurnAmount = rotVel;
  setNextArgument(ArArg("turn threshold (mm)", &myTurnThreshold, "The number of mm away from obstacle to begin turnning."));
  setNextArgument(ArArg("turn amount (deg)", &myTurnAmount, "The number of degress to turn if turning."));
  myTurning = 0;
}

/*
  Sets the myRobot pointer (all setRobot overloaded functions must do this),
  finds the sonar device from the robot, and if the sonar isn't there, 
  then it deactivates itself.
*/
void ActionTurn::setRobot(ArRobot *robot)
{
  myRobot = robot;
  mySonar = myRobot->findRangeDevice("sonar");
  if (mySonar == NULL)
    deactivate();
}

/*
  This is the guts of the action.
*/
ArActionDesired *ActionTurn::fire(ArActionDesired currentDesired)
{
  double leftRange, rightRange;

  // reset the actionDesired (must be done)
  myDesired.reset();

  // if the sonar is null we can't do anything, so deactivate
  if (mySonar == NULL)
  {
    deactivate();
    return NULL;
  }

  // Get the left readings and right readings off of the sonar
  leftRange = (mySonar->currentReadingPolar(0, 100) - 
	       myRobot->getRobotRadius());
  rightRange = (mySonar->currentReadingPolar(-100, 0) - 
		myRobot->getRobotRadius());

  // if neither left nor right range is within the turn threshold,
  // reset the turning variable and don't turn
  if (leftRange > myTurnThreshold && rightRange > myTurnThreshold)
  {
    myTurning = 0;
    myDesired.setRotVel(0);
  }
  // if we're already turning some direction, keep turning that direction
  else if (myTurning)
  {
    myDesired.setRotVel(myTurnAmount * myTurning);
  }
  // if we're not turning already, but need to, and left is closer, turn right
  // and set the turning variable so we turn the same direction for as long as
  // we need to
  else if (leftRange < rightRange)
  {
    myTurning = -1;
    myDesired.setRotVel(myTurnAmount * myTurning);
  }
  // if we're not turning already, but need to, and right is closer, turn left
  // and set the turning variable so we turn the same direction for as long as
  // we need to
  else 
  {
    myTurning = 1;
    myDesired.setRotVel(myTurnAmount * myTurning);
  }
  // return a pointer to the actionDesired, so resolver knows what to do
  return &myDesired;
}

int main(void)
{
  // The connection we'll use to talk to the robot
  ArTcpConnection con;
  // the robot
  ArRobot robot;
  // the sonar device
  ArSonarDevice sonar;

  // some stuff for return values
  int ret;
  std::string str;

  // the behaviors from above, and a stallRecover behavior that uses defaults
  ActionGo go(500, 350);
  ActionTurn turn(400, 30);
  ArActionStallRecover recover;

  // this needs to be done
  Aria::init();

  // open the connection, just using the defaults, if it fails, exit
  if ((ret = con.open()) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    Aria::shutdown();
    return 1;
  }
  
  // add the range device to the robot, you should add all the range 
  // devices and such before you add actions
  robot.addRangeDevice(&sonar);
  // set the robot to use the given connection
  robot.setDeviceConnection(&con);
  
  // do a blocking connect, if it fails exit
  if (!robot.blockingConnect())
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    return 1;
  }

  // enable the motors, disable amigobot sounds
  robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  // add our actions in a good order, the integer here is the priority, 
  // with higher priority actions going first
  robot.addAction(&recover, 100);
  robot.addAction(&go, 50);
  robot.addAction(&turn, 49);
  
  // run the robot, the true here is to exit if it loses connection
  robot.run(true);
  
  // now just shutdown and go away
  Aria::shutdown();
  return 0;
}
