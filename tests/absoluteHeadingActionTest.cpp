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

class ActionTurnToHeading : public ArAction
{
public:
  ActionTurnToHeading(double heading = 0);
  virtual ~ActionTurnToHeading(void) {}
  virtual ArActionDesired *fire(ArActionDesired currentDesired);
  void setHeading(double heading);
protected:
  ArActionDesired myDesired;
  double myHeading;
};

ActionTurnToHeading::ActionTurnToHeading(double heading) :
  ArAction("TurnToHeading", "Turns to a heading")
{
  setNextArgument(ArArg("desired heading", &myHeading, "Heading to turn to."));
  myHeading = heading;
}

void ActionTurnToHeading::setHeading(double heading)
{
  myHeading = heading;
}

ArActionDesired *ActionTurnToHeading::fire(ArActionDesired currentDesired)
{
  myDesired.reset();
  
  myDesired.setHeading(myHeading);
  
  return &myDesired;
}

class ConnHandler
{
public:
  ConnHandler(ArRobot *robot);
  ~ConnHandler(void) {}
  void connFail(void);
  void connected(void);
protected:
  ArRobot *myRobot;
  ArFunctorC<ConnHandler> *myConnFailCB;
  ArFunctorC<ConnHandler> *myConnectedCB;
};

ConnHandler::ConnHandler(ArRobot *robot) 
{
  myRobot = robot;
  myConnectedCB = new ArFunctorC<ConnHandler>(this, &ConnHandler::connected);
  myRobot->addConnectCB(myConnectedCB, ArListPos::FIRST);
  myConnFailCB = new ArFunctorC<ConnHandler>(this, &ConnHandler::connFail);
  myRobot->addFailedConnectCB(myConnFailCB, ArListPos::FIRST);
}

void ConnHandler::connFail(void)
{
  printf("Failed to connect.\n");
  myRobot->stopRunning();
  Aria::shutdown();
  return;
}

void ConnHandler::connected(void)
{
  printf("Connected\n");
  myRobot->comInt(ArCommands::SONAR, 0);
  myRobot->comInt(ArCommands::ENABLE, 1);
  myRobot->comInt(ArCommands::SOUNDTOG, 0);
}

int main(int argc, char **argv) 
{
  std::string str;
  int ret;

  ArSerialConnection con;
  ArRobot robot;
  ConnHandler ch(&robot);

  ActionTurnToHeading headingAction;
  
  Aria::init(Aria::SIGHANDLE_THREAD);

  if ((ret = con.open()) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    Aria::shutdown();
    return 1;
  }


  robot.setDeviceConnection(&con);
  if (!robot.blockingConnect())
  {
    printf(
    "asyncConnect failed because robot is not running in its own thread.\n");
    Aria::shutdown();
    return 1;
  }
  
  robot.addAction(&headingAction, 100);
  robot.runAsync(true);

  robot.enableMotors();

  printf("Turning the robot to 180, then sleeping 4 seconds.\n");
  headingAction.setHeading(180);
  ArUtil::sleep(4000);
  printf("Robot got to %.1f\n", robot.getTh());

  printf("Turning the robot to 30, then sleeping 4 seconds.\n");
  headingAction.setHeading(30);
  ArUtil::sleep(4000);

  printf("Robot got to %.1f\n", robot.getTh());
  printf("Turning the robot to -170, then sleeping 4 seconds.\n");
  headingAction.setHeading(-170);
  ArUtil::sleep(4000);

  printf("Robot got to %.1f\n", robot.getTh());
  printf("Turning the robot to -130, then sleeping 4 seconds.\n");
  headingAction.setHeading(-130);
  ArUtil::sleep(4000);

  printf("Robot got to %.1f\n", robot.getTh());
  printf("Now exiting\n");
  Aria::exit();
  return 0;
}

