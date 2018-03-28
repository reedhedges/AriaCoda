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
#ifndef ARINTERPOLATION_H
#define ARINTERPOLATION_H

#include "ariaTypedefs.h"
#include "ariaUtil.h"

/** 
    Store a buffer of positions (ArPose objects) with associated timestamps, can
    be queried to interpolate (or optionally extrapolate) a pose for any arbitrary
    timestamp.  

    Will work best when positions are stored frequenly and regularly.

    ArRobot maintains an ArInterpolation object which can be accessed by
    ArRobot::getPoseInterpPosition(), ArRobot::getPoseInterpNumReadings(),
    ArRobot::setPoseInterpNumReadings(), and ArRobot::getPoseInterpolation().
    Or, you could use your own ArInterpolation object; use an ArRobot
    "sensor-interpretation" task to store each robot pose received. 

    This class takes care of storing in readings of position vs time, and then
    interpolating between them to find where the robot was at a particular 
    point in time.  It has two lists, one containing the times, and one 
    containing the positions at those same times (per position), they must be 
    walked through jointly to maintain cohesion.  The new entries are at the
    front of the list, while the old ones are at the back.  
    numberOfReadings and the setNumberOfReadings control the number of entries
    in the list.  If a size is set that is smaller than the current size, then
    the old ones are chopped off.
    
    This class now has a couple of variables for when it allows
    prediction (extrapolation beyond the most recently stored pose). They're set
    with setAllowedMSForPrediction() and
    setAllowedPercentageForPrediction().  If either is below 0 than they
    are ignored (if both are below 0 it means any prediction is
    allowed, which would be bad).  Previous there was no MS limit, and
    the percentage limit was 50 (and so that is what the default is
    now).

    @ingroup UtilityClasses
**/
class ArInterpolation
{
public:
  /// Constructor
  AREXPORT ArInterpolation(size_t numberOfReadings = 100);
  /// Destructor
  AREXPORT virtual ~ArInterpolation();
  /// Adds a new reading
  AREXPORT bool addReading(ArTime timeOfReading, ArPose position);
  /// Finds a position
  AREXPORT int getPose(ArTime timeStamp, ArPose *position, 
		       ArPoseWithTime *lastData = NULL);
  /// Sets the name
  AREXPORT void setName(const char *name);
  /// Gets the name
  AREXPORT const char *getName(void);
  /// Sets the allowed milliseconds for prediction
  AREXPORT void setAllowedMSForPrediction(int ms = -1);
  /// Sets the allowed milliseconds for prediction
  AREXPORT int getAllowedMSForPrediction(void);
  /// Sets the allowed percentage for prediction
  AREXPORT void setAllowedPercentageForPrediction(int percentage = 5000);
  /// Sets the allowed milliseconds for prediction
  AREXPORT int getAllowedPercentageForPrediction(void);
  /// Sets if we should log events for this interpolation
  AREXPORT void setLogPrediction(bool logPrediction = false);
  /// Gets if we should log events for this interpolation
  AREXPORT bool getLogPrediction(void);
  /// Sets the number of readings this instance holds back in time
  AREXPORT void setNumberOfReadings(size_t numberOfReadings);
  /// Gets the number of readings this instance holds back in time
  AREXPORT size_t getNumberOfReadings(void) const;
  /// Empties the interpolated positions
  AREXPORT void reset(void);
protected:
  ArMutex myDataMutex;
  std::string myName;
  std::list<ArTime> myTimes;
  std::list<ArPose> myPoses;
  size_t mySize;
  bool myLogPrediction;
  int myAllowedMSForPrediction;
  int myAllowedPercentageForPrediction;
};

#endif
