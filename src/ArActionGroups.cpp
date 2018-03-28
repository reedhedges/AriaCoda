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
#include "ArRobot.h"
#include "ArActionGroup.h"
#include "ArActionGroups.h"
#include "ArActionLimiterTableSensor.h"
#include "ArActionLimiterForwards.h"
#include "ArActionLimiterBackwards.h"
#include "ArActionInput.h"
#include "ArActionStop.h"
#include "ArActionStallRecover.h"
#include "ArActionBumpers.h"
#include "ArActionAvoidFront.h"
#include "ArActionConstantVelocity.h"
#include "ArActionJoydrive.h"
#include "ArActionKeydrive.h"
#include "ArActionDeceleratingLimiter.h"
#include "ArActionRatioInput.h"
#include "ArRatioInputKeydrive.h"
#include "ArRatioInputJoydrive.h"
#include "ArRatioInputRobotJoydrive.h"

AREXPORT ArActionGroupInput::ArActionGroupInput(ArRobot *robot)
  : ArActionGroup(robot)
{
  addAction(new ArActionLimiterTableSensor, 100);
  addAction(new ArActionLimiterForwards("Speed Limiter Near", 
                                                      300, 600, 250),
                          90);
  addAction(new ArActionLimiterForwards("Speed Limiter Far",
                                                      300, 1100, 400),
                          89);
  addAction(new ArActionLimiterBackwards, 80);
  myInput = new ArActionInput;
  addAction(myInput, 70);
}

AREXPORT ArActionGroupInput::~ArActionGroupInput()
{
  removeActions();
  deleteActions();
}

AREXPORT void ArActionGroupInput::setVel(double vel)
{
  myInput->setVel(vel);
}

AREXPORT void ArActionGroupInput::setRotVel(double rotVel)
{
  myInput->setRotVel(rotVel);
}

AREXPORT void ArActionGroupInput::deltaHeadingFromCurrent(double delta)
{
  myInput->deltaHeadingFromCurrent(delta);
}

AREXPORT void ArActionGroupInput::setHeading(double heading)
{
  myInput->setHeading(heading);
}

AREXPORT void ArActionGroupInput::clear(void)
{
  myInput->clear();
}

AREXPORT ArActionInput *ArActionGroupInput::getActionInput(void)
{
  return myInput;
}

AREXPORT ArActionGroupStop::ArActionGroupStop(ArRobot *robot)
  : ArActionGroup(robot)
{
  myActionStop = new ArActionStop;
  addAction(myActionStop, 100);
}

AREXPORT ArActionGroupStop::~ArActionGroupStop()
{
  removeActions();
  deleteActions();
}

AREXPORT ArActionStop *ArActionGroupStop::getActionStop(void)
{
  return myActionStop;
}

AREXPORT ArActionGroupTeleop::ArActionGroupTeleop(ArRobot *robot)
  : ArActionGroup(robot)
{
  addAction(new ArActionLimiterTableSensor, 100);
  addAction(new ArActionLimiterForwards("Speed Limiter Near", 
                                                      300, 600, 250),
                          90);
  addAction(new ArActionLimiterForwards("Speed Limiter Far",
                                                      300, 1100, 400),
                          89);
  addAction(new ArActionLimiterBackwards, 80);
  myJoydrive = new ArActionJoydrive;
  myJoydrive->setStopIfNoButtonPressed(false);
  addAction(myJoydrive, 70);
  addAction(new ArActionKeydrive, 69);
}

AREXPORT ArActionGroupTeleop::~ArActionGroupTeleop()
{
  removeActions();
  deleteActions();
}

AREXPORT void ArActionGroupTeleop::setThrottleParams(int lowSpeed, 
						     int highSpeed)
{
  myJoydrive->setThrottleParams(lowSpeed, highSpeed);
}

AREXPORT ArActionGroupUnguardedTeleop::ArActionGroupUnguardedTeleop(ArRobot *robot)
  : ArActionGroup(robot)
{
  myJoydrive = new ArActionJoydrive;
  myJoydrive->setStopIfNoButtonPressed(false);
  addAction(myJoydrive, 70);
  addAction(new ArActionKeydrive, 69);
}

AREXPORT ArActionGroupUnguardedTeleop::~ArActionGroupUnguardedTeleop()
{
  removeActions();
  deleteActions();
}

AREXPORT void ArActionGroupUnguardedTeleop::setThrottleParams(int lowSpeed, 
							 int highSpeed)
{
  myJoydrive->setThrottleParams(lowSpeed, highSpeed);
}

AREXPORT ArActionGroupWander::ArActionGroupWander(ArRobot *robot, int forwardVel, int avoidFrontDist, int avoidVel, int avoidTurnAmt)
  : ArActionGroup(robot)
{
  addAction(new ArActionBumpers, 100);
  addAction(new ArActionStallRecover, 90);
  //addAction(new ArActionAvoidFront("Avoid Front Near", 250, 0), 80);
  addAction(new ArActionAvoidFront("Avoid Front", avoidFrontDist, avoidVel, avoidTurnAmt), 79);
  addAction(new ArActionConstantVelocity("Constant Velocity",
                                         forwardVel),
					 50);

}

AREXPORT ArActionGroupWander::~ArActionGroupWander()
{
  removeActions();
  deleteActions();
}

