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
#ifndef ARSONARAUTODISABLER_H
#define ARSONARAUTODISABLER_H

/// Class for automatically disabling sonar when the robot is stopped
/**
   If you create one of this class it will disable the sonar when the
   robot stops moving and then enable the sonar when the robot moves.
   Later this may get more parameters and the ability to be turned on
   and off and things like that (email on aria-users if you want
   them).

   Note that this class assumes it is the only class turning the sonar
   on or off and that the sonar start on.

    @ingroup OptionalClasses
 **/

#include "ariaTypedefs.h"
#include "ariaUtil.h"
#include "ArFunctor.h"

class ArRobot;

class ArSonarAutoDisabler
{
public:
  /// Constructor
  AREXPORT ArSonarAutoDisabler(ArRobot *robot);
  /// Destructor
  AREXPORT virtual ~ArSonarAutoDisabler();
  /// Supresses this disabler (which turns off the sonar)
  void supress(void) 
    { ArLog::log(ArLog::Normal, "ArSonarAutoDisabler::supress:"); 
      mySupressed = true; }
  /// Gets the callback to supress the autodisabler
  ArFunctor *getSupressCallback(void) { return &mySupressCB; }
  /// Unsupresses this disabler (goes back to auto enabling/disabling)
  void unsupress(void) 
    { ArLog::log(ArLog::Normal, "ArSonarAutoDisabler::unsupress:"); 
      mySupressed = false; }
  /// Gets the callback to supress the autodisabler
  ArFunctor *getUnsupressCallback(void) { return &myUnsupressCB; }

  /// Sets that we're autonomous drivign so we only enable some sonar
  void setAutonomousDriving(void) 
    { ArLog::log(ArLog::Normal, "ArSonarAutoDisabler::setAutonomousDriving:"); 
      myAutonomousDriving = true; }
  /// Gets the callback to set that we're driving autonomously
  ArFunctor *getSetAutonomousDrivingCallback(void) 
    { return &mySetAutonomousDrivingCB; }
  /// Sets that we're driving non-autonomously so we enable all sonar
  void clearAutonomousDriving(void) 
    { ArLog::log(ArLog::Normal, "ArSonarAutoDisabler::clearAutonomousDriving:"); 
      myAutonomousDriving = false; }
  /// Gets the callback to set that we're not driving autonomously
  ArFunctor *getClearAutonomousDrivingCallback(void) 
    { return &myClearAutonomousDrivingCB; }
protected:
  /// our user task
  AREXPORT void userTask(void);
  ArRobot *myRobot;
  ArTime myLastMoved;
  ArTime myLastSupressed;
  bool mySupressed;
  bool myAutonomousDriving;

  ArFunctorC<ArSonarAutoDisabler> myUserTaskCB;
  ArFunctorC<ArSonarAutoDisabler> mySupressCB;
  ArFunctorC<ArSonarAutoDisabler> myUnsupressCB;
  ArFunctorC<ArSonarAutoDisabler> mySetAutonomousDrivingCB;
  ArFunctorC<ArSonarAutoDisabler> myClearAutonomousDrivingCB;
};

#endif // ARSONARAUTODISABLER
