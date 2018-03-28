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
#include "ArActionDriveDistance.h"
#include "ArRobot.h"

AREXPORT ArActionDriveDistance::ArActionDriveDistance(const char *name,
						      double speed,
						      double deceleration) :
  ArAction(name, "Drives a given distance.")
{
  myPrinting = false;

  setNextArgument(ArArg("speed", &mySpeed, 
			"Speed to travel to at. (mm/sec)"));
  setNextArgument(ArArg("deceleration", &myDeceleration, 
			"Speed to decelerate at. (mm/sec/sec)"));
  mySpeed = speed;
  myDeceleration = deceleration;
  myState = STATE_NO_DISTANCE;
}

AREXPORT ArActionDriveDistance::~ArActionDriveDistance()
{

}

AREXPORT bool ArActionDriveDistance::haveAchievedDistance(void)
{
  if (myState == STATE_ACHIEVED_DISTANCE)
    return true;
  else
    return false;
}

AREXPORT void ArActionDriveDistance::cancelDistance(void)
{
  myState = STATE_NO_DISTANCE;
}

AREXPORT void ArActionDriveDistance::setDistance(
	double distance, bool useEncoders)
{
  myState = STATE_GOING_DISTANCE;
  myDistance = distance;
  myUseEncoders = useEncoders;
  if (myUseEncoders)
    myLastPose = myRobot->getEncoderPose();
  else
    myLastPose = myRobot->getPose();
  myDistTravelled = 0;
  myLastVel = 0;
}


AREXPORT ArActionDesired *ArActionDriveDistance::fire(
	ArActionDesired currentDesired)
{
  double distToGo;
  double vel;

  // if we're there we don't do anything
  if (myState == STATE_ACHIEVED_DISTANCE || myState == STATE_NO_DISTANCE)
    return NULL;


  if (myUseEncoders)
  {
    myDistTravelled += myRobot->getEncoderPose().findDistanceTo(myLastPose);
    myLastPose = myRobot->getEncoderPose();
  }
  else
  {
    myDistTravelled += myRobot->getPose().findDistanceTo(myLastPose);
    myLastPose = myRobot->getPose();
  }

  if (myDistance >= 0)
    distToGo = myDistance - myDistTravelled;
  else
    distToGo = -myDistance - myDistTravelled;

  if (distToGo <= 0 && ArMath::fabs(myRobot->getVel() < 5))
  {
    if (myPrinting)
    {
      ArLog::log(ArLog::Normal, 
		 "Achieved distToGo %.0f realVel %.0f realVelDelta %.0f", 
		 distToGo, myRobot->getVel(), myRobot->getVel() - myLastVel);
    }
    myState = STATE_ACHIEVED_DISTANCE;
    myDesired.setVel(0);
    myDesired.setRotVel(0);
    return &myDesired;  
  }

  myDesired.setRotVel(0);
  // if we're close, stop
  if (distToGo <= 0)
  {
    myDesired.setVel(0);
    vel = 0;
  }
  else
  {
    vel = sqrt(distToGo * myDeceleration * 2);
    if (vel > mySpeed)
      vel = mySpeed;
    if (myDistance < 0)
      vel *= -1;
    myDesired.setVel(vel);
  }
  if (myPrinting)
    ArLog::log(ArLog::Normal, 
	       "distToGo %.0f cmdVel %.0f realVel %.0f realVelDelta %.0f", 
	       distToGo, vel, myRobot->getVel(), 
	       myRobot->getVel() - myLastVel);
  myLastVel = myRobot->getVel();
  return &myDesired;
}

