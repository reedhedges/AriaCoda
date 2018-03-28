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

/* Tests SIM_SET_POSE command to move the robot in the simulator. */


#include "ariaInternal.h"
#include "ArArgumentParser.h";
#include "ArRobotConnector.h"
#include "ArRobot.h"
#include "ArSimUtil.h"
#include "ariaUtil.h"
#include <stdio.h>


int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  ArRobot robot;
  ArRobotConnector connector(&parser, &robot);


  if (!connector.connectRobot())
  {
    printf("Could not connect to robot... exiting\n");
    return 2;
  }
  printf("Connected to robot.\n");

  robot.runAsync(true);

  ArSimUtil sim(&robot);

  printf("-> SIM_SET_POSE 1000, 1000, 90\n");
  sim.setSimTruePose(ArPose(1000, 1000, 90));
  ArUtil::sleep(500);

  printf("-> SIM_SET_POSE 1000, -2000, -90\n");
  sim.setSimTruePose(ArPose(1000, -2000, -90));
  ArUtil::sleep(500);

  printf("-> SIM_SET_POSE 0, -2000, 180\n");
  sim.setSimTruePose(ArPose(0, -2000, 180));
  ArUtil::sleep(500);

  printf("-> SIM_SET_POSE 8000, 1, 0\n");
  sim.setSimTruePose(ArPose(8000, 1, 0));
  ArUtil::sleep(500);

  printf("-> SIM_SET_POSE -8000, -1000, 45\n");
  sim.setSimTruePose(ArPose(-8000, -1000, 45));
  ArUtil::sleep(500);

  printf("** Done with tests. Sleeping for 3 seconds and then stopping ArRobot thread... **\n");
  ArUtil::sleep(3000);
  robot.stopRunning();
  printf("** Exiting. **\n");
  return 0;
}

