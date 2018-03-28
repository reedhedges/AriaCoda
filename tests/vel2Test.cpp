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

int main(int argc, char **argv) 
{
  Aria::init();
  
  ArArgumentParser argParser(&argc, argv);
  argParser.loadDefaultArguments();


  ArRobot robot;
  ArRobotConnector con(&argParser, &robot);

  if(!Aria::parseArgs())
  {
    Aria::logOptions();
    Aria::exit(1);
    return 1;
  }

  if(!con.connectRobot())
  {
    ArLog::log(ArLog::Normal, "vel2Test: Could not connect to the robot. Exiting.");

    if(argParser.checkHelpAndWarnUnparsed()) 
    {
      Aria::logOptions();
	}
    Aria::exit(1);
    return 1;
  }

  ArLog::log(ArLog::Normal, "vel2Test: Connected.");

  if(!Aria::parseArgs() || !argParser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(1);
  }

  robot.comInt(ArCommands::SONAR, 0);
  robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  // Run the robot processing cycle in its own thread. Note that after starting this
  // thread, we must lock and unlock the ArRobot object before and after
  // accessing it.
  robot.runAsync(false);



  printf("vel2Test: Telling the robot to go 300 mm on left wheel and 100 mm on right wheel for 5 seconds\n");
  robot.lock();
  robot.setVel2(300, 100);
  robot.unlock();
  ArTime start;
  start.setToNow();
  while (1)
  {
    robot.lock();
    if (start.mSecSince() > 5000)
    {
      robot.unlock();
      break;
    }   
    robot.unlock();
    ArUtil::sleep(50);
  }
  
  printf("vel2Test: Setting vel2 to 200 mm/sec on both wheels, then sleeping 3 seconds\n");
  robot.lock();
  robot.setVel2(200, 200);
  robot.unlock();
  ArUtil::sleep(3000);
  printf("vel2Test: Stopping the robot, then sleeping for 2 seconds\n");
  robot.lock();
  robot.stop();
  robot.unlock();
  ArUtil::sleep(2000);
  printf("vel2Test: Setting velocity to 200 mm/sec then sleeping 3 seconds\n");
  robot.lock();
  robot.setVel(200);
  robot.unlock();
  ArUtil::sleep(3000);
  printf("vel2Test: Stopping the robot, then sleeping for 2 seconds\n");
  robot.lock();
  robot.stop();
  robot.unlock();
  ArUtil::sleep(2000);
  printf("vel2Test: Setting vel2 with 0 on left wheel, 200 mm/sec on right, then sleeping 5 seconds\n");
  robot.lock();
  robot.setVel2(0, 200);
  robot.unlock();
  ArUtil::sleep(5000);
  printf("vel2Test: Telling the robot to rotate at 50 deg/sec then sleeping 5 seconds\n");
  robot.lock();
  robot.setRotVel(50);
  robot.unlock();
  ArUtil::sleep(5000);
  printf("vel2Test: Telling the robot to rotate at -50 deg/sec then sleeping 5 seconds\n");
  robot.lock();
  robot.setRotVel(-50);
  robot.unlock();
  ArUtil::sleep(5000);
  printf("vel2Test: Setting vel2 with 0 on both wheels, then sleeping 3 seconds\n");
  robot.lock();
  robot.setVel2(0, 0);
  robot.unlock();
  ArUtil::sleep(3000);
  printf("vel2Test: Now having the robot change heading by -125 degrees, then sleeping for 6 seconds\n");
  robot.lock();
  robot.setDeltaHeading(-125);
  robot.unlock();
  ArUtil::sleep(6000);
  printf("vel2Test: Now having the robot change heading by 45 degrees, then sleeping for 6 seconds\n");
  robot.lock();
  robot.setDeltaHeading(45);
  robot.unlock();
  ArUtil::sleep(6000);
  printf("vel2Test: Setting vel2 with 200 on left wheel, 0 on right wheel, then sleeping 5 seconds\n");
  robot.lock();
  robot.setVel2(200, 0);
  robot.unlock();
  ArUtil::sleep(5000);

  printf("vel2Test: Done, exiting.\n");
  Aria::exit(0);
  return 0;
}

