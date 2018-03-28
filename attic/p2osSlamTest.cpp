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
ArGripper *grip;
ArSonyPTZ *cam;

void runCommands(void)
{
  static int state = 0;
  static ArTime last;
  /* MODIFY THIS TO SEND MORE PACKETS */
  int numToSend = 4;
  int i;

  if (last.getSec() != 0 && last.getMSec() != 0 && last.mSecSince() < 8000)
    return;
  
  last.setToNow();

  for (i = 0; i < numToSend; i++)
    robot->comInt(97, state);
  
  switch (state % 4) {
  case 0:
    printf("Down Open, Down Left\n");
    cam->panTilt(-90, -45);
    grip->liftDown();
    grip->gripOpen();
    break;
  case 1:
    printf("Up Open, Up Left\n");
    cam->panTilt(-90, 45);
    grip->liftUp();
    grip->gripOpen();
    break;
  case 2:
    printf("Up Closed, Up Right\n");
    cam->panTilt(90, 45);
    grip->liftUp();
    grip->gripClose();
    break;
  case 3:
    printf("Down Closed, Down Right\n");
    cam->panTilt(90, -45);
    grip->liftDown();
    grip->gripClose();
    break;
  }
  
  for (i = 0; i < numToSend; i++)
    robot->comInt(97, state);
  
  state++;

  if (numToSend > 0)
    printf("Should be at %d times\n", state * numToSend * 2);
}

/*
  This program will just have the robot wander around, it uses some avoidance 
  routines, then just has a constant velocity.
*/

int main(void)
{
  
  ArGlobalFunctor func(&runCommands);
  // sonar, must be added to the robot
  ArSonarDevice sonar;

  // the connection
  ArSerialConnection con;
  ArActionStallRecover recover;
  ArActionBumpers bumpers;
  ArActionAvoidFront avoidFront;
  ArActionConstantVelocity constantVelocity("Constant Velocity", 400);

  // robot
  robot = new ArRobot;
  cam = new ArSonyPTZ(robot);
  grip = new ArGripper(robot);


  // mandatory init
  Aria::init();

  // set the port for the connection
  con.setPort();
  con.setBaud(38400);

  // set the device connection on the robot
  robot->setDeviceConnection(&con);
  robot->addUserTask("slam p2os", 50, &func);

  // add the sonar to the robot
  robot->addRangeDevice(&sonar);
  
  // try to connect, if we fail exit
  if (!robot->blockingConnect())
  {
    printf("Could not connect to robot->.. exiting\n");
    Aria::shutdown();
    return 1;
  }

  ArUtil::sleep(100);
  robot->addAction(&recover, 100);
  robot->addAction(&bumpers, 75);
  robot->addAction(&avoidFront, 50);
  robot->addAction(&constantVelocity, 25);

  // start the robot running, true so that if we lose connection the run stops
  robot->run(true);

  // now exit
  Aria::shutdown();
  return 0;
}

