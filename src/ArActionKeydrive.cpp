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
#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArActionKeydrive.h"
#include "ArRobot.h"
#include "ariaInternal.h"
#include "ArKeyHandler.h"

AREXPORT ArActionKeydrive::ArActionKeydrive(const char *name,
					    double transVelMax,
					    double turnAmountMax,
					    double velIncrement,
					    double turnIncrement)
  :
  ArAction(name, "This action reads the keyboard arrow keys and sets the translational and rotational velocities based on this."),
  myUpCB(this, &ArActionKeydrive::up),
  myDownCB(this, &ArActionKeydrive::down),
  myLeftCB(this, &ArActionKeydrive::left),
  myRightCB(this, &ArActionKeydrive::right),
  mySpaceCB(this, &ArActionKeydrive::space)
{
  setNextArgument(ArArg("trans vel max", &myTransVelMax, "The maximum speed to go (mm/sec)"));
  myTransVelMax = transVelMax;

  setNextArgument(ArArg("turn amount max", &myTurnAmountMax, "The maximum amount to turn (deg/cycle)"));
  myTurnAmountMax = turnAmountMax;

  setNextArgument(ArArg("vel increment per keypress", &myVelIncrement, "The amount to increment velocity by per keypress (mm/sec)"));
  myVelIncrement = velIncrement;
  
  setNextArgument(ArArg("turn increment per keypress", &myVelIncrement, "The amount to turn by per keypress (deg)"));
  myTurnIncrement = turnIncrement;

  myDesiredSpeed = 0;
  myDeltaVel = 0;
  myTurnAmount = 0;
  mySpeedReset = true;
}

AREXPORT ArActionKeydrive::~ArActionKeydrive()
{

}

AREXPORT void ArActionKeydrive::setRobot(ArRobot *robot)
{
  ArKeyHandler *keyHandler;
  myRobot = robot;
  if (robot == NULL)
    return;
   
  // see if there is already a keyhandler, if not make one for ourselves
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    keyHandler = new ArKeyHandler;
    Aria::setKeyHandler(keyHandler);
    myRobot->attachKeyHandler(keyHandler);
  }
  takeKeys();
}

AREXPORT void ArActionKeydrive::takeKeys(void)
{
  ArKeyHandler *keyHandler;
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    ArLog::log(ArLog::Terse, 
	       "ArActionKeydrive::takeKeys: There is no key handler, keydrive will not work.");
  }
  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (!keyHandler->addKeyHandler(ArKeyHandler::UP, &myUpCB))
    ArLog::log(ArLog::Terse, "The key handler already has a key for up, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::DOWN, &myDownCB))
    ArLog::log(ArLog::Terse, "The key handler already has a key for down, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::LEFT, &myLeftCB))
    ArLog::log(ArLog::Terse,  
	       "The key handler already has a key for left, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::RIGHT, &myRightCB))
    ArLog::log(ArLog::Terse,  
	       "The key handler already has a key for right, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::SPACE, &mySpaceCB))
    ArLog::log(ArLog::Terse,  
	       "The key handler already has a key for space, keydrive will not work correctly.");
}

AREXPORT void ArActionKeydrive::giveUpKeys(void)
{
  ArKeyHandler *keyHandler;
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    ArLog::log(ArLog::Terse, 
	       "ArActionKeydrive::giveUpKeys: There is no key handler, something is probably horribly wrong .");
  }
  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (!keyHandler->remKeyHandler(&myUpCB))
    ArLog::log(ArLog::Terse, "ArActionKeydrive: The key handler already didn't have a key for up, something is wrong.");
  if (!keyHandler->remKeyHandler(&myDownCB))
    ArLog::log(ArLog::Terse, "ArActionKeydrive: The key handler already didn't have a key for down, something is wrong.");
  if (!keyHandler->remKeyHandler(&myLeftCB))
    ArLog::log(ArLog::Terse,  
	       "ArActionKeydrive: The key handler already didn't have a key for left, something is wrong.");
  if (!keyHandler->remKeyHandler(&myRightCB))
    ArLog::log(ArLog::Terse,  
	       "ArActionKeydrive: The key handler already didn't have a key for right, something is wrong.");
  if (!keyHandler->remKeyHandler(&mySpaceCB))
    ArLog::log(ArLog::Terse,  
	       "ArActionKeydrive: The key handler didn't have a key for space, something is wrong.");
}

AREXPORT void ArActionKeydrive::setSpeeds(double transVelMax, 
					  double turnAmountMax)
{
  myTransVelMax = transVelMax;
  myTurnAmountMax = turnAmountMax;
}

AREXPORT void ArActionKeydrive::setIncrements(double velIncrement, 
					      double turnIncrement)
{
  myVelIncrement = velIncrement;
  myTurnIncrement = turnIncrement;
}

AREXPORT void ArActionKeydrive::up(void)
{
  myDeltaVel += myVelIncrement;
}

AREXPORT void ArActionKeydrive::down(void)
{
  myDeltaVel -= myVelIncrement;
}

AREXPORT void ArActionKeydrive::left(void)
{
  myTurnAmount += myTurnIncrement;
  if (myTurnAmount > myTurnAmountMax)
    myTurnAmount = myTurnAmountMax;
}

AREXPORT void ArActionKeydrive::right(void)
{
  myTurnAmount -= myTurnIncrement;
  if (myTurnAmount < -myTurnAmountMax)
    myTurnAmount = -myTurnAmountMax;
}

AREXPORT void ArActionKeydrive::space(void)
{
  mySpeedReset = false;
  myDesiredSpeed = 0;
  myTurnAmount = 0;
}

AREXPORT void ArActionKeydrive::activate(void)
{
  if (!myIsActive)
    takeKeys();
  myIsActive = true;
}

AREXPORT void ArActionKeydrive::deactivate(void)
{
  if (myIsActive)
    giveUpKeys();
  myIsActive = false;
  myDesiredSpeed = 0;
  myTurnAmount = 0;
}

AREXPORT ArActionDesired *ArActionKeydrive::fire(ArActionDesired currentDesired)
{
  myDesired.reset();

  // if we don't have any strength left
  if (fabs(currentDesired.getVelStrength() - 1.0) < .0000000000001)
  {
    mySpeedReset = true;
  }

  // if our speed was reset, set our desired to how fast we're going now
  if (mySpeedReset && myDesiredSpeed > 0 && myDesiredSpeed > myRobot->getVel())
    myDesiredSpeed = myRobot->getVel();
  if (mySpeedReset && myDesiredSpeed < 0 && myDesiredSpeed < myRobot->getVel())
    myDesiredSpeed = myRobot->getVel();
  mySpeedReset = false;

  if (currentDesired.getMaxVelStrength() && 
      myDesiredSpeed > currentDesired.getMaxVel())
    myDesiredSpeed = currentDesired.getMaxVel();

  if (currentDesired.getMaxNegVelStrength() && 
      myDesiredSpeed < currentDesired.getMaxNegVel())
    myDesiredSpeed = currentDesired.getMaxNegVel();

  myDesiredSpeed += myDeltaVel;
  if (myDesiredSpeed > myTransVelMax)
    myDesiredSpeed = myTransVelMax;
  if (myDesiredSpeed < -myTransVelMax)
    myDesiredSpeed = -myTransVelMax;

  myDesired.setVel(myDesiredSpeed);
  myDeltaVel = 0;
  
  myDesired.setDeltaHeading(myTurnAmount);
  myTurnAmount = 0;

  
  return &myDesired;
}
