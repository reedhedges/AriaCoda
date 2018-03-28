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
#include "ArActionGoto.h"
#include "ArRobot.h"

AREXPORT ArActionGoto::ArActionGoto(const char *name, ArPose goal, 
				    double closeDist, double speed,
				    double speedToTurnAt, double turnAmount) :
  ArAction(name, "Goes to the given goal.")
{
  myDirectionToTurn = myCurTurnDir = 1;
  myTurnedBack = false;
  myPrinting = false;

  setNextArgument(ArArg("goal", &myGoal, "ArPose to go to. (ArPose)"));
  setGoal(goal);
  
  setNextArgument(ArArg("close dist", &myCloseDist, 
			"Distance that is close enough to goal. (mm)"));
  myCloseDist = closeDist;

  setNextArgument(ArArg("speed", &mySpeed, 
			"Speed to travel to goal at. (mm/sec)"));
  mySpeed = speed;

  setNextArgument(ArArg("speed to turn at", &mySpeedToTurnAt,
			"Speed to start obstacle avoiding at (mm/sec)"));
  mySpeedToTurnAt = speedToTurnAt;
  
  setNextArgument(ArArg("amount to turn", &myTurnAmount,
			"Amount to turn when avoiding (deg)"));
  myTurnAmount = turnAmount;
  
}

AREXPORT ArActionGoto::~ArActionGoto()
{

}

AREXPORT bool ArActionGoto::haveAchievedGoal(void)
{
  if (myState == STATE_ACHIEVED_GOAL)
    return true;
  else
    return false;
}

AREXPORT void ArActionGoto::cancelGoal(void)
{
  myState = STATE_NO_GOAL;
}

AREXPORT void ArActionGoto::setGoal(ArPose goal)
{
  myState = STATE_GOING_TO_GOAL;
  myGoal = goal;
  myTurnedBack = false;
  myCurTurnDir = myDirectionToTurn;
  myOldGoal = myGoal;
}

AREXPORT ArActionDesired *ArActionGoto::fire(ArActionDesired currentDesired)
{
  double angle;
  double dist;
  double vel;

  /* If myGoal changed since the last time setGoal() was called (it's an action
   * argument) call setGoal() to reset to the new goal.
   */
  if (myGoal.findDistanceTo(myOldGoal) > 5) 
    setGoal(myGoal);
  
  // if we're there we don't do anything
  if (myState == STATE_ACHIEVED_GOAL || myState == STATE_NO_GOAL)
    return NULL;

  dist = myRobot->getPose().findDistanceTo(myGoal);
  if (dist < myCloseDist && ArMath::fabs(myRobot->getVel() < 5))
  {
    if (myPrinting)
      printf("Achieved goal\n");
    myState = STATE_ACHIEVED_GOAL;
    myDesired.setVel(0);
    myDesired.setDeltaHeading(0);
    return &myDesired;
  }

  if (myPrinting)
    printf("%.0f  ", dist);
  // see where we want to point
  angle = myRobot->getPose().findAngleTo(myGoal);

  if (ArMath::fabs(ArMath::subAngle(angle, myRobot->getTh())) > 120)
  {
    myCurTurnDir *= -1;
  }


  // see if somethings in front of us
  if (currentDesired.getMaxVelStrength() > 0 &&
      currentDesired.getMaxVel() < mySpeedToTurnAt)
  {
    if (myPrinting)
      printf("Something slowing us down.  ");
    myDesired.setDeltaHeading(myTurnAmount * myCurTurnDir);
  }
  else
  {
    if (myPrinting)
      printf("Can speed up and turn back again.  ");
    // see if we want to just point at the goal or not
    if (ArMath::fabs(
	    ArMath::subAngle(angle, 
			     ArMath::addAngle(myTurnAmount * 
					      myCurTurnDir * -1, 
					      myRobot->getTh())))
	> myTurnAmount/2)
    {
      if (myPrinting)
	printf("Pointing to goal  ");
      myDesired.setHeading(angle);
    }
    else
    {
      if (myPrinting)
	printf("turning back  ");
      myDesired.setDeltaHeading(myTurnAmount * myCurTurnDir * -1);
    }
  }
  if (dist < myCloseDist && ArMath::fabs(myRobot->getVel() < 5))
  {
    if (myPrinting)
      printf("#achieved\n");
    myState = STATE_ACHIEVED_GOAL;
    myDesired.setVel(0);
    myDesired.setDeltaHeading(0);
  }
  // if we're close, stop
  else if (dist < myCloseDist)
  {
    if (myPrinting)
      printf("#stop\n");
    myDesired.setVel(0);
  }
  else
  {
    vel = sqrt(dist * 200 * 2);
    if (vel > mySpeed)
      vel = mySpeed;
    if (myPrinting)
      printf("#go %.0f\n", vel);
    myDesired.setVel(vel);
  }
  return &myDesired;
}
