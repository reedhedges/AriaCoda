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

// number of minutes to wait between logs
int numMins = 1;
double shutdownVoltage = 11.0;

ArRobot *robot;
ArTime startTime;
ArTime lastOut;
FILE *outFile;
bool first = true;

void batteryLogger(void)
{
  if (outFile == NULL)
  {
    outFile = ArUtil::fopen("batteryVoltage.txt", "w+");
    startTime.setToNow();
    if (outFile == NULL)
    {
      printf("Could not open file.  Exiting.\n");
      robot->disconnect();
      robot->stopRunning();
      Aria::shutdown();
      exit(0);
    }
    printf("File opened\n");
  }
  if (outFile != NULL)
  {
    if (lastOut.secSince() > 60 * numMins || first)
    {
      if (fprintf(outFile, "%ld %.2f\n", startTime.secSince()/60, robot->getBatteryVoltage()) <= 0)
	printf("PROBLEM writing to file\n");
      printf("%ld %.2f\n", startTime.secSince()/60, robot->getBatteryVoltage());
      fflush(outFile);
      lastOut.setToNow();
      first = false;
    }	      
  }
#ifndef WIN32
  if (startTime.secSince() > 30 &&
      robot->getBatteryVoltage() < shutdownVoltage)
    system("halt\n");
#endif
    
}
  

int main(int argc, char **argv) 
{
  std::string str;
  int ret;
  
  ArGlobalFunctor batteryLoggerCB(&batteryLogger);
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
  robot->addUserTask("Battery Logger", 50, &batteryLoggerCB);
  robot->comInt(ArCommands::SONAR, 0);
  robot->comInt(ArCommands::SOUNDTOG, 0);

  robot->run(true);
  Aria::shutdown();
  
}

