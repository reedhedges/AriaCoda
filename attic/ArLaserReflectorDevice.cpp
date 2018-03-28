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
#include "ArSick.h"
#include "ArLaserReflectorDevice.h"
#include "ArRobot.h"

AREXPORT ArLaserReflectorDevice::ArLaserReflectorDevice(ArRangeDevice *laser,
							ArRobot *robot,
							const char *name) :
  /*
  ArRangeDevice(laser->getCurrentRangeBuffer()->getSize(), 
		laser->getCumulativeRangeBuffer()->getSize(), name,
		laser->getMaxRange()), */
  // MPL 12/11/12 this is how it was... but the part I'm putting in was above commented out, so this may go wrong
  //ArRangeDevice(361, 361, name, 32000),
  ArRangeDevice(laser->getCurrentRangeBuffer()->getSize(), 
		laser->getCumulativeRangeBuffer()->getSize(), name,
		laser->getMaxRange()), 
  myProcessCB(this, &ArLaserReflectorDevice::processReadings)
{
  myLaser = laser;
  myRobot = robot;
  if (myRobot != NULL)
    myRobot->addSensorInterpTask(myName.c_str(), 10, &myProcessCB);
  setCurrentDrawingData(new ArDrawingData("polyDots", 
                                          ArColor(0xb0, 0xb0, 0xff), 
                                          60,  // mm length of arrow
                                          77,  // above the normal laser
					  200, // default refresh
					  "DefaultOff"), // defaults to off but can be turned on
			true);
  myReflectanceThreshold = 31;
}

AREXPORT ArLaserReflectorDevice::~ArLaserReflectorDevice()
{
  if (myRobot != NULL)
    myRobot->remSensorInterpTask(&myProcessCB);
}

AREXPORT void ArLaserReflectorDevice::setRobot(ArRobot *robot)
{
  // specifically do nothing since this is just here for debugging
}

AREXPORT void ArLaserReflectorDevice::addToConfig(ArConfig *config, 
						  const char *section)
{

  config->addSection(ArConfig::CATEGORY_ROBOT_PHYSICAL,
                     section,
                     "Settings for using the reflector readings from this laser");

  config->addParam(
	  ArConfigArg("ReflectanceThreshold", &myReflectanceThreshold,
		      "The threshold to start showing reflector readings at (normalized from 0 to 255, 31 is the default)", 
		      0, 255),
	  section, ArPriority::DETAILED);
		      
}

AREXPORT void ArLaserReflectorDevice::processReadings(void)
{
  //int i;
  ArSensorReading *reading;
  myLaser->lockDevice();
  lockDevice();
  
  const std::list<ArSensorReading *> *rawReadings;
  std::list<ArSensorReading *>::const_iterator rawIt;
  rawReadings = myLaser->getRawReadings();
  myCurrentBuffer.beginRedoBuffer();

  if (myReflectanceThreshold < 0 || myReflectanceThreshold > 255)
    myReflectanceThreshold = 0;

  if (rawReadings->begin() != rawReadings->end())
  {
    for (rawIt = rawReadings->begin(); rawIt != rawReadings->end(); rawIt++)
    {
      reading = (*rawIt);
      if (!reading->getIgnoreThisReading() && 
	  reading->getExtraInt() > myReflectanceThreshold)
	myCurrentBuffer.redoReading(reading->getPose().getX(), 
				    reading->getPose().getY());
    }
  }

  myCurrentBuffer.endRedoBuffer();

  unlockDevice();
  myLaser->unlockDevice();
}

