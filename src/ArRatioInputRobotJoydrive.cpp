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
#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArRatioInputRobotJoydrive.h"
#include "ArRobot.h"
#include "ArRobotJoyHandler.h"
#include "ariaInternal.h"
#include "ArCommands.h"

/**
   @param robot robot
   @param input Action to attach to and use to drive the robot. 
   @param priority Priority of this joystick input handler with respect to other input
   objects attached to the @a input action object.
   @param requireDeadmanPushed if this is true the joystick "dead man" button must be
   pushed for us to drive. If  this is false we'll follow the
   joystick input no matter what
**/

AREXPORT ArRatioInputRobotJoydrive::ArRatioInputRobotJoydrive(
	ArRobot *robot, ArActionRatioInput *input, 
	int priority, bool requireDeadmanPushed) :
  myFireCB(this, &ArRatioInputRobotJoydrive::fireCallback)
{
  myRobot = robot;
  myInput = input;
  myRequireDeadmanPushed = requireDeadmanPushed;
  myDeadZoneLast = false;
  
  if ((myRobotJoyHandler = Aria::getRobotJoyHandler()) == NULL)
  {
    myRobotJoyHandler = new ArRobotJoyHandler(robot);
    Aria::setRobotJoyHandler(myRobotJoyHandler);
  }

  myFireCB.setName("RobotJoydrive");
  myInput->addFireCallback(priority, &myFireCB);

}

AREXPORT ArRatioInputRobotJoydrive::~ArRatioInputRobotJoydrive()
{

}

AREXPORT void ArRatioInputRobotJoydrive::fireCallback(void)
{
  bool printing = false;

  bool button1 = myRobotJoyHandler->getButton1();
  // if we need the deadman to activate and it isn't pushed just bail
  if (myRequireDeadmanPushed && !button1)
  {
    if (printing)
      printf("Nothing\n");
    myDeadZoneLast = false;
    return;
  }

  double rotRatio;
  double transRatio;
  double throttleRatio;

  myRobotJoyHandler->getDoubles(&rotRatio, &transRatio, &throttleRatio);
  
  rotRatio *= 100.0;
  transRatio *= 100.0;
  throttleRatio *= 100.0;

  bool doTrans = true;
  bool doRot = true;

  
  if (!myRequireDeadmanPushed)
  {
    doTrans = ArMath::fabs(transRatio) > 33;
    doRot = ArMath::fabs(rotRatio) > 33;
  }

  if (!doTrans && !doRot)
  {
    // if the joystick is in the center, we don't need the deadman,
    // and we were stopped lasttime, then just let other stuff go
    if (myDeadZoneLast && !myRequireDeadmanPushed) 
    {
      if (printing)
	printf("deadzone Nothing\n");
      return;
    }
    // if the deadman doesn't need to be pushed let something else happen here
    if (printing)
      printf("deadzone\n");
    //myInput->setRatios(transRatio, rotRatio, throttleRatio);
    myInput->setRatios(0, 0, throttleRatio);
    myDeadZoneLast = true;
    return;
  }

  myDeadZoneLast = false;
  if (!doRot)
    rotRatio = 0;
  if (!doTrans)
    transRatio = 0;

  if (printing)
    printf("%.0f %.0f %.0f\n", transRatio, rotRatio, throttleRatio);
  
  if (printing)
    printf("(%ld ms ago) we got %d %d %.2f %.2f %.2f (speed %.0f %.0f)\n", 
	   myRobotJoyHandler->getDataReceivedTime().mSecSince(),
	   button1, myRobotJoyHandler->getButton2(), transRatio, rotRatio, 
	   throttleRatio, myRobot->getVel(), myRobot->getRotVel());

  myInput->setRatios(transRatio, rotRatio, throttleRatio);
}
