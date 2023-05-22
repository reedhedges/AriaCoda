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
#ifndef ARIRS_H
#define ARIRS_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ArRangeDevice.h"
#include "Aria/ArRobotParams.h"
#include "Aria/ArFunctor.h"

#include <vector>

/// A class that treats a robot's infrared sensors as a range device.
/**
  (Only Peoplebot and some Powerbots have IR sensors.)

  @ingroup OptionalClasses  
  @ingroup DeviceClasses
*/
class ArIRs final : public ArRangeDevice
{
public:
  AREXPORT ArIRs(size_t currentBufferSize = 10, 
		     size_t cumulativeBufferSize = 10,
		     const char *name = "irs",
		     int maxSecondsToKeepCurrent = 15);
  AREXPORT virtual ~ArIRs();

  AREXPORT virtual void setRobot(ArRobot *robot) override;
  AREXPORT void processReadings();

protected:
  ArFunctorC<ArIRs> myProcessCB;
  const ArRobotParams *myParams = nullptr;
  std::vector<int> cycleCounters;
};


#endif // ARIRS_H
