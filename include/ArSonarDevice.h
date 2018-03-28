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
#ifndef ARSONARDEVICE_H
#define ARSONARDEVICE_H

#include "ariaTypedefs.h"
#include "ArRangeDevice.h"
#include "ArFunctor.h"

#include "ArRobot.h"

/// Keep track of recent sonar readings from a robot as an ArRangeDevice
/** 
    This class is for keeping a sonar history, which you may use for obstacle 
    avoidance, display, etc.
    Simply use ArRobot::addRangeDevice()  (or ArSonarDevice::setRobot())
    to attach an ArSonarDevice object to an ArRobot
    robot object; ArSonarDevice will add a Sensor Interpretation task to the
    ArRobot which will read new sonar readings each robot cycle and add
    them to its sonar history.

    (Note that sonar range readings are from the surface of the sonar transducer disc,
    not from the center of the robot.)

    @ingroup ImportantClasses
   @ingroup DeviceClasses
*/
class ArSonarDevice : public ArRangeDevice
{
public:
  /// Constructor
  AREXPORT ArSonarDevice(size_t currentBufferSize = 24, 
			 size_t cumulativeBufferSize = 64, 
			 const char * name = "sonar");
  /// Destructor
  AREXPORT virtual ~ArSonarDevice();
  /// Grabs the new readings from the robot and adds them to the buffers
  /// (Primarily for internal use.)
  AREXPORT void processReadings(void);

  /// Sets the robot pointer, also attaches its process function to the
  /// robot as a Sensor Interpretation task.
  AREXPORT virtual void setRobot(ArRobot *robot);

  /// Adds sonar readings to the current and cumulative buffers
  /// Overrides the ArRangeDevice default action.
  /// (This method is primarily for internal use.)
  AREXPORT virtual void addReading(double x, double y);

  /// Sets a callback which if it returns true will ignore the reading
  AREXPORT void setIgnoreReadingCB(ArRetFunctor1<bool, ArPose> *ignoreReadingCB);
 
  /// Gets the callback which if it returns true will ignore the reading
  AREXPORT ArRetFunctor1<bool, ArPose> *getIgnoreReadingCB(void)
    { return myIgnoreReadingCB; }

  /** @deprecated
   *  @sa ArRangeDevice::setMaxDistToKeepCumulative()
   */
  AREXPORT void setCumulativeMaxRange(double range) 
    { setMaxDistToKeepCumulative(range); }
protected:
  ArFunctorC<ArSonarDevice> myProcessCB;
  double myFilterNearDist;	// we throw out cumulative readings this close to current one
  double myFilterFarDist;	// throw out cumulative readings beyond this far from robot

  ArRetFunctor1<bool, ArPose> *myIgnoreReadingCB;
};


#endif // ARSONARDEVICE_H
