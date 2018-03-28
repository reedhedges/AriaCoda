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

FILE *file;
ArTime lastFiled;

void printer(void)
{
  printf("\r%.1f, %.2f", robot->getBatteryVoltage(), 
         robot->getIOAnalogVoltage(4));
  fflush(stdout);
  if (file != NULL && lastFiled.mSecSince() > 1000)
    {
      fprintf(file, "\r%.1f, %.2f\n", robot->getBatteryVoltage(), 
              robot->getIOAnalogVoltage(4));
      lastFiled.setToNow();
      fflush(file);
    }
}
  

int main(int argc, char **argv) 
{
  std::string str;
  int ret;
  int i;
  ArSensorReading *reading;
  
  ArGlobalFunctor printerCB(&printer);
  ArSerialConnection con;
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
    printf("Could not connect to robot... exiting\n");
    exit(0);
  }

  file = ArUtil::fopen("chargeLog.txt", "w+");
  lastFiled.setToNow();
  robot->comInt(ArCommands::IOREQUEST, 2);

  robot->addUserTask("printer", 50, &printerCB);
  robot->comInt(ArCommands::SONAR, 1);
  robot->comInt(ArCommands::SOUNDTOG, 0);

  

  robot->run(true);
  Aria::shutdown();
  
}
