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

// this test prints out the middle readings of the laser continually

ArSick *sick;

void failedConnect(void)
{
  printf("Failed connect\n");
  system("echo 'Failed' >> results");
  sick->stopRunning();
  sick->disconnect();
  exit(0);
}

int main(int argc, char **argv)
{
  int ret;
  std::string str;
  ArSerialConnection con;
  const std::list<ArSensorReading *> *readings;
  std::list<ArSensorReading *>::const_iterator it;
  int i;
  ArGlobalFunctor failedConnectCB(&failedConnect);
  
  Aria::init();
  sick = new ArSick;
  // open the connection, if it fails, exit
  if ((ret = con.open("/dev/ttyS2")) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    Aria::shutdown();
    return 1;
  }
  
  sick->configure(false);
  sick->setDeviceConnection(&con);

  sick->addFailedConnectCB(&failedConnectCB, ArListPos::FIRST);
  sick->runAsync();

  ArUtil::sleep(100);
  sick->lockDevice();
  sick->asyncConnect();
  sick->unlockDevice();
  while (!sick->isConnected())
    ArUtil::sleep(100);
  printf("Connected\n");
//  while (sick->isConnected())
  while (1)
  {
    //dist = sick->getCurrentBuffer().getClosestPolar(-90, 90, ArPose(0, 0), 30000, &angle);
    sick->lockDevice();
    if (!sick->getRunning() || !sick->isConnected())
      {
	break;
      }
    printf("\r");
    readings = sick->getRawReadings();
    if (readings != NULL)
    {
      for (i = 0, it = readings->begin(); it != readings->end(); it++, i++)
      {
	if (abs(i - readings->size()/2) < 3)
	  printf("(%.2f %d) ", (*it)->getSensorTh(), (*it)->getRange());
      }
    }
    // switch the commenting in out of the fflush and the
    // printf("\n"); if you want to print on the same line or have it
    // scrolling
    fflush(stdout);
    //printf("\n");

    sick->unlockDevice();
    ArUtil::sleep(100);
  }
  sick->lockDevice();
  sick->stopRunning();
  sick->disconnect();
  sick->unlockDevice();
   return 0;
}




