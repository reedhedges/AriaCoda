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
#include "ArRatioInputKeydrive.h"
#include "ArRobot.h"
#include "ariaInternal.h"
#include "ArKeyHandler.h"

AREXPORT ArRatioInputKeydrive::ArRatioInputKeydrive(ArRobot *robot, 
						    ArActionRatioInput *input,
						    int priority,
						    double velIncrement) :
  myUpCB(this, &ArRatioInputKeydrive::up),
  myDownCB(this, &ArRatioInputKeydrive::down),
  myLeftCB(this, &ArRatioInputKeydrive::left),
  myRightCB(this, &ArRatioInputKeydrive::right),
  myZCB(this, &ArRatioInputKeydrive::z),
  myXCB(this, &ArRatioInputKeydrive::x),
  mySpaceCB(this, &ArRatioInputKeydrive::space),
  myFireCB(this, &ArRatioInputKeydrive::fireCallback),
  myActivateCB(this, &ArRatioInputKeydrive::activate),
  myDeactivateCB(this, &ArRatioInputKeydrive::deactivate)
{
  myRobot = robot;
  myInput = input;
  myInput->addFireCallback(priority, &myFireCB);
  myInput->addActivateCallback(&myActivateCB);
  myInput->addDeactivateCallback(&myDeactivateCB);
  myFireCB.setName("Keydrive");
  myVelIncrement = velIncrement;
  myLatVelIncrement = velIncrement;
  myHaveKeys = false;
  myTransRatio = 0;
  myRotRatio = 0;
  myLatRatio = 0;
  myThrottle = 100;
  myPrinting = false;
}

AREXPORT ArRatioInputKeydrive::~ArRatioInputKeydrive()
{
  myInput->remFireCallback(&myFireCB);
  myInput->remActivateCallback(&myActivateCB);
}


