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

/*
  This is an example of how to use the limiting actions.

  It requires a SICK laser. Use either a joystick or keyboard
  to drive the robot.  If the SICK laser detects obstacles, actions
  slow down the robot as it approaches.
  
  The way it works is that it has a limiting behavior higher priority
  than the joydrive and keydrive actions.  So if the limiting behaviors
  detect obstacles that are too close, they set speed limits proportional 
  to the distance that slow down the robot and eventually prevent the 
  joydrive and keydrive actions from having any effect on the robot.
*/


int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser argParser(&argc, argv);
  argParser.loadDefaultArguments();
  argParser.addDefaultArgument("-connectLaser"); // also by default always try to connect to the first laser.

  ArRobot robot;
  ArRobotConnector robotConnector(&argParser, &robot);
  ArLaserConnector laserConnector(&argParser, &robot, &robotConnector);

  ArActionJoydrive jdAct;
  ArActionKeydrive keyAct;
  
  // limiter for close obstacles
  ArActionLimiterForwards limiter("speed limiter near", 300, 600, 250);

  // limiter for far away obstacles
  ArActionLimiterForwards limiterFar("speed limiter far", 300, 1100, 400);

  // limiter that uses the IR sensors on a Peoplebot, if equipped
  ArActionLimiterTableSensor tableLimiter;

  // limiter so we don't bump things backwards
  ArActionLimiterBackwards backwardsLimiter;

  
  if(!robotConnector.connectRobot())
  {
    printf("Could not connect to robot... exiting\n");
    Aria::logOptions();
    Aria::exit(1);
    return 1;
  }

  if(!Aria::parseArgs() || !argParser.checkHelpAndWarnUnparsed())
  {
    printf("Could not connect to robot... exiting\n");
    Aria::logOptions();
    Aria::exit(2);
    return 2;
  }

  ArLog::log(ArLog::Normal, "Connected to robot.");
  robot.runAsync(true);

  // laserConnector uses parameters from the robot parameter file(s) and from
  // command line options to determine what kinds of lasers the robot has and
  // how to connect to them.  See the section on robot parameter files in the
  // ARIA API Reference Manual, and the ArLaserConnector class documentation.

  if(!laserConnector.connectLasers())
  {
    ArLog::log(ArLog::Terse, "Error connecting to configured lasers (see robot parameter file(s) and command line options)");
    Aria::exit(3);
    return 3;
  }

  // If laserConnector succeeded in connecting to lasers, ArLaser interface
  // objects for them  are stored in a
  // list in the ArRobot object (robot).  ArRobot can automatically use them in
  // its utilities for checking range devices for obstacles, and you can query
  // that list. (ArLaser is a  subclass of ArRangeDevice.)

  // enable the motors, disable amigobot sounds
  robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  // add the actions, use high priority numbers for the limitier actions.
  robot.addAction(&tableLimiter, 100);
  robot.addAction(&limiter, 95);
  robot.addAction(&limiterFar, 90);
  robot.addAction(&backwardsLimiter, 85);
  robot.addAction(&keyAct, 51);
  robot.addAction(&jdAct, 50);
  
  // wait for end of robot connection
  robot.waitForRunExit();
  
  Aria::exit(0);
  return 0;
}
