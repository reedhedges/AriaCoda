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
#include "ArActionLimiterForwards.h"
#include "ArRobot.h"
#include "ArRangeDevice.h"

/**
   @param name name of the action
   @param stopDistance distance at which to stop (mm)
   @param slowDistance distance at which to slow down (mm)
   @param slowSpeed speed allowed at slowDistance, scales to 0 at slow 
   distance (mm/sec)
   @param widthRatio Ratio of the width of the box to look at to the robot radius (multiplier)
*/
AREXPORT ArActionLimiterForwards::ArActionLimiterForwards(const char *name, 
							  double stopDistance,
							  double slowDistance,
							  double slowSpeed,
							  double widthRatio) :
  ArAction(name,
	   "Slows the robot down so as not to hit anything in front of it.")
{
  setNextArgument(ArArg("stop distance", &myStopDist, 
			"Distance at which to stop. (mm)"));
  myStopDist = stopDistance;

  setNextArgument(ArArg("slow distance", &mySlowDist, 
			"Distance at which to slow down. (mm)"));
  mySlowDist = slowDistance;

  setNextArgument(ArArg("slow speed", &mySlowSpeed, 
			 "Speed at which to slow to at the slow distance, (mm/sec)"));
  mySlowSpeed = slowSpeed;
  
  setNextArgument(ArArg("width ratio", &myWidthRatio,
			"Ratio of the width of the box to look at to the robot radius (multiplier)"));
  myWidthRatio = widthRatio;
  myLastStopped = false;
  myLastSensorReadingDev = NULL;
}

AREXPORT ArActionLimiterForwards::~ArActionLimiterForwards()
{

}

/**
   @param stopDistance distance at which to stop (mm)
   @param slowDistance distance at which to slow down (mm)
   @param slowSpeed speed allowed at slowDistance, scales to 0 at slow 
   distance (mm/sec)
   @param widthRatio Ratio of the width of the box to look at to the robot radius (multiplier)
*/
AREXPORT void ArActionLimiterForwards::setParameters(double stopDistance,
						     double slowDistance,
						     double slowSpeed,
						     double widthRatio)
{
  myStopDist = stopDistance;
  mySlowDist = slowDistance;
  mySlowSpeed = slowSpeed;
  myWidthRatio = widthRatio;
}

AREXPORT ArActionDesired *
ArActionLimiterForwards::fire(ArActionDesired currentDesired)
{
  double dist;
  double maxVel;
  bool printing = false;
  double checkDist;

  if (myStopDist > mySlowDist)
    checkDist = myStopDist;
  else
    checkDist = mySlowDist;

  myDesired.reset();
  dist = myRobot->checkRangeDevicesCurrentBox(
        0,
				-myRobot->getRobotWidth()/2.0 * myWidthRatio,
  			checkDist + myRobot->getRobotLength()/2,
				myRobot->getRobotWidth()/2.0 * myWidthRatio,
        &myLastSensorReadingPos,
        &myLastSensorReadingDev
  );
  dist -= myRobot->getRobotLength() / 2;
  //printf("%.0f\n", dist);

  if (dist < myStopDist)
  {
    if(!myLastStopped)
    {
      if (printing) printf("Stopping\n");
      ArLog::log(ArLog::Verbose, "%s: Stopping due to sensor reaoding", getName());
    }
    myLastStopped = true;
    myDesired.setMaxVel(0);
    return &myDesired;
  }

  if(myLastStopped)
  {
    if (printing) printf("Going\n");
    ArLog::log(ArLog::Verbose, "%s: Allowing motion", getName());
  } 

  myLastStopped = false;
  //printf("%f ", dist);
  if (dist > mySlowDist)
  {
    //printf("Nothing\n");
    return NULL;
    //return &myDesired;
  }
      
			
  maxVel = mySlowSpeed * ((dist - myStopDist) / (mySlowDist - myStopDist));
  //printf("Max vel %f (stopdist %.1f slowdist %.1f slowspeed %.1f\n", maxVel,	 myStopDist, mySlowDist, mySlowSpeed);
  myDesired.setMaxVel(maxVel);
  return &myDesired;
  
}
