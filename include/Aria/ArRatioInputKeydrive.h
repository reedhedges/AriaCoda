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
#ifndef ARRATIOINPUTKEYDRIVE_H
#define ARRATIOINPUTKEYDRIVE_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ArAction.h"
#include "Aria/ArFunctor.h"
#include "Aria/ArActionRatioInput.h"


/// This will use the keyboard arrow keys and the ArActionRatioInput to drive the robot
/**
   You have to make an ArActionRatioInput and add it to the robot like
   a normal action for this to work.

  @ingroup OptionalClasses
**/
class ArRatioInputKeydrive 
{
public:
  /// Constructor
  AREXPORT ArRatioInputKeydrive(ArRobot *robot, ArActionRatioInput *input, 
				int priority = 25, double velIncrement = 5);
  /// Destructor
  AREXPORT virtual ~ArRatioInputKeydrive();
  /// Takes the keys this action wants to use to drive
  AREXPORT void takeKeys();
  /// Gives up the keys this action wants to use to drive
  AREXPORT void giveUpKeys();
  /// Internal, callback for up arrow
  AREXPORT void up();
  /// Internal, callback for down arrow
  AREXPORT void down();
  /// Internal, callback for z
  AREXPORT void z();
  /// Internal, callback for x 
  AREXPORT void x();
  /// Internal, callback for left arrow
  AREXPORT void left();
  /// Internal, callback for right arrow
  AREXPORT void right();
  /// Internal, callback for space key
  AREXPORT void space();
  /// Internal, gets our firecb
  AREXPORT ArFunctor *getFireCB() { return &myFireCB; }
protected:
  AREXPORT void activate();
  AREXPORT void deactivate();
  AREXPORT void fireCallback();
  ArFunctorC<ArRatioInputKeydrive> myUpCB;
  ArFunctorC<ArRatioInputKeydrive> myDownCB;
  ArFunctorC<ArRatioInputKeydrive> myLeftCB;
  ArFunctorC<ArRatioInputKeydrive> myRightCB;
  ArFunctorC<ArRatioInputKeydrive> myZCB;
  ArFunctorC<ArRatioInputKeydrive> myXCB;
  ArFunctorC<ArRatioInputKeydrive> mySpaceCB;

  bool myPrinting;
  double myTransRatio;
  double myRotRatio;
  double myThrottle;
  double myLatRatio;

  ArRobot *myRobot;
  bool myHaveKeys;
  double myVelIncrement;
  double myLatVelIncrement;
  ArActionRatioInput *myInput;
  ArFunctorC<ArRatioInputKeydrive> myFireCB;
  ArFunctorC<ArRatioInputKeydrive> myActivateCB;
  ArFunctorC<ArRatioInputKeydrive> myDeactivateCB;
};


#endif // ARRATIOINPUTKEYDRIVE_H
