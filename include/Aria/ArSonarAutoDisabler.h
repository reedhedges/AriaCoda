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

#include "Aria/ariaTypedefs.h"
#include "Aria/ariaUtil.h"
#include "Aria/ArFunctor.h"

class ArRobot;

class ArSonarAutoDisabler
{
public:
  /// Constructor
  AREXPORT explicit ArSonarAutoDisabler(ArRobot *robot);
  /// Destructor
  AREXPORT  ~ArSonarAutoDisabler();
  /// Suppresses this disabler (which turns off the sonar)
  void supress() 
    { ArLog::log(ArLog::Normal, "ArSonarAutoDisabler::suppress:"); 
      mySupressed = true; }
  void suppress() { supress(); }
  /// Gets the callback to suppress the autodisabler
  ArFunctor *getSupressCallback() { return &mySupressCB; }
  ArFunctor *getSuppressCallback() { return &mySupressCB; }
  /// Unsuppresses this disabler (goes back to auto enabling/disabling)
  void unsupress() 
    { ArLog::log(ArLog::Normal, "ArSonarAutoDisabler::unsuppress:"); 
      mySupressed = false; }
  void unsuppress() { unsupress(); }
  /// Gets the callback to supress the autodisabler
  ArFunctor *getUnsupressCallback() { return &myUnsupressCB; }
  ArFunctor *getUnsuppressCallback() { return &myUnsupressCB; }
  /// Sets that we're autonomous drivign so we only enable some sonar
  void setAutonomousDriving() 
  { ArLog::log(ArLog::Normal, "ArSonarAutoDisabler::setAutonomousDriving:"); 
    myAutonomousDriving = true; }
  /// Gets the callback to set that we're driving autonomously
  ArFunctor *getSetAutonomousDrivingCallback() 
    { return &mySetAutonomousDrivingCB; }
  /// Sets that we're driving non-autonomously so we enable all sonar
  void clearAutonomousDriving() 
    { ArLog::log(ArLog::Normal, "ArSonarAutoDisabler::clearAutonomousDriving:"); 
      myAutonomousDriving = false; }
  /// Gets the callback to set that we're not driving autonomously
  ArFunctor *getClearAutonomousDrivingCallback() 
    { return &myClearAutonomousDrivingCB; }
protected:
  /// our user task
  void userTask();
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
