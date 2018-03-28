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
#ifndef ARACTIONLIMITERROT_H
#define ARACTIONLIMITERROT_H

#include "ariaTypedefs.h"
#include "ArAction.h"

/// Action to limit the forwards motion of the robot based on range sensor readings
/**
   This action uses the robot's range sensors (e.g. sonar, laser) to find a 
   maximum speed at which to travel
   and will increase the deceleration so that the robot doesn't hit
   anything.  If it has to, it will trigger an estop to avoid a
   collision.

   Note that this cranks up the deceleration with a strong strength,
   but it checks to see if there is already something decelerating
   more strongly... so you can put these actions lower in the priority list so
   things will play together nicely.

   @ingroup ActionClasses
**/
class ArActionLimiterRot : public ArAction
{
public:
  /// Constructor
  AREXPORT ArActionLimiterRot(const char *name = "limitRot");
  /// Destructor
  AREXPORT virtual ~ArActionLimiterRot();
  AREXPORT virtual ArActionDesired *fire(ArActionDesired currentDesired);
  AREXPORT virtual ArActionDesired *getDesired(void) { return &myDesired; }
#ifndef SWIG
  AREXPORT virtual const ArActionDesired *getDesired(void) const 
                                                        { return &myDesired; }
#endif
  /// Sets the parameters (don't use this if you're using the addToConfig)
  AREXPORT void setParameters(bool checkRadius = false,
			      double inRadiusSpeed = 0);
  /// Adds to the ArConfig given, in section, with prefix
  AREXPORT void addToConfig(ArConfig *config, const char *section,
			    const char *prefix = NULL);
  /// Sets if we're using locationDependent range devices or not
  bool getUseLocationDependentDevices(void) 
    { return myUseLocationDependentDevices; }
  /// Sets if we're using locationDependent range devices or not
  void setUseLocationDependentDevices(bool useLocationDependentDevices)
    { myUseLocationDependentDevices = useLocationDependentDevices; }
protected:
  bool myCheckRadius;
  double myInRadiusSpeed;
  bool myUseLocationDependentDevices;
  ArActionDesired myDesired;
};

#endif // ARACTIONSPEEDLIMITER_H
