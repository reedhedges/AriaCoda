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
#include <time.h>

int main(int argc, char **argv) 
{
  // set up our simpleConnector
  ArSimpleConnector simpleConnector(&argc, argv);
  // robot
  ArRobot robot;
  
  Aria::init();
  
  // if there are more arguments left then it means we didn't
  // understand an option
  if (!simpleConnector.parseArgs() || argc > 1)
  {    
    simpleConnector.logOptions();
    exit(1);
  }

  ArRobotConfigPacketReader config(&robot);
  config.requestPacket();

  printf("Before connect: %s\n", ArUtil::convertBool(config.hasPacketArrived()));
  // set up the robot for connecting
  if (!simpleConnector.connectRobot(&robot))
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    return 1;
  }
  
  robot.comInt(ArCommands::SONAR, 0);
  printf("After connect before run %s robot orig %s\n", ArUtil::convertBool(config.hasPacketArrived()), ArUtil::convertBool(robot.getOrigRobotConfig()->hasPacketArrived()));

  robot.runAsync(true);
  ArUtil::sleep(1000);
  robot.lock();
  printf("1 second after run arrived %s robot orig %s\n", ArUtil::convertBool(config.hasPacketArrived()), ArUtil::convertBool(robot.getOrigRobotConfig()->hasPacketArrived()));
  if (config.hasPacketArrived())
  {
    config.log();
    printf("Robot maxTransVel %.0f maxRotVel %.0f transAcc %.0f transDecel %.0f rotAcc %.0f rotDecel %.0f\n", robot.getTransVelMax(), robot.getRotVelMax(), robot.getTransAccel(), robot.getTransDecel(), robot.getRotAccel(), robot.getRotDecel());
  }
  robot.unlock();
  Aria::shutdown();
  return 0;
}



