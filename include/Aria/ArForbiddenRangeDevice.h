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
#ifndef ARFORBIDDENRANGEDEVICE_H
#define ARFORBIDDENRANGEDEVICE_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ArRangeDevice.h"

class ArMapInterface;

/// Class that takes forbidden lines and turns them into range readings
///  @ingroup OptionalClasses
class ArForbiddenRangeDevice : public ArRangeDevice
{
public:
  /// Constructor
  AREXPORT explicit ArForbiddenRangeDevice(ArMapInterface *armap, 
                                  double distanceIncrement = 100,
				                          unsigned int maxRange = 4000,
				                          const char *name = "forbidden");
  //AREXPORT virtual ~ArForbiddenRangeDevice();
  /// Saves the forbidden lines from the map
  AREXPORT void processMap();
  /// Remakes the readings 
  AREXPORT void processReadings();
  /// Sets the robot pointer and attaches its process function
  AREXPORT virtual void setRobot(ArRobot *robot);

  /// Enable readings 
  AREXPORT void enable();
  /// Disables readings until reenabled
  AREXPORT void disable();
  /// Sees if this device is active or not
  AREXPORT bool isEnabled() const { return myIsEnabled;; }
  /// Gets a callback to enable the device
  AREXPORT ArFunctor *getEnableCB() { return &myEnableCB; } 
  /// Gets a callback to disable the device
  AREXPORT ArFunctor *getDisableCB() { return &myDisableCB; } 
protected:
  ArMutex myDataMutex;
  ArMapInterface *myMap;
  double myDistanceIncrement;
  std::vector<ArLineSegment> mySegments;
  ArFunctorC<ArForbiddenRangeDevice> myProcessCB;
  ArFunctorC<ArForbiddenRangeDevice> myMapChangedCB;
  bool myIsEnabled;
  ArFunctorC<ArForbiddenRangeDevice> myEnableCB;
  ArFunctorC<ArForbiddenRangeDevice> myDisableCB;
};

#endif // ARFORBIDDENRANGEDEVICE_H
