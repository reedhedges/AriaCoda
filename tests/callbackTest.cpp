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

class CBTest
{
public:
  CBTest(void) {};
  ~CBTest(void) {};
  void connected(void) { printf("Connected\n"); }
  void disconnected(void) { printf("Disconnected\n"); }
  void disconnectedError(void) { printf("Disconnected Error\n"); }
  void failedConnect(void) { printf("Failed Connect\n"); }
 
};

int main(void)
{
  int ret;
  std::string str;
  CBTest cbTest;

  ArFunctorC<CBTest> connectCB(&cbTest, &CBTest::connected);
  ArFunctorC<CBTest> failedConnectCB(&cbTest, &CBTest::failedConnect);
  ArFunctorC<CBTest> disconnectCB(&cbTest, &CBTest::disconnected);
  ArFunctorC<CBTest> disconnectErrorCB(&cbTest, &CBTest::disconnectedError);

  ArSerialConnection con;
  ArRobot robot;

  printf("If a robot is attached to your port you should see:\n");
  printf("Failed connect, Connected, Disconnected Error, Connected, Disconnected\n");
  printf("If no robot is attached you should see:\n");
  printf("Failed connect, Failed connect, Failed connect\n");
  printf("-------------------------------------------------------\n");
  ArLog::init(ArLog::None, ArLog::Terse);

  srand(time(NULL));

  robot.setDeviceConnection(&con);
  robot.addConnectCB(&connectCB, ArListPos::FIRST);
  robot.addFailedConnectCB(&failedConnectCB, ArListPos::FIRST);
  robot.addDisconnectNormallyCB(&disconnectCB, ArListPos::FIRST);
  robot.addDisconnectOnErrorCB(&disconnectErrorCB, ArListPos::FIRST);
  
  // this should fail since there isn't an open port yet
  robot.blockingConnect();
  
  if ((ret = con.open()) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    exit(0);
  }
  
  robot.blockingConnect();

  con.close();
  robot.loopOnce();
  

  if ((ret = con.open()) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    exit(0);
  }  
  robot.blockingConnect();
  robot.disconnect();

  exit(0);

}
