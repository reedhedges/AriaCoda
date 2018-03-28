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

ArRobot *robot;

ArSick laser;

void laserPrinter(void)
{
  double d;
  double th;
	

  d = laser.currentReadingPolar(-45, 45, &th);
  printf("front: %5.0fmm ", d);
  if (d != laser.getMaxRange())
    printf("%3.0fdeg ", th);
  else
    printf("???deg ");

  d = laser.currentReadingPolar(-135, -45, &th);
  printf(" right: %5.0fmm ", d);
  if (d != laser.getMaxRange())
    printf("%3.0fdeg ", th);
  else
    printf("???deg ");

  d = laser.currentReadingPolar(45, 135, &th);
  printf(" left: %5.0fmm ", d);
  if (d != laser.getMaxRange())
    printf("%3.0fdeg ", th);
  else
    printf("???deg ");

  d = laser.currentReadingPolar(135, -135, &th);
  printf(" back: %5.0fmm ", d);
  if (d != laser.getMaxRange())
    printf("%3.0fdeg ", th);
  else
    printf("???deg ");

  printf("\r");
  fflush(stdout);
}

int main(int argc, char** argv)
{
  ArGlobalFunctor laserPrinterCB(&laserPrinter);
  Aria::init();

  ArSimpleConnector connector(&argc, argv);
  if(!connector.parseArgs())
  {
    connector.logOptions();
    return 1;
  }

  robot = new ArRobot;
  robot->addRangeDevice(&laser);

  if(!connector.connectRobot(robot))
  {
    printf("Could not connect to robot.\n");
    return 2;
  }

  connector.setupLaser(&laser);
  laser.runAsync();
  if(!laser.blockingConnect())
  {
    printf("Could not connect to SICK laser\n");
    return 3;
  }

  robot->comInt(ArCommands::SOUNDTOG, 0);

  printf("Closest readings within quadrants:\n");
  robot->addUserTask("Laser printer", 50, &laserPrinterCB);

  robot->run(true);
  Aria::shutdown();


}
