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
#ifndef ARLASERFILTER_H
#define ARLASERFILTER_H

#include "ArLaser.h"
#include "ArFunctor.h"

class ArRobot;
class ArConfig;

/// Range device with data obtained from another range device and filtered 
/**
   This is a class for generically filtering laser readings, either to
   separate out ones that are too far or too close compared to their
   neighbors.  The filtering parameters can be adjusted on line
   through ArConfig options.

   This implements ArLaser and so can be used like any other laser,
   though you have to set all its options before you create this and
   probably should connect it too.  Then you should replace the
   original laser on ArRobot with this one, and replace the original
   laser as a range device too.
**/
class ArLaserFilter : public ArLaser
{
public:
  /// Constructor
  AREXPORT ArLaserFilter(ArLaser *laser, const char *name = NULL);
  /// Destructor
  AREXPORT ~ArLaserFilter();
  /// Set robot
  AREXPORT virtual void setRobot(ArRobot *robot);
  /// Add to the config
  AREXPORT void addToConfig(ArConfig *config, const char *sectionName,
			    const char *prefix = "");

  AREXPORT virtual bool blockingConnect(void) 
    { return myLaser->blockingConnect(); }
  AREXPORT virtual bool asyncConnect(void)
    { return myLaser->asyncConnect(); }
  AREXPORT virtual bool disconnect(void)
    { return myLaser->disconnect(); }
  AREXPORT virtual bool isConnected(void)
    { return myLaser->isConnected(); }
  AREXPORT virtual bool isTryingToConnect(void)
    { return myLaser->isTryingToConnect(); }

  AREXPORT virtual void *runThread(void *arg) { return NULL; } 
  AREXPORT virtual bool laserCheckParams(void) 
    { 
      if (!myLaser->laserCheckParams())
	return false;
      
      laserSetAbsoluteMaxRange(myLaser->getAbsoluteMaxRange());
      return true; 
    }
  
  /// Gets the base laser this is filtering
  ArLaser *getBaseLaser(void) { return myLaser; }
protected:
  AREXPORT int selfLockDevice(void);
  AREXPORT int selfTryLockDevice(void);
  AREXPORT int selfUnlockDevice(void);

  ArLaser *myLaser;

  // Parameters
  double myAngleToCheck;
  double myAllFactor;
  double myAnyFactor;
  double myAnyMinRange;
  double myAnyMinRangeLessThanAngle;
  double myAnyMinRangeGreaterThanAngle;

  /// Does the check against all neighbor factor
  bool checkRanges(int thisReading, int otherReading, double factor);
  
  // Callback to do the actual filtering
  void processReadings(void);

  ArFunctorC<ArLaserFilter> myProcessCB;
};

#endif // ARLASERFILTER
