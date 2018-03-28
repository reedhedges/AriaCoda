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

/* This tests the response time for IO packet requests sent to the robot.  It can
 * also test response for velocity commands if the section is uncommented.
 *
 * This can be used to see what the delay is in the time a command is sent to the
 * time the command takes effect.
 */

ArTime requestTime;
ArTime velTime;
int n = 0;
int vel = 0;
int ABOUT_RIGHT = 5;

void userTask(ArRobot *robot)
{
  if (robot->getIOPacketTime().mSecSince() > requestTime.mSecSince())
    return;
  else
  {
    ArLog::log(ArLog::Terse, "last packet time: %ld", requestTime.mSecSince() - robot->getIOPacketTime().mSecSince());
    fflush(stdout);
    robot->comInt(ArCommands::IOREQUEST, 1);
    requestTime.setToNow();
  }

  /*  Uncomment the next section to test response in velocity commands. */

  /*
  if (abs((int)(robot->getVel() - vel)) <= ABOUT_RIGHT &&
      velTime.secSince() > 5)
  {
    if (vel == 0)
      vel = 200;
    else if (vel == 200)
      vel = 400;
    else if (vel == 400)
      vel = 0;
    robot->setVel(vel);
    velTime.setToNow();
  }

  ArLog::log(ArLog::Terse, "vel requested: %d\trobot vel: %3.2f\tvel time: %d", vel, robot->getVel(), velTime.mSecSince());
  */

  ArLog::log(ArLog::Terse, "Aria cycle time: %d", robot->getCycleTime());
  fflush(stdout);
}

int main(int argc, char** argv)
{
  // set up our simpleConnector
  ArSimpleConnector simpleConnector(&argc, argv);
  // robot
  ArRobot robot;
  // a key handler so we can do our key handling
  ArKeyHandler keyHandler;

  ArLog::init(ArLog::StdOut,ArLog::Verbose);

  // if there are more arguments left then it means we didn't
  // understand an option
  if (!simpleConnector.parseArgs() || argc > 1)
  {    
    simpleConnector.logOptions();
    keyHandler.restore();
    exit(1);
  }

  // mandatory init
  Aria::init();
  ArLog::init(ArLog::StdOut, ArLog::Terse, NULL, true);

  // let the global aria stuff know about it
  Aria::setKeyHandler(&keyHandler);
  // toss it on the robot
  robot.attachKeyHandler(&keyHandler);

  // set up the robot for connecting
  if (!simpleConnector.connectRobot(&robot))
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    keyHandler.restore();
    return 1;
  }

  // turn on the motors for the velocity response test
  robot.comInt(ArCommands::ENABLE, 1);
  velTime.setToNow();

  // turn off the sonar
  robot.comInt(ArCommands::SONAR, 0);

  ArGlobalFunctor1<ArRobot *> userTaskCB(&userTask, &robot);
  robot.addUserTask("iotest", 100, &userTaskCB);

  robot.comInt(ArCommands::IOREQUEST, 1);
  requestTime.setToNow();

  //start the robot running, true so that if we lose connection the run stops
  robot.run(true);
  
  // now exit
  Aria::shutdown();
  return 0;


}

