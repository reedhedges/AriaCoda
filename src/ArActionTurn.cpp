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
#include "ArActionTurn.h"
#include "ArRobot.h"

AREXPORT ArActionTurn::ArActionTurn(const char *name, double speedStartTurn,
				    double speedFullTurn, double turnAmount) :
  ArAction(name, 
	   "Turns the robot depending on actions by higher priority actions.")
{
  setNextArgument(ArArg("speed start turn", &mySpeedStart,
			"max vel at which to start turning (mm/sec)"));
  mySpeedStart = speedStartTurn;

  setNextArgument(ArArg("speed full turn", &mySpeedFull,
			"max vel at which to turn the full amount (mm/sec)"));
  mySpeedFull = speedFullTurn;

  setNextArgument(ArArg("turn amount", &myTurnAmount,
			"max vel at which to start turning (mm/sec)"));
  myTurnAmount = turnAmount;

  myTurning = 0;

}

AREXPORT ArActionTurn::~ArActionTurn()
{

}

AREXPORT ArActionDesired *ArActionTurn::fire(ArActionDesired currentDesired)
{
  myDesired.reset();
  double turnAmount;
  double angle;

  // if there's no strength, bail
  // if our max velocity is higher than our start turn, bail
  if (myRobot->getVel() > mySpeedStart)
  {
    if (myTurning != 0)
    {
      //printf("Resetting\n");
      myTurning = 0;
    }
    return NULL;
  }

  // we're going to turn now... so figure out the amount to turn

  // if our max vel is lower than the turn amount, just do the full turn
  if (myRobot->getVel() < mySpeedFull)
  {
    //printf("full\n");
    turnAmount = myTurnAmount;
  }
  // otherwise scale it
  else
  {
    turnAmount = ((mySpeedStart - myRobot->getVel()) /
		  (mySpeedStart - mySpeedFull)) * myTurnAmount;
  
    //printf("%.2f\n", (mySpeedStart - currentDesired.getMaxVel()) / (mySpeedStart - mySpeedFull));
  }
  // if we're already turning, turn that direction
  if (myTurning != 0)
    turnAmount *= myTurning;
  // find out which side the closest obstacle is, and turn away
  else
  {
    if (myRobot->checkRangeDevicesCurrentPolar(-90, 90, &angle) < 3000)
    {
      if (angle > 0)
      {
	//printf("### right\n");
	myTurning = -1;
      }
      else
      {
	//printf("### left\n");
	myTurning = 1;
      }
    }
    else
    {
      //printf("### left\n");
      myTurning = 1;
    }
    turnAmount *= myTurning;
  }
  myDesired.setDeltaHeading(turnAmount);
  return &myDesired;
}
