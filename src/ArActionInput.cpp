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
#include "ArActionInput.h"
#include "ArRobot.h"
/**
   @param name name of the action
*/
AREXPORT ArActionInput::ArActionInput(const char *name) :
    ArAction(name, "Inputs vel and heading")
{
  clear();
}

AREXPORT ArActionInput::~ArActionInput()
{
}

AREXPORT void ArActionInput::setVel(double vel)
{
  myUsingVel = true;
  myVelSet = vel;
}

AREXPORT void ArActionInput::setRotVel(double rotVel)
{
  myRotRegime = ROTVEL;
  myRotVal = rotVel;
}

AREXPORT void ArActionInput::deltaHeadingFromCurrent(double delta)
{
  myRotRegime = DELTAHEADING;
  myRotVal = delta;
}

AREXPORT void ArActionInput::setHeading(double heading)
{
  myRotRegime = SETHEADING;
  myRotVal = heading;
}

AREXPORT void ArActionInput::clear(void)
{
  myUsingVel = false;
  myRotRegime = NONE;
}

AREXPORT ArActionDesired *ArActionInput::fire(
	ArActionDesired currentDesired)
{
  myDesired.reset();

  if (myUsingVel)
    myDesired.setVel(myVelSet);
  
  if (myRotRegime == ROTVEL)
    myDesired.setRotVel(myRotVal);
  else if (myRotRegime == DELTAHEADING)
  {
    myDesired.setDeltaHeading(myRotVal);
    myRotVal = 0;
  }
  else if (myRotRegime == SETHEADING)
    myDesired.setHeading(myRotVal);
  else if (myRotRegime != NONE)
    ArLog::log(ArLog::Normal, "ArActionInput::fire: Bad rot regime %d", 
	       myRotRegime);

  return &myDesired;
}
