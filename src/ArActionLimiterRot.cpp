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
#include "ArActionLimiterRot.h"
#include "ArRobot.h"
#include "ArCommands.h"
#include "ariaInternal.h"
#include "ArRobotConfigPacketReader.h"
#include "ArRangeDevice.h"

/**
   @param name name of the action
*/
AREXPORT ArActionLimiterRot::ArActionLimiterRot(
	const char *name) :
  ArAction(name,
      "Applies a limit to rotation if there are sensor readings in the radius")
{
  setParameters();

  myUseLocationDependentDevices = true;
}

AREXPORT ArActionLimiterRot::~ArActionLimiterRot()
{

}

/**
   @param checkRadius True to check the radius and apply this speed, false not to
   @param inRadiusSpeed the speed to go at if something is in the radius
   
*/
AREXPORT void ArActionLimiterRot::setParameters(
	bool checkRadius, double inRadiusSpeed)
{
  myCheckRadius = checkRadius;
  myInRadiusSpeed = inRadiusSpeed;
}

AREXPORT void ArActionLimiterRot::addToConfig(ArConfig *config, 
						       const char *section, 
						       const char *prefix)
{
  std::string strPrefix;
  std::string name;
  if (prefix == NULL || prefix[0] == '\0')
    strPrefix = "";
  else
    strPrefix = prefix;

  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::NORMAL);  

  name = strPrefix;
  name += "CheckRadius";
  config->addParam(
	  ArConfigArg(name.c_str(), &myCheckRadius, 
		      "True to check the robot's radius and apply InRadiusSpeed, false not to"), 
	  section, ArPriority::NORMAL);

  name = strPrefix;
  name += "InRadiusSpeed";
  config->addParam(
	  ArConfigArg(name.c_str(), &myInRadiusSpeed, 
		      "Maximum speed to allow if CheckRadius is true and there are sensor readings in the robot's radius, 0 or less means allow no rotation (deg/sec)"), 
	  section, ArPriority::NORMAL);

  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::NORMAL);
}

AREXPORT ArActionDesired *
ArActionLimiterRot::fire(ArActionDesired currentDesired)
{
  bool printing = false;


  if (!myCheckRadius)
  {
    if (printing)
      printf("Nothing\n");
    return NULL;
  }

  double leftDist;
  const ArRangeDevice *leftRangeDevice = NULL;
  double rightDist;
  const ArRangeDevice *rightRangeDevice = NULL;

  double dist;
  //const ArRangeDevice *rangeDevice = NULL;

  //ArLog::LogLevel verboseLogLevel = ArLog::Verbose;
  //ArLog::LogLevel verboseLogLevel = ArLog::Verbose;
  //if (printing)
  //  verboseLogLevel = ArLog::Normal;


  leftDist = myRobot->checkRangeDevicesCurrentPolar(
	  0, 179.999, NULL, &leftRangeDevice, myUseLocationDependentDevices);

  rightDist = myRobot->checkRangeDevicesCurrentPolar(
	  -179.999, 0, NULL, &rightRangeDevice, myUseLocationDependentDevices);
  
  if (leftDist > 0 && rightDist < 0)
  {
    dist = leftDist;
    //rangeDevice = leftRangeDevice;
  }
  else if (rightDist > 0 && leftDist < 0)
  {
    dist = rightDist;
    //rangeDevice = rightRangeDevice;
  }
  else if (leftDist > 0 && rightDist > 0)
  {
    if (leftDist < rightDist)
    {
      dist = leftDist;
      //rangeDevice = leftRangeDevice;
    }
    else 
    {
      dist = rightDist;
      //rangeDevice = rightRangeDevice;
    }
  }
  else
  {
    dist = -1;
    //rangeDevice = NULL;
  }

  if (printing)
    printf("left %.0f right %.0f dist %.0f robotRadius %.0f\n", 
	   leftDist, rightDist, dist, myRobot->getRobotRadius());

  if (dist > 0 && dist < myRobot->getRobotRadius())
  {
    if (myInRadiusSpeed <= 0)
    {
      if (printing)
	printf("rot of zero\n");
      myDesired.setMaxRotVel(0);
    }
    else
    {
      if (printing)
	printf("rot of %.0f\n", myInRadiusSpeed);
      myDesired.setMaxRotVel(myInRadiusSpeed);
    }
    return &myDesired;
  }
  else
  {
    if (printing)
      printf("Nothing at the end\n");
    return NULL;
  }
}



