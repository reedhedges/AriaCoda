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
#include "ArActionLimiterBackwards.h"
#include "ArRobot.h"

/**
   @param name name of the action
   @param stopDistance distance at which to stop (mm)
   @param slowDistance distance at which to slow down (mm)
   @param maxBackwardsSpeed maximum backwards speed, speed allowed scales
     from this to 0 at the stop distance (mm/sec)
   @param widthRatio The ratio of robot width to the width of the region this action checks for sensor readings. 
   @param avoidLocationDependentObstacles If true, stop as the robot nears location-dependent sensed obstacles, if false, ignore them.
*/
AREXPORT ArActionLimiterBackwards::ArActionLimiterBackwards(
	const char *name, double stopDistance, double slowDistance, 
	double maxBackwardsSpeed, double widthRatio, 
	bool avoidLocationDependentObstacles) :
  ArAction(name,
	   "Slows the robot down so as not to hit anything behind it.")
{
  setNextArgument(ArArg("stop distance", &myStopDist, 
			"Distance at which to stop. (mm)"));
  myStopDist = stopDistance;

  setNextArgument(ArArg("slow distance", &mySlowDist, 
			"Distance at which to slow down. (mm)"));
  mySlowDist = slowDistance;

  setNextArgument(ArArg("maximum backwards speed", &myMaxBackwardsSpeed, 
			 "Maximum backwards speed, scales from this to 0 at stopDistance (-mm/sec)"));
  myMaxBackwardsSpeed = maxBackwardsSpeed;

  setNextArgument(ArArg("width ratio", &myWidthRatio, 
			 "The ratio of robot width to how wide an area to check (ratio)"));
  myWidthRatio = widthRatio;

  setNextArgument(ArArg("avoid location dependent obstacles", 
			&myAvoidLocationDependentObstacles, 
			 "Whether to avoid location dependent obstacles or not"));
  myAvoidLocationDependentObstacles = avoidLocationDependentObstacles;
}

AREXPORT ArActionLimiterBackwards::~ArActionLimiterBackwards()
{

}

AREXPORT ArActionDesired *
ArActionLimiterBackwards::fire(ArActionDesired currentDesired)
{
  double dist;
  double maxVel;
  
  double slowStopDist = ArUtil::findMax(myStopDist, mySlowDist);
  

  myDesired.reset();
  dist = myRobot->checkRangeDevicesCurrentBox(
	  // MPL changing this to 0 since right now it won't stop on
	  //bumper hits since they are inside the robot...
	  //-myRobot->getRobotLength()/2,
	  0,
	  -(myRobot->getRobotWidth()/2.0 * myWidthRatio),
	  slowStopDist + (-myRobot->getRobotLength()),
	  (myRobot->getRobotWidth()/2.0 * myWidthRatio),
	  NULL,
	  NULL,
	  myAvoidLocationDependentObstacles);
  dist -= myRobot->getRobotRadius();
  if (dist < -myStopDist)
  {
    //printf("backwards stop\n");
    myDesired.setMaxNegVel(0);
    return &myDesired;
  }
  if (dist > -mySlowDist)
  {
    //printf("backwards nothing\n");
    myDesired.setMaxNegVel(-ArMath::fabs(myMaxBackwardsSpeed));
    return &myDesired;
  }
      
			
  maxVel = -ArMath::fabs(myMaxBackwardsSpeed) * ((-dist - myStopDist) / (mySlowDist - myStopDist));
  //printf("Neg Max vel %f (stopdist %.1f slowdist %.1f slowspeed %.1f\n", maxVel,	 myStopDist, mySlowDist, myMaxBackwardsSpeed);
  myDesired.setMaxNegVel(maxVel);
  return &myDesired;
  
}
