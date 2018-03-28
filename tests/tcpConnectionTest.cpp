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

/* Test ArTcpConnection */

const char* host = "127.0.0.1";

int main(int argc, char **argv)
{
  ArRobot robot;
  Aria::init();
  ArTcpConnection tcpCon;

  printf("Opening TCP connection to %s...\n", host);
  int status = tcpCon.open(host);
  if(status != 0)
  {
    printf("Failed to connect via TCP to %s: %s.\n", host, tcpCon.getOpenMessage(status));
    exit(1);
  }
  
  printf("Connecting to robot...\n");
  robot.setDeviceConnection(&tcpCon);
  if(!robot.blockingConnect())
  {
    printf("Failed to connect to robot.\n");
    exit(2);
  }

  printf("Connected to robot.\n");

  // Start the robot running in the background.
  // True parameter means that if the connection is lost, then the 
  // run loop ends.
  robot.runAsync(true);

  // Sleep for 3 seconds.
  printf("Sleeping for 3 seconds...\n");
  ArUtil::sleep(3000);
  
  printf("Ending robot thread...\n");
  robot.stopRunning();

  printf("Exiting.\n");
  return 0;
}
