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

ArRobot *robot;

ArTime startTime;

void sonarPrinter(void)
{
  if(startTime.secSince() > 5)
    Aria::exit(0);

  int i;
  printf("\r");
  for (i = 0; i < robot->getNumSonar(); i++)
  {
    printf(" %4d", robot->getSonarRange(i));
    if (robot->isSonarNew(i))
      printf("* ");
    else
      printf("  ");
  }
  if (robot->getNumSonar() == 0)
    printf("No sonar");
  fflush(stdout);

}
  

int main(int argc, char **argv) 
{
  Aria::init();
  std::string str;
  int ret;
  int i;
  ArSensorReading *reading;
  
  ArGlobalFunctor sonarPrinterCB(&sonarPrinter);
  ArArgumentParser argP(&argc, argv);
  argP.loadDefaultArguments();
  robot = new ArRobot;
  ArRobotConnector con(&argP, robot);

  if (!(con.connectRobot()))
  {
    printf("Conn failed\n");
    exit(0);
  }
  Aria::parseArgs();


  printf("%d Sonar:  Their positions:\n", robot->getNumSonar());
  for (i = 0; i < robot->getNumSonar(); i++)
  {
    reading = robot->getSonarReading(i);
    if (reading != NULL)
      printf("%d x: %.1f y: %.1f th: %.1f\n", i, reading->getSensorX(), 
	     reading->getSensorY(), reading->getSensorTh());
  }

  robot->addUserTask("Sonar printer", 50, &sonarPrinterCB);
  robot->comInt(ArCommands::SONAR, 1);
  robot->comInt(ArCommands::SOUNDTOG, 0);

  puts("Will exit after 5 seconds.");

  robot->run(true);
  Aria::shutdown();
  
}

