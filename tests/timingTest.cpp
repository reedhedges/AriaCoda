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

ArTime lastLoopTime;
int loopTime;



void loopTester(void)
{
  printf("%6ld ms since last loop. ms longer than desired:  %6ld.\n",
	 lastLoopTime.mSecSince(), 
	 lastLoopTime.mSecSince() - loopTime);
  lastLoopTime.setToNow();

}

int main(void)
{
  ArRobot robot;
  ArTcpConnection conn;
  ArGlobalFunctor loopTesterCB(&loopTester);

  lastLoopTime.setToNow();
  Aria::init();

  conn.setPort();
  robot.setDeviceConnection(&conn);
  robot.setCycleChained(true);
  if (!robot.blockingConnect())
  {
    printf("Could not connect to robot.\n");
    Aria::shutdown();
    exit(0);
  }
  robot.comInt(ArCommands::ENCODER, 1);
  robot.setCycleTime(100);
  loopTime = robot.getCycleTime();
  robot.addUserTask("loopTester", 0, &loopTesterCB);
  robot.run(false);

  Aria::shutdown();
}



