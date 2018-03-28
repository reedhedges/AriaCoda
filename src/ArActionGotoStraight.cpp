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
#include "ArActionGotoStraight.h"
#include "ArRobot.h"

#define DEBUG false // if true enables misc debug logging

AREXPORT ArActionGotoStraight::ArActionGotoStraight(const char *name,
						    double speed) :
  ArAction(name, "Goes to the given goal."),
  myPrinting(DEBUG)
{
   setNextArgument(ArArg("speed", &mySpeed, 
			"Speed to travel to goal at. (mm/sec)"));
  mySpeed = speed;
  myState = STATE_NO_GOAL;

  myUseEncoderGoal = false;
  myBacking = false;
  setCloseDist();
  myTurnThresh = 360;
  myMaxTurnSpeed = 0;
  myMaxRotVel = 0;
}

AREXPORT ArActionGotoStraight::~ArActionGotoStraight()
{

}

AREXPORT bool ArActionGotoStraight::haveAchievedGoal(void)
{
  if (myState == STATE_ACHIEVED_GOAL)
    return true;
  else
    return false;
}

AREXPORT void ArActionGotoStraight::cancelGoal(void)
{
  myState = STATE_NO_GOAL;
}

AREXPORT void ArActionGotoStraight::setGoal(ArPose goal, bool backToGoal, 
					    bool justDistance)
{
  myState = STATE_GOING_TO_GOAL;
  myGoal = goal;
  myUseEncoderGoal = false;
  myBacking = backToGoal;
  myLastPose = myRobot->getPose();
  myDist = myRobot->getPose().findDistanceTo(goal);
  myJustDist = justDistance;
  myDistTravelled = 0;
}

AREXPORT void ArActionGotoStraight::setGoalRel(double dist, 
					       double deltaHeading,
					       bool backToGoal, 
					       bool justDistance)
{
  ArPose goal;
  goal.setX(dist * ArMath::cos(deltaHeading));
  goal.setY(dist * ArMath::sin(deltaHeading));
  goal = myRobot->getToGlobalTransform().doTransform(goal);
  setGoal(goal, backToGoal, justDistance);
}

AREXPORT void ArActionGotoStraight::setEncoderGoal(ArPose encoderGoal, 
						   bool backToGoal,
						   bool justDistance)
{
  myState = STATE_GOING_TO_GOAL;
  myEncoderGoal = encoderGoal;
  myUseEncoderGoal = true;
  myBacking = backToGoal;
  myDist = myRobot->getEncoderPose().findDistanceTo(encoderGoal);
  myJustDist = justDistance;
  myDistTravelled = 0;
  myLastPose = myRobot->getEncoderPose();
}

AREXPORT void ArActionGotoStraight::setEncoderGoalRel(double dist, 
						      double deltaHeading,
						      bool backToGoal,
						      bool justDistance)
{
  ArPose goal;
  goal.setX(dist * ArMath::cos(deltaHeading));
  goal.setY(dist * ArMath::sin(deltaHeading));
  goal = myRobot->getToGlobalTransform().doTransform(goal);
  goal = myRobot->getEncoderTransform().doInvTransform(goal);
  setEncoderGoal(goal, backToGoal, justDistance);
}

AREXPORT ArActionDesired *ArActionGotoStraight::fire(ArActionDesired currentDesired)
{
  double angle;
  double dist;
  double distToGo;
  double vel;

  // if we're there we don't do anything
  if (myState == STATE_ACHIEVED_GOAL || myState == STATE_NO_GOAL)
    return NULL;


  ArPose goal;
  if (!myUseEncoderGoal)
  {
    goal = myGoal;
    myDistTravelled += myRobot->getPose().findDistanceTo(myLastPose);
    myLastPose = myRobot->getPose();
  }
  else
  {
    goal = myRobot->getEncoderTransform().doTransform(myEncoderGoal);
    myDistTravelled += myRobot->getEncoderPose().findDistanceTo(myLastPose);
    myLastPose = myRobot->getEncoderPose();
  }

  if (myJustDist)
  {
    distToGo = myDist - myDistTravelled;
    dist = fabs(distToGo);
  }
  else
  {
    dist = myRobot->getPose().findDistanceTo(goal);
  }

  if (((myJustDist && distToGo <= 0) || 
       (!myJustDist && dist < myCloseDist))
      && ArMath::fabs(myRobot->getVel() < 5))
  {
    if(myJustDist)
      ArLog::log(ArLog::Normal, "%s: Achieved distance to goal (dist=%.2f, robotVel=%.2f)", getName(), distToGo, myRobot->getVel());
    else
      ArLog::log(ArLog::Normal, "%s: Achieved goal (dist=%.2f, closeDist=%.2f, robotVel=%.2f)", getName(), dist, myCloseDist, myRobot->getVel());
    myState = STATE_ACHIEVED_GOAL;
    myDesired.setVel(0);
    myDesired.setDeltaHeading(0);
    return &myDesired;  
  }

  // see where we want to point
  angle = myRobot->getPose().findAngleTo(goal);
  if (myBacking)
    angle = ArMath::subAngle(angle, 180);
  myDesired.setHeading(angle);
  const double angleDelta = ArMath::subAngle(myLastPose.getTh(), angle);

  // drive forward (or back if backing up flag).  if we're close, 
  // stop. if the angle turned above is bigger than turnThresh, i
  // don't start driving forward yet.
  if ((myJustDist && distToGo <= 0) || 
      (!myJustDist && dist < myCloseDist))
  {
    myDesired.setVel(0);
    vel = 0;
  }
  else if(fabs(angleDelta) <= myTurnThresh)
  {
    vel = sqrt(dist * 200 * 2);
    if (vel > mySpeed)
      vel = mySpeed;
    if (myBacking)
      vel *= -1;
    myDesired.setVel(vel);
    if(myMaxRotVel > ArMath::epsilon())
      myDesired.setMaxRotVel(myMaxRotVel);
  }
  else
  {
    // no forward motion until we are below myTurnThresh,
    // also use different max speed setting.
    if(myMaxTurnSpeed > ArMath::epsilon())
      myDesired.setMaxRotVel(myMaxTurnSpeed);
  }

  if (myPrinting)
    ArLog::log(ArLog::Normal, "dist %.0f angle %.0f angleDelta %.0f vel %.0f turnThresh %.0f desiredHeading %.0f desiredVel %.0f closeDist %.0f", 
	       dist, angle, angleDelta, vel, myTurnThresh, myDesired.getHeading(), myDesired.getVel(), myCloseDist);
  return &myDesired;
}

