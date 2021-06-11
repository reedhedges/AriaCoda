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
#include "Aria/ArSensorReading.h"
#include "Aria/ariaUtil.h"

/**
   @param xPos the x position of the sensor on the robot (mm)
   @param yPos the y position of the sensor on the robot (mm)
   @param thPos the heading of the sensor on the robot (deg)
*/
ArSensorReading::ArSensorReading(double xPos, double yPos, double thPos)
{
  myReading.setPose(-1, -1);
  myReadingTaken.setPose(-1, -1, -1);
  resetSensorPosition(xPos, yPos, thPos, true);
}



AREXPORT void ArSensorReading::newData(unsigned int range, const ArPose& robotPose,
				       const ArPose& encoderPose, const ArTransform& trans, 
				       unsigned int counter,
				       const ArTime& timeTaken,
				       bool ignoreThisReading, int extraInt)
{
  // TODO calculate the x and y position of the sensor
  myRange = range;
  myCounterTaken = counter;
  myReadingTaken = robotPose;
  myEncoderPoseTaken = encoderPose;
  const double rx = getSensorX() + myRange * mySensorCos;
  const double ry = getSensorY() + myRange * mySensorSin;
  myLocalReading.setPose(rx, ry);
  myReading = trans.doTransform(myLocalReading);
  myTimeTaken = timeTaken;
  myIgnoreThisReading = ignoreThisReading;
  myExtraInt = extraInt;
  myAdjusted = false;
}

AREXPORT void ArSensorReading::newData(int sx, int sy, const ArPose& robotPose,
				       const ArPose& encoderPose, const ArTransform& trans, 
				       unsigned int counter, const ArTime& timeTaken,
				       bool ignoreThisReading, int extraInt)
{
  // TODO calculate the x and y position of the sensor
  myRange = (unsigned int)sqrt((double)(sx*sx + sy*sy));
  myCounterTaken = counter;
  myReadingTaken = robotPose;
  myEncoderPoseTaken = encoderPose;
  const double rx = getSensorX() + sx;
  const double ry = getSensorY() + sy;
  myLocalReading.setPose(rx, ry);
  myReading = trans.doTransform(myLocalReading);
  myTimeTaken = timeTaken;
  myIgnoreThisReading = ignoreThisReading;
  myExtraInt = extraInt;
  myAdjusted = false;
}


/**
   @param xPos the x position of the sensor on the robot (mm)
   @param yPos the y position of the sensor on the robot (mm)
   @param thPos the heading of the sensor on the robot (deg)
   @param forceComputation recompute position even if new position is the same as current
*/
AREXPORT void ArSensorReading::resetSensorPosition(double xPos, double yPos, 
						   double thPos, 
						   bool forceComputation)
{
  // if its the same position and we're not forcing, just bail
  if (!forceComputation && fabs(thPos - mySensorPos.getTh()) < .00001 &&
      xPos == mySensorPos.getX() && yPos == mySensorPos.getY())
    return;
      
  mySensorPos.setPose(xPos, yPos, thPos);
  myDistToCenter = sqrt(xPos * xPos + yPos * yPos);
  myAngleToCenter = ArMath::atan2(yPos, xPos);
  mySensorCos = ArMath::cos(thPos);
  mySensorSin = ArMath::sin(thPos);
  //printf("xpose %d ypose %d thpose %d disttoC %.1f angletoC %.1f\n",
  //xPos, yPos, thPos, myDistToCenter, myAngleToCenter);
}

/**
   @param trans the transform to apply to the reading and where the reading was taken
*/
AREXPORT void ArSensorReading::applyTransform(ArTransform trans)
{
  myReading = trans.doTransform(myReading);
  myReadingTaken = trans.doTransform(myReadingTaken);
}

/**
   @param trans the transform to apply to the encoder pose taken
*/
AREXPORT void ArSensorReading::applyEncoderTransform(ArTransform trans)
{
  myEncoderPoseTaken = trans.doTransform(myEncoderPoseTaken);
}