AREXPORT void ArRatioInputKeydrive::takeKeys(void)
{
  myHaveKeys = true;
  ArKeyHandler *keyHandler;
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    ArLog::log(ArLog::Terse, 
	       "ArRatioInputKeydrive::takeKeys: There is no key handler, keydrive will not work.");
  }
  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (!keyHandler->addKeyHandler(ArKeyHandler::UP, &myUpCB))
    ArLog::log(ArLog::Terse, "The key handler already has a key for up, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::DOWN, &myDownCB))
    ArLog::log(ArLog::Terse, "The key handler already has a key for down, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::LEFT, &myLeftCB))
    ArLog::log(ArLog::Terse,  
	       "The key handler already has a key for left, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::RIGHT, &myRightCB))
    ArLog::log(ArLog::Terse,  
	       "The key handler already has a key for right, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::SPACE, &mySpaceCB))
    ArLog::log(ArLog::Terse,  
	       "The key handler already has a key for space, keydrive will not work correctly.");
  if (myRobot != NULL && myRobot->hasLatVel())
  {
    if (!keyHandler->addKeyHandler('z', &myZCB))
      ArLog::log(ArLog::Terse,  
		 "The key handler already has a key for z, keydrive will not work correctly.");
    if (!keyHandler->addKeyHandler('Z', &myZCB))
      ArLog::log(ArLog::Terse,  
		 "The key handler already has a key for Z, keydrive will not work correctly.");
    if (!keyHandler->addKeyHandler('x', &myXCB))
      ArLog::log(ArLog::Terse,  
		 "The key handler already has a key for x, keydrive will not work correctly.");
    if (!keyHandler->addKeyHandler('X', &myXCB))
      ArLog::log(ArLog::Terse,  
		 "The key handler already has a key for x, keydrive will not work correctly.");
  }
}

AREXPORT void ArRatioInputKeydrive::giveUpKeys(void)
{
  ArKeyHandler *keyHandler;
  myHaveKeys = false;
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    ArLog::log(ArLog::Terse, 
	       "ArRatioInputKeydrive::giveUpKeys: There is no key handler, something is probably horribly wrong .");
  }
  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (!keyHandler->remKeyHandler(&myUpCB))
    ArLog::log(ArLog::Terse, "ArRatioInputKeydrive: The key handler already didn't have a key for up, something is wrong.");
  if (!keyHandler->remKeyHandler(&myDownCB))
    ArLog::log(ArLog::Terse, "ArRatioInputKeydrive: The key handler already didn't have a key for down, something is wrong.");
  if (!keyHandler->remKeyHandler(&myLeftCB))
    ArLog::log(ArLog::Terse,  
	       "ArRatioInputKeydrive: The key handler already didn't have a key for left, something is wrong.");
  if (!keyHandler->remKeyHandler(&myRightCB))
    ArLog::log(ArLog::Terse,  
	       "ArRatioInputKeydrive: The key handler already didn't have a key for right, something is wrong.");
  if (!keyHandler->remKeyHandler(&mySpaceCB))
    ArLog::log(ArLog::Terse,  
	       "ArRatioInputKeydrive: The key handler didn't have a key for space, something is wrong.");
  if (myRobot != NULL && myRobot->hasLatVel())
  {
    if (!keyHandler->remKeyHandler(&myZCB))
      ArLog::log(ArLog::Terse,  
		 "ArRatioInputKeydrive: The key handler didn't have a key for z, something is wrong.");
    if (!keyHandler->remKeyHandler(&myXCB))
      ArLog::log(ArLog::Terse,  
		 "ArRatioInputKeydrive: The key handler didn't have a key for x, something is wrong.");
  }

}


AREXPORT void ArRatioInputKeydrive::up(void)
{
  if (myPrinting)
    printf("up\n");
  myTransRatio += myVelIncrement;
  if (myTransRatio > 100)
    myTransRatio = 100;
}

AREXPORT void ArRatioInputKeydrive::down(void)
{
  if (myPrinting)
    printf("down\n");
  myTransRatio -= myVelIncrement;
  if (myTransRatio < -100)
    myTransRatio = -100;
}

AREXPORT void ArRatioInputKeydrive::left(void)
{
  if (myPrinting)
    printf("left\n");
  myRotRatio = 100;
}

AREXPORT void ArRatioInputKeydrive::right(void)
{
  if (myPrinting)
    printf("right\n");
  myRotRatio = -100;
}

AREXPORT void ArRatioInputKeydrive::space(void)
{
  if (myPrinting)
    printf("stop\n");
  myTransRatio = 0;
  myRotRatio = 0;
  myLatRatio = 0;
}

AREXPORT void ArRatioInputKeydrive::z(void)
{
  if (myPrinting)
    printf("up\n");
  myLatRatio += myLatVelIncrement;
  if (myLatRatio > 100)
    myLatRatio = 100;
}

AREXPORT void ArRatioInputKeydrive::x(void)
{
  if (myPrinting)
    printf("down\n");
  myLatRatio -= myLatVelIncrement;
  if (myLatRatio < -100)
    myLatRatio = -100;
}

AREXPORT void ArRatioInputKeydrive::activate(void)
{
  // set things so we'll stop
  myTransRatio = 0;
  myRotRatio = 0;
  myLatRatio = 0;
  if (myHaveKeys)
    takeKeys();
}

AREXPORT void ArRatioInputKeydrive::deactivate(void)
{
  if (myHaveKeys)
    giveUpKeys();
}

AREXPORT void ArRatioInputKeydrive::fireCallback(void)
{
  // set what we want to do
  myInput->setTransRatio(myTransRatio);
  myInput->setRotRatio(myRotRatio);
  myInput->setLatRatio(myLatRatio);
  myInput->setThrottleRatio(myThrottle);

  // reset us to going straight (if they're holding the key we'll keep turning)
  myRotRatio = 0;

  if (myHaveKeys)
    return;
  ArKeyHandler *keyHandler;
   
  // see if there is already a keyhandler, if not make one for ourselves
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    keyHandler = new ArKeyHandler;
    Aria::setKeyHandler(keyHandler);
    myRobot->attachKeyHandler(keyHandler);
  }
  takeKeys();
}
