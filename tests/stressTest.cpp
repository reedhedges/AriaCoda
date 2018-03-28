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

ArTime lastLoopTime;
int loopTime;

class FillerThread : public ArASyncTask
{
public:
  FillerThread(void) {}
  ~FillerThread(void) {}

  virtual void * runThread(void *arg);
protected:
  ArTime myStartBusyTime;
};

void *FillerThread::runThread(void *arg)
{
  while (1)
  {
    myStartBusyTime.setToNow();
    while (myStartBusyTime.mSecSince() < 150);
  }
}

class JoydriveAction : public ArAction
{
public:
  JoydriveAction(void);
  virtual ~JoydriveAction(void) {};
  virtual ArActionDesired *fire(ArActionDesired currentDesired);
  bool joystickInited(void);
protected:
  ArActionDesired myDesired;
  ArJoyHandler myJoyHandler;
};

JoydriveAction::JoydriveAction(void) :
  ArAction("Joydrive Action", "This action reads the joystick and sets the translational and rotational velocity based on this.")
{
  myJoyHandler.init();
  myJoyHandler.setSpeeds(50, 700);
}

bool JoydriveAction::joystickInited(void)
{
  return myJoyHandler.haveJoystick();
}

ArActionDesired *JoydriveAction::fire(ArActionDesired currentDesired)
{
  int rot, trans;

  printf("%6ld ms since last loop. ms longer than desired:  %6ld.  mpac %d\n",
	 lastLoopTime.mSecSince(), 
	 lastLoopTime.mSecSince() - loopTime, myRobot->getMotorPacCount());
  lastLoopTime.setToNow();

  if (myJoyHandler.haveJoystick() && (myJoyHandler.getButton(1) ||
				    myJoyHandler.getButton(2)))
  {
    if (ArMath::fabs(myRobot->getVel()) < 10.0)
      myRobot->comInt(ArCommands::ENABLE, 1);
    myJoyHandler.getAdjusted(&rot, &trans);
    myDesired.setVel(trans);
    myDesired.setDeltaHeading(-rot);
    return &myDesired;
  }
  else
  {
    myDesired.setVel(0);
    myDesired.setDeltaHeading(0);
    return &myDesired;
  }
}

int main(void)
{
  ArTcpConnection con;
  ArRobot robot;
  int ret;
  std::string str;
  JoydriveAction jdAct;
  FillerThread ft;

  ft.create();

  FillerThread ft2;

  ft2.create();

  Aria::init();
  /*
  if (!jdAct.joystickInited())
  {
    printf("Do not have a joystick, set up the joystick then rerun the program\n\n");
    Aria::shutdown();
    return 1;
  }
  */
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
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    return 1;
  }

  robot.comInt(ArCommands::SONAR, 0);
  robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  lastLoopTime.setToNow();
  loopTime = robot.getCycleTime();

  robot.addAction(&jdAct, 100);
  robot.runAsync(true);
  
  robot.waitForRunExit();
  Aria::shutdown();
  return 0;
}
