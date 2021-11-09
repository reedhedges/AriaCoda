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
#include "Aria/ArRobot.h"
#include "Aria/ariaUtil.h"
#include "Aria/ArForbiddenRangeDevice.h"
#include "Aria/ArMapInterface.h"

/**
   This will take a map and then convert the forbidden lines into
   range device readings every cycle.

   It doesn't add any data for drawing the current range device
   readings since the lines themselves show up in the map.

   @param armap the map to use forbidden lines from
   @param distanceIncrement how many mm to increment search in sensor readings for forbidden lines
   @param maxRange how far out to look total
   @param name the name of the device
 **/
AREXPORT ArForbiddenRangeDevice::ArForbiddenRangeDevice(
	ArMapInterface *armap, double distanceIncrement, unsigned int maxRange,
	const char *name) :
  ArRangeDevice(INT_MAX, 0, name, maxRange, 0, 0, 0, true),
  myDataMutex(),
  myMap(armap),
  myDistanceIncrement(distanceIncrement),
  myProcessCB(this, &ArForbiddenRangeDevice::processReadings),
  myMapChangedCB(this, &ArForbiddenRangeDevice::processMap)  ,
  myIsEnabled(true),
  myEnableCB(this, &ArForbiddenRangeDevice::enable),
  myDisableCB(this, &ArForbiddenRangeDevice::disable)
{
  myDataMutex.setLogName("ArForbiddenRangeDevice::myDataMutex");
  
  myMapChangedCB.setName("ArForbiddenRangeDevice");
}

/* AREXPORT ArForbiddenRangeDevice::~ArForbiddenRangeDevice()
{

} */

AREXPORT void ArForbiddenRangeDevice::processMap()
{
  myDataMutex.lock();
//  ArUtil::deleteSet(mySegments.begin(), mySegments.end());
  mySegments.clear();
  mySegments.reserve(myMap->getMapObjects().size() * 4);
  for (auto it = myMap->getMapObjects().cbegin();
       it != myMap->getMapObjects().cend();
       ++it)
  {
    const ArMapObject *obj = *it;
    if (strcmp(obj->getType(), "ForbiddenLine") == 0 && obj->hasFromTo())
    {
      mySegments.push_back(ArLineSegment(obj->getFromPose(), 
					     obj->getToPose()));
    }
    if (strcmp(obj->getType(), "ForbiddenArea") == 0 && obj->hasFromTo())
    {
      const double angle = obj->getPose().getTh();
      const double sa = ArMath::sin(angle);
      const double ca = ArMath::cos(angle);
      const double fx = obj->getFromPose().getX();
      const double fy = obj->getFromPose().getY();
      const double tx = obj->getToPose().getX();
      const double ty = obj->getToPose().getY();
      const ArPose P0((fx*ca - fy*sa), (fx*sa + fy*ca));
      const ArPose P1((tx*ca - fy*sa), (tx*sa + fy*ca));
      const ArPose P2((tx*ca - ty*sa), (tx*sa + ty*ca));
      const ArPose P3((fx*ca - ty*sa), (fx*sa + ty*ca));
      //mySegments.push_back(ArLineSegment(P0, P1));
      //mySegments.push_back(ArLineSegment(P1, P2));
      //mySegments.push_back(ArLineSegment(P2, P3));
      //mySegments.push_back(ArLineSegment(P3, P0));
      mySegments.emplace_back(P0, P1);
      mySegments.emplace_back(P1, P2);
      mySegments.emplace_back(P2, P3);
      mySegments.emplace_back(P3, P0);
    }
  }
  myDataMutex.unlock();
}

AREXPORT void ArForbiddenRangeDevice::processReadings()
{
  lockDevice();
  myDataMutex.lock();

  myCurrentBuffer.beginRedoBuffer();

  if (!myIsEnabled)
  {
    myCurrentBuffer.endRedoBuffer();
    myDataMutex.unlock();
    unlockDevice();
    return;
  }

  const double robotX = myRobot->getX();
  const double robotY = myRobot->getY();
  const double max = (double) myMaxRange;
  const double maxSquared = (double) myMaxRange * (double) myMaxRange;
  ArTime startingTime;
  //startingTime.setToNow();
  // now see if the end points of the segments are too close to us
  for (auto it = mySegments.begin(); it != mySegments.end(); it++)
  {
    // if either end point or some perpindicular point is close to us
    // add the line's data
    if (ArMath::squaredDistanceBetween(
        it->getX1(), it->getY1(), 
        myRobot->getX(), myRobot->getY()) < maxSquared ||
      ArMath::squaredDistanceBetween(
        it->getX2(), it->getY2(), 
        myRobot->getX(), myRobot->getY()) < maxSquared ||
      it->getPerpDist(myRobot->getPose()) < max)
    {
      const ArPose start(it->getX1(), it->getY1());
      const ArPose end(it->getX2(), it->getY2());
      const double angle = start.findAngleTo(end);
      const double cos = ArMath::cos(angle);
      const double sin = ArMath::sin(angle);
      const double startX = start.getX();
      const double startY = start.getY();
      const double length = start.findDistanceTo(end);
      // first put in the start point if we should
      if (ArMath::squaredDistanceBetween(startX, startY, robotX, robotY) < maxSquared)
        myCurrentBuffer.redoReading(start.getX(), start.getY());
      // now walk the length of the line and see if we should put the points in
      for (double gone = 0; gone < length; gone += myDistanceIncrement)
      {
        const double atX = startX + gone * cos;
        const double atY = startY + gone * sin;
        if (ArMath::squaredDistanceBetween(atX, atY, robotX, robotY) < maxSquared)
          myCurrentBuffer.redoReading(atX, atY);
      }
      // now check the end point
      if (end.squaredFindDistanceTo(myRobot->getPose()) < maxSquared)
        myCurrentBuffer.redoReading(end.getX(), end.getY());
    }
  }
  myDataMutex.unlock();
  // and we're done
  myCurrentBuffer.endRedoBuffer();
  unlockDevice();
  //printf("%d\n", startingTime.mSecSince());
}

AREXPORT void ArForbiddenRangeDevice::setRobot(ArRobot *robot)
{
  myRobot = robot;
  if (myRobot != NULL)
    myRobot->addSensorInterpTask(myName.c_str(), 20, &myProcessCB);
  ArRangeDevice::setRobot(robot);
  myMap->lock();
  myMap->addMapChangedCB(&myMapChangedCB);
  processMap();
  myMap->unlock();
}

AREXPORT void ArForbiddenRangeDevice::enable()
{
  myDataMutex.lock();
  myIsEnabled = true;
  myDataMutex.unlock();
}

AREXPORT void ArForbiddenRangeDevice::disable()
{
  myDataMutex.lock();
  myIsEnabled = false;
  myDataMutex.unlock();
}
