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


*/
#include "Aria/ArExport.h"
#include "Aria/ariaOSDef.h"
#include "Aria/ArSonarDevice.h"
#include "Aria/ArSensorReading.h"
#include "Aria/ArRobot.h"

AREXPORT ArSonarDevice::ArSonarDevice(size_t currentBufferSize,
			     size_t cumulativeBufferSize, const char *name) :
  ArRangeDevice(currentBufferSize, cumulativeBufferSize, name, 5000), 
  myProcessCB(this, &ArSonarDevice::processReadings),
  myIgnoreReadingCB(NULL)
{
  // Remove readings from cumulative buffer if far away from robot after it
  // moves:
  setMaxDistToKeepCumulative(3000); 

  // Do our own filtering of readings that are closer than 50 mm  to each other from 
  //cumulative buffer between cumulative readings, at least:
  myFilterNearDist = 50;

  // Do our own filtering of cumulative readings this far from robot:
  myFilterFarDist = 3000;

  // Remove readings from current buffer if more than 5 seconds old:
  setMaxSecondsToKeepCurrent(5);

  // Remove readings from cumulative buffer if more than 15 seconds old:
  setMaxSecondsToKeepCumulative(15);

  // Visualization properties for GUI clients such as MobileEyes:
  setCurrentDrawingData(new ArDrawingData("polyArrows", 
                                          ArColor(0x33, 0xCC, 0xFF), 
                                          200,  // mm length of arrow
                                          70),  // first sensor layer
                        true);
}

AREXPORT ArSonarDevice::~ArSonarDevice()
{
  if (myRobot != NULL)
  {
    myRobot->remSensorInterpTask(&myProcessCB);
    myRobot->remRangeDevice(this);
  }
}

AREXPORT void ArSonarDevice::setRobot(ArRobot *robot)
{
  myRobot = robot;
  if (myRobot != NULL)
    myRobot->addSensorInterpTask(myName.c_str(), 10, &myProcessCB);
  ArRangeDevice::setRobot(robot);
}

AREXPORT void ArSonarDevice::processReadings()
{
  int i;
  ArSensorReading *reading;
  lockDevice();

  for (i = 0; i < myRobot->getNumSonar(); i++)
  {
    // Get a reference to the ArSensorReading object for sonar sensor #i, which
    // will provide an X,Y position and timestamp for the most recently received
    // obstacle detected by that sonar sensor. ArRobot created a set of 
    // ArSensorReading objects for each sonar sensor  when it connected to the 
    // robot, configured them with the position of the sonar sensor relative to the
    // center of the robot. It calls ArSensorReading::newData() when new sonar
    // data is received to cause the ArSensorReading object to update the X,Y
    // position based on the range value received from the sonar sensor.

    reading = myRobot->getSonarReading(i);
    if (reading == NULL || !reading->isNew(myRobot->getCounter()))
      continue;
    // make sure we don't want to ignore the reading
    if (myIgnoreReadingCB == NULL || !myIgnoreReadingCB->invokeR(reading->getPose()))
      addReading(reading->getX(), reading->getY());
  }

  // delete too-far readings
  myCumulativeBuffer.beginInvalidationSweep();
  const std::list<ArPoseWithTime>& readingList = myCumulativeBuffer.getBuffer();
  const double rx = myRobot->getX();
  const double ry = myRobot->getY();
  // walk through the list and see if this makes any old readings bad
  for (auto it = readingList.begin(); it != readingList.end(); ++it)
  {
    const double dx = it->getX() - rx;
    const double dy = it->getY() - ry;
    if ((dx*dx + dy*dy) > (myFilterFarDist * myFilterFarDist)) 
      myCumulativeBuffer.invalidateReading(it);
  }

  myCumulativeBuffer.endInvalidationSweep();
  // leave this unlock here or the world WILL end
  unlockDevice();
}

/**
   Adds a sonar reading with the global coordinates x,y.  Makes sure the
   reading is within the proper distance to the robot, for
   both current and cumulative buffers.  Filters buffer points 
   Note: please lock the device using lockDevice() / unlockDevice() if
   calling this from outside process().
   @param x the global x coordinate of the reading
   @param y the global y coordinate of the reading
*/
AREXPORT void ArSonarDevice::addReading(double x, double y, bool *wasAdded)
{
  const double rx = myRobot->getX();
  const double ry = myRobot->getY();
  const double rdx = x - rx;		
  const double rdy = y - ry;
  const double dist2 = rdx*rdx + rdy*rdy;
  
  if (dist2 < myMaxRange*myMaxRange)
  {
    myCurrentBuffer.addReading(x,y);
    if(wasAdded) *wasAdded = true;
  }
  else
    if(wasAdded) *wasAdded = false;
  
  if (dist2 < myMaxDistToKeepCumulative * myMaxDistToKeepCumulative)
  {
    myCumulativeBuffer.beginInvalidationSweep();
    const std::list<ArPoseWithTime>& readingList = myCumulativeBuffer.getBuffer();
    // walk through the list and see if this makes any old readings bad
    for (auto it = readingList.begin(); it != readingList.end(); ++it)
    {
      const double dx = it->getX() - x;
      const double dy = it->getY() - y;
      if ((dx*dx + dy*dy) < (myFilterNearDist * myFilterNearDist)) 
        myCumulativeBuffer.invalidateReading(it);
    }
    myCumulativeBuffer.endInvalidationSweep();

    myCumulativeBuffer.addReading(x,y);
  }

}


AREXPORT void ArSonarDevice::setIgnoreReadingCB(
	ArRetFunctor1<bool, ArPose> *ignoreReadingCB)
{
  lockDevice();
  myIgnoreReadingCB = ignoreReadingCB;
  unlockDevice();
}
