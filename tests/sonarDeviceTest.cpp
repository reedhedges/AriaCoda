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

ArSonarDevice sonar;

void sonarPrinter(void)
{
  ArSonarDevice *sd;
  
  std::list<ArPoseWithTime *>::iterator it;
  std::list<ArPoseWithTime *> *readings;
  ArPose *pose;

  sd = (ArSonarDevice *)robot->findRangeDevice("sonar");
  if (sd != NULL)
  {
    sd->lockDevice();
    readings = sd->getCurrentBuffer();
    if (readings != NULL)
    {
      for (it = readings->begin(); it != readings->end(); ++it)
      {
	pose = (*it);
	//pose->log();
      }
    }
    sd->unlockDevice();
  }

  double d;
  double th;
  int i;
	
  printf("Closest readings within polar sections:\n");

  d = sonar.currentReadingPolar(-45, 45, &th);
  printf(" front quadrant: %5.0f  ", d);
  if (d != 5000)
    printf("%3.0f ", th);
  printf("\n");

  d = sonar.currentReadingPolar(-135, -45, &th);
  printf(" right quadrant: %5.0f ", d);
  if (d != 5000)
    printf("%3.0f ", th);
  printf("\n");

  d = sonar.currentReadingPolar(45, 135, &th);
  printf(" left quadrant: %5.0f ", d);
  if (d != 5000)
    printf("%3.0f ", th);
  printf("\n");

  d = sonar.currentReadingPolar(-135, 135, &th);
  printf(" back quadrant: %5.0f ", d);
  if (d != 5000)
    printf("%3.0f ", th);
  printf("\n");

  fflush(stdout);
}

int main(void)
{
  std::string str;
  int ret;
  ArGlobalFunctor sonarPrinterCB(&sonarPrinter);
  ArTcpConnection con;
  Aria::init();

  robot = new ArRobot;

  if ((ret = con.open()) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    exit(0);
  }

  robot->setDeviceConnection(&con);
  if (!robot->blockingConnect())
  {
    printf("Could not connect to robot->.. exiting\n");
    exit(0);
  }

  robot->addRangeDevice(&sonar);

  robot->addUserTask("Sonar printer", 50, &sonarPrinterCB);
  robot->comInt(ArCommands::SONAR, 1);
  robot->comInt(ArCommands::SOUNDTOG, 0);

  robot->run(true);
  Aria::shutdown();


}