/*
// The color follow action group
AREXPORT ArActionGroupColorFollow::ArActionGroupColorFollow(ArRobot *robot, ArACTS_1_2 *acts, ArPTZ *camera)
  : ArActionGroup(robot)
{
  // Add the limiters so the robot is less likely to run into things
  addAction(new ArActionLimiterTableSensor, 100);
  addAction(new ArActionLimiterForwards("Speed Limiter Near", 
                                                      300, 600, 250),
                          90);
  addAction(new ArActionLimiterForwards("Speed Limiter Far",
                                                      300, 1100, 400),
                          89);
  addAction(new ArActionLimiterBackwards, 80);

  // Construct the color follower and add it
  myColorFollow = new ArActionColorFollow("Follow a color.", acts, camera);
  addAction(myColorFollow, 70);
}

// Destructor
AREXPORT ArActionGroupColorFollow::~ArActionGroupColorFollow()
{
  removeActions();
  deleteActions();
}

// Set the channel to get blob info from
AREXPORT void ArActionGroupColorFollow::setChannel(int channel)
{
  myColorFollow->setChannel(channel);
}

// Set the camera to control
AREXPORT void ArActionGroupColorFollow::setCamera(ArPTZ *camera)
{
  myColorFollow->setCamera(camera);
}

// Allow the robot to move
AREXPORT void ArActionGroupColorFollow::startMovement()
{
  myColorFollow->startMovement();
}

// Keep the robot from moving
AREXPORT void ArActionGroupColorFollow::stopMovement()
{
  myColorFollow->stopMovement();
}

// Toggle whether or not the robot will try to actively
// acquire a color blob
AREXPORT void ArActionGroupColorFollow::setAcquire(bool acquire)
{
  myColorFollow->setAcquire(acquire);
}

// Return the channel that the robot is looking on
AREXPORT int ArActionGroupColorFollow::getChannel()
{
  return myColorFollow->getChannel();
}

// Return whether the robot is allowed to actively
// acquire a color blob
AREXPORT bool ArActionGroupColorFollow::getAcquire()
{
  return myColorFollow->getAcquire();
}

// Return whether the robot is allowed to move
AREXPORT bool ArActionGroupColorFollow::getMovement()
{
  return myColorFollow->getMovement();
}

// Return if the robot is targeting a color blob
AREXPORT bool ArActionGroupColorFollow::getBlob()
{
  return myColorFollow->getBlob();
}
*/

AREXPORT ArActionGroupRatioDrive::ArActionGroupRatioDrive(ArRobot *robot)
  : ArActionGroup(robot)
{
  // add the actions, put the ratio input on top, then have the
  // limiters since the ratio doesn't touch decel except lightly
  // whereas the limiter will touch it strongly

  myInput = new ArActionRatioInput;
  addAction(myInput, 50); 

  myKeydrive = new ArRatioInputKeydrive(robot, myInput);
  myJoydrive = new ArRatioInputJoydrive(robot, myInput);
  myRobotJoydrive = new ArRatioInputRobotJoydrive(robot, myInput);

  myDeceleratingLimiterForward = new ArActionDeceleratingLimiter(
	  "DeceleratingLimiterForward", ArActionDeceleratingLimiter::FORWARDS);
  addAction(myDeceleratingLimiterForward, 40);

  myDeceleratingLimiterBackward = new ArActionDeceleratingLimiter(
	  "DeceleratingLimiterBackward", 
	  ArActionDeceleratingLimiter::BACKWARDS);
  addAction(myDeceleratingLimiterBackward, 39);

  myDeceleratingLimiterLateralLeft = NULL;
  myDeceleratingLimiterLateralRight = NULL;
  if (myRobot->hasLatVel())
  {
    myDeceleratingLimiterLateralLeft = new ArActionDeceleratingLimiter(
	    "DeceleratingLimiterLateral", 
	    ArActionDeceleratingLimiter::LATERAL_LEFT);
    addAction(myDeceleratingLimiterLateralLeft, 38);
    myDeceleratingLimiterLateralRight = new ArActionDeceleratingLimiter(
	    "DeceleratingLimiterLateralRight", 
	    ArActionDeceleratingLimiter::LATERAL_RIGHT);
    addAction(myDeceleratingLimiterLateralRight, 37);
  }

}

AREXPORT ArActionGroupRatioDrive::~ArActionGroupRatioDrive()
{
  removeActions();
  deleteActions();
}


AREXPORT ArActionRatioInput *ArActionGroupRatioDrive::getActionRatioInput(void)
{
  return myInput;
}

AREXPORT void ArActionGroupRatioDrive::addToConfig(ArConfig *config, 
						 const char *section)
{
  myInput->addToConfig(config, section);
  myDeceleratingLimiterForward->addToConfig(config, section, "Forward");
  myDeceleratingLimiterBackward->addToConfig(config, section, "Backward");
}

AREXPORT ArActionGroupRatioDriveUnsafe::ArActionGroupRatioDriveUnsafe(ArRobot *robot)
  : ArActionGroup(robot)
{
  // add the actions, put the ratio input on top, then have the
  // limiters since the ratio doesn't touch decel except lightly
  // whereas the limiter will touch it strongly

  myInput = new ArActionRatioInput;
  addAction(myInput, 50); 

  myKeydrive = new ArRatioInputKeydrive(robot, myInput);
  myJoydrive = new ArRatioInputJoydrive(robot, myInput);
  myRobotJoydrive = new ArRatioInputRobotJoydrive(robot, myInput);
}

AREXPORT ArActionGroupRatioDriveUnsafe::~ArActionGroupRatioDriveUnsafe()
{
  removeActions();
  deleteActions();  
}


AREXPORT ArActionRatioInput *ArActionGroupRatioDriveUnsafe::getActionRatioInput(void)
{
  return myInput;
}

AREXPORT void ArActionGroupRatioDriveUnsafe::addToConfig(ArConfig *config, 
						 const char *section)
{
  myInput->addToConfig(config, section);
}
