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
#ifndef ARRATIOINPUTROBOTJOYDRIVE_H
#define ARRATIOINPUTROBOTJOYDRIVE_H

#include "ariaTypedefs.h"
#include "ArActionRatioInput.h"

class ArRobotPacket;
class ArRobot;
class ArRobotJoyHandler;

/// Use robot's joystick to control an ArActionRatioInput action and drive the robot.
/**
   This class connects the joystick data obtained from  the robot's built
   in joystick port (if it has one, not all robots have a joystick port)
   to an ArActionRatioInput which drives the robot. (See ArRatioInputJoydrive
   for a similar class that uses a joystick plugged in to the computer.)

   A callback is attached to the ArActionRatioInput object which reads joystick
   information using an ArRobotJoyHandler object, and sets requested drive rations on the ArActionRatioInput
   object.


    @sa ArRatioInputJoydrive
    @sa ArActionRatioInput


  @ingroup OptionalClasses
**/
class ArRatioInputRobotJoydrive 
{
public:
  /// Constructor
  AREXPORT ArRatioInputRobotJoydrive(ArRobot *robot, 
				     ArActionRatioInput *input,
				     int priority = 75,
				     bool requireDeadmanPushed = true);
  /// Destructor
  AREXPORT virtual ~ArRatioInputRobotJoydrive();
protected:
  AREXPORT void fireCallback(void);

  ArRobot *myRobot;
  ArActionRatioInput *myInput;
  bool myRequireDeadmanPushed;
  bool myDeadZoneLast;

  ArRobotJoyHandler *myRobotJoyHandler;
  ArFunctorC<ArRatioInputRobotJoydrive> myFireCB;
};

#endif //ARRATIOINPUTROBOTJOYDRIVE_H
