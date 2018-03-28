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
#include "Aria.h"

/** @example actionGroupExample.cpp Example of using ArActionGroup objects to switch
 *         between two different kinds of behavior.
 *
 *  This program creates two action groups, a teleoperation
 *  group and a wander group. Each group contains actions
 *  that together effect the desired behavior: in teleoperation
 *  mode, input actions allow the robot to be driven by keyboard
 *  or joystick, and higher-priority limiter actions help avoid obstacles.
 *  In wander mode, a constant-velocity action drives the robot
 *  forward, but higher-priority avoidance actions make the robot
 *  turn away from obstacles, or back up if an obstacle is hit or
 *  the motors stall.  Keyboard commands (the T and W keys) are
 *  used to switch between the two modes, by activating the 
 *  action group for the chosen mode.
 *
 *  @see ArActionGroup
 *  @see @ref actions overview
 *  @see actionExample.cpp
 */

ArActionGroup *teleop;
ArActionGroup *wander;

// Activate the teleop action group. activateExlcusive() causes
// all other active action groups to be deactivated.
void teleopMode(void)
{
  teleop->activateExclusive();
  printf("\n== Teleoperation Mode ==\n");
  printf("   Use the arrow keys to drive, and the spacebar to stop.\n    For joystick control hold the trigger button.\n    Press 'w' to switch to wander mode.\n    Press escape to exit.\n");
}

// Activate the wander action group. activateExlcusive() causes
// all other active action groups to be deactivated.
void wanderMode(void)
{
  wander->activateExclusive();
  printf("\n== Wander Mode ==\n");
  printf("    The robot will now just wander around avoiding things.\n    Press 't' to switch to  teleop mode.\n    Press escape to exit.\n");
}


int main(int argc, char** argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;
  ArSonarDevice sonar;

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  ArRobotConnector robotConnector(&parser, &robot);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "actionGroupExample: Could not connect to the robot.");
    if(parser.checkHelpAndWarnUnparsed())
    {
        // -help not given
        Aria::logOptions();
        Aria::exit(1);
    }
  }

  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(1);
  }

  ArLog::log(ArLog::Normal, "actionGroupExample: Connected to robot.");

  /* - the action group for teleoperation actions: */
  teleop = new ArActionGroup(&robot);

  // don't hit any tables (if the robot has IR table sensors)
  teleop->addAction(new ArActionLimiterTableSensor, 100);

  // limiter for close obstacles
  teleop->addAction(new ArActionLimiterForwards("speed limiter near", 
						300, 600, 250), 95);

  // limiter for far away obstacles
  teleop->addAction(new ArActionLimiterForwards("speed limiter far", 
					       300, 1100, 400), 90);

  // limiter so we don't bump things backwards
  teleop->addAction(new ArActionLimiterBackwards, 85);

  // the joydrive action (drive from joystick)
  ArActionJoydrive joydriveAct("joydrive", 400, 15);
  teleop->addAction(&joydriveAct, 50);

  // the keydrive action (drive from keyboard)
  teleop->addAction(new ArActionKeydrive, 45);
  


  /* - the action group for wander actions: */
  wander = new ArActionGroup(&robot);

  // if we're stalled we want to back up and recover
  wander->addAction(new ArActionStallRecover, 100);

  // react to bumpers
  wander->addAction(new ArActionBumpers, 75);

  // turn to avoid things closer to us
  wander->addAction(new ArActionAvoidFront("Avoid Front Near", 225, 0), 50);

  // turn avoid things further away
  wander->addAction(new ArActionAvoidFront, 45);

  // keep moving
  wander->addAction(new ArActionConstantVelocity("Constant Velocity", 400), 25);

  

  /* - use key commands to switch modes, and use keyboard
   *   and joystick as inputs for teleoperation actions. */

  // create key handler if Aria does not already have one
  ArKeyHandler *keyHandler = Aria::getKeyHandler();
  if (keyHandler == NULL)
  {
    keyHandler = new ArKeyHandler;
    Aria::setKeyHandler(keyHandler);
    robot.attachKeyHandler(keyHandler);
  }

  // set the callbacks
  ArGlobalFunctor teleopCB(&teleopMode);
  ArGlobalFunctor wanderCB(&wanderMode);
  keyHandler->addKeyHandler('w', &wanderCB);
  keyHandler->addKeyHandler('W', &wanderCB);
  keyHandler->addKeyHandler('t', &teleopCB);
  keyHandler->addKeyHandler('T', &teleopCB);

  // if we don't have a joystick, let 'em know
  if (!joydriveAct.joystickInited())
    printf("Note: Do not have a joystick, only the arrow keys on the keyboard will work.\n");
  
  // set the joystick so it won't do anything if the button isn't pressed
  joydriveAct.setStopIfNoButtonPressed(false);


  /* - connect to the robot, then enter teleoperation mode.  */

  robot.addRangeDevice(&sonar);

  robot.enableMotors();
  teleopMode();
  robot.run(true);

  Aria::exit(0);
}
