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
#include <string>


/* main function */
int main(int argc, char **argv)
{
  ArRobot robot;
  ArSonarDevice sonar;

  Aria::init();
  ArLog::init(ArLog::StdErr, ArLog::Normal);

  ArSimpleConnector connector(&argc, argv);
  if (!connector.parseArgs() || argc > 1)
  {
    connector.logOptions();
    exit(1);
  }

  
  robot.addRangeDevice(&sonar);

  ArActionKeydrive keydriveAction;
  
  if (!connector.connectRobot(&robot))
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    return 1;
  }

  ArSonarAutoDisabler sonarAutoDisabler(&robot);

  robot.addAction(&keydriveAction, 50);
  puts("Drive robot with arrow keys. There is NO obstacle avoidance!");

  robot.enableMotors();
  robot.comInt(ArCommands::SOUNDTOG, 0);
  robot.comInt(ArCommands::SONAR, 1);

  robot.runAsync(true);
  

  char timestamp[24];
  while(robot.isRunning()) {
    robot.lock();
    time_t t = time(NULL);
    strftime(timestamp, 24, "%Y-%m-%d %H:%M:%S", localtime(&t));
    printf( "%s  ArRobot::areSonarsEnabled()=%s, areMotorsEnabled()=%s\n", timestamp, robot.areSonarsEnabled()?"true":"false", robot.areMotorsEnabled()?"true":"false");
    robot.unlock();
    ArUtil::sleep(1000);
  }
  
  // robot cycle stopped, probably because of lost robot connection
  Aria::shutdown();
  return 0;
}
