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

/** @example teleopActionsExample.cpp  Example using actions for safe 
 * teleoperation with keyboard or joystick.
  
  This example program creates several "limiting" actions (stop the robot
  from hitting detected obstacles), as well as Joydrive and Keydrive
  actions which request movement based on keys pressed or a joystick
  attached to the computer. The limiting actions are added at a higher 
  priority than the teleoperation actions, so they prevent those actions
  from driving the robot if nearby obstacles are detected; otherwise,
  you can drive the robot using they joystick or keyboard.
*/

int main(int argc, char **argv)
{
    Aria::init();
    ArArgumentParser parser(&argc, argv);
    parser.loadDefaultArguments();
    ArRobot robot;
    ArRobotConnector robotConnector(&parser, &robot);
    if(!robotConnector.connectRobot())
    {
      ArLog::log(ArLog::Terse, "teleopActionsExample: Could not connect to the robot.");
      if(parser.checkHelpAndWarnUnparsed())
      {
          Aria::logOptions();
          Aria::exit(1);
      }
    }
    if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
    {
      Aria::logOptions();
      Aria::exit(1);
    }
    ArLog::log(ArLog::Normal, "teleopActionsExample: Connected.");

  
  // limiter for close obstacles
  ArActionLimiterForwards limiter("speed limiter near", 300, 600, 250);
  // limiter for far away obstacles
  ArActionLimiterForwards limiterFar("speed limiter far", 300, 1100, 400);
  // limiter that checks IR sensors (like Peoplebot has)
  ArActionLimiterTableSensor tableLimiter;
  // limiter so we don't bump things backwards
  ArActionLimiterBackwards backwardsLimiter;
  // the joydrive action
  ArActionJoydrive joydriveAct;
  // the keydrive action
  ArActionKeydrive keydriveAct;
  
  // sonar device, used by the limiter actions.
  ArSonarDevice sonar;



  printf("This program will allow you to use a joystick or keyboard to control the robot.\nYou can use the arrow keys to drive, and the spacebar to stop.\nFor joystick control press the trigger button and then drive.\nPress escape to exit.\n");

  // if we don't have a joystick, let 'em know
  if (!joydriveAct.joystickInited())
    printf("Do not have a joystick, only the arrow keys on the keyboard will work.\n");
  
  // add the sonar to the robot
  robot.addRangeDevice(&sonar);


  // set the robots maximum velocity (sonar don't work at all well if you're
  // going faster)
  robot.setAbsoluteMaxTransVel(400);

  // enable the motor
  robot.enableMotors();

  // Add the actions, with the limiters as highest priority, then the teleop.
  // actions.  This will keep the teleop. actions from being able to drive too 
  // fast and hit something
  robot.addAction(&tableLimiter, 100);
  robot.addAction(&limiter, 95);
  robot.addAction(&limiterFar, 90);
  robot.addAction(&backwardsLimiter, 85);
  robot.addAction(&joydriveAct, 50);
  robot.addAction(&keydriveAct, 45);

  // Configure the joydrive action so it will let the lower priority actions
  // (i.e. keydriveAct) request motion if the joystick button is
  // not pressed.
  joydriveAct.setStopIfNoButtonPressed(false);

  
  // run the robot, true means that the run will exit if connection lost
  robot.run(true);
  
  Aria::exit(0);
}
