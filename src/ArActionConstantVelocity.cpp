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
#include "ArActionConstantVelocity.h"

/**
   @param name name of the action
   @param velocity velocity to travel at (mm/sec)
*/
AREXPORT ArActionConstantVelocity::ArActionConstantVelocity(const char *name,
						   double velocity) :
  ArAction(name, "Sets the robot to travel straight at a constant velocity.")
{
  setNextArgument(ArArg("velocity", &myVelocity, 
			"The velocity to make the robot travel at. (mm/sec)"));
  myVelocity = velocity;  
}

AREXPORT ArActionConstantVelocity::~ArActionConstantVelocity()
{

}

AREXPORT ArActionDesired *ArActionConstantVelocity::fire(
	ArActionDesired currentDesired)
{
  myDesired.reset();

  myDesired.setVel(myVelocity);
  myDesired.setDeltaHeading(0);

  return &myDesired;
}
