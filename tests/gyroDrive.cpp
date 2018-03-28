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

/** If you start this program up with an argument it will not use the gyro
    with no argument it will use the gyro
**/
ArRobot *robot;

void printer(void)
{
  printf("%.2f\n", robot->getTh());  
}

void hardExit(void)
{
  ArKeyHandler *keyHandler;
  robot->disconnect();
  if ((keyHandler = Aria::getKeyHandler()) != NULL)
    keyHandler->restore();
  else
    printf("Could not restore keyboard settings.");
  exit(0);
}

int main(int argc, char **argv)
{
  bool done;
  double distToTravel = 3000;
  int spinTime = 0;

  // set up our simpleConnector
  ArSimpleConnector simpleConnector(&argc, argv);
  // set up a key handler so escape exits and attach to the robot
  ArKeyHandler keyHandler;

  Aria::init();

  robot = new ArRobot;

  printf("You can press the escape key to exit this program\n");

  // parse its arguments
  if (simpleConnector.parseArgs())
  {
    simpleConnector.logOptions();
    exit(1);
  }

  // if there are more arguments left then it means we didn't
  // understand an option
  /*
  if (argc > 1)
  {    
    simpleConnector.logOptions();
    keyHandler.restore();
    exit(1);
  }
  */

  ArGlobalFunctor exitCB(&hardExit);
  ArGlobalFunctor printerCB(&printer);

  keyHandler.addKeyHandler(ArKeyHandler::ESCAPE, &exitCB);
  robot->attachKeyHandler(&keyHandler);
  Aria::setKeyHandler(&keyHandler);



  // set up the robot for connecting
  if (!simpleConnector.connectRobot(robot))
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    keyHandler.restore();
    return 1;
  }

  //robot->addUserTask("printer", 50, &printerCB);
  // run the robot, true here so that the run will exit if connection lost
  robot->runAsync(true);

#ifdef WIN32
  // wait until someone pushes the motor button to go
  printf("Press the motor button to start the robot moving\n");
  while (1)
  {
    robot->lock();
    if (!robot->isRunning())
      hardExit();
    if (robot->areMotorsEnabled())
    {
      robot->unlock();
      break;
    }
    robot->unlock();
    ArUtil::sleep(100);
  }
#endif
  ArAnalogGyro *gyro;

  if (argc == 1)
  {
    printf("Gyro\n");
    gyro = new ArAnalogGyro(robot);
  }
  printf("Waiting for inertial to stabilize for 5 seconds.\n");
  // wait a bit for the inertial to warm up
  ArUtil::sleep(5000);
  // basically from here on down the robot just cruises around a bit
  robot->lock();
  // enable the motors, disable amigobot sounds
  robot->comInt(ArCommands::SONAR, 0);
  robot->comInt(ArCommands::ENABLE, 1);
  robot->setMoveDoneDist(200);

  // move a couple meters
  printf("Driving out\n");
  robot->move(distToTravel);
  robot->setHeading(0);
  robot->unlock();
  do {
    ArUtil::sleep(100);
    robot->lock();
    //robot->setHeading(0);
    done = robot->isMoveDone(200);
    robot->unlock();
  } while (!done);

  if (spinTime != 0)
  {
    printf("Spinning a while\n");
    // rotate a few times
    robot->lock();
    robot->setRotVel(200);
    robot->unlock();
    ArUtil::sleep(spinTime * 1000);
  }

  printf("Pointing back\n");
  robot->lock();
  robot->setHeading(180);
  robot->unlock();
  do {
    ArUtil::sleep(100);
    robot->lock();
    //robot->setHeading(180);
    done = robot->isHeadingDone(5);
    robot->unlock();
  } while (!done);

  printf("Driving back\n");
  // move a couple meters
  robot->lock();
  robot->move(distToTravel);
  robot->setHeading(180);
  robot->unlock();
  do {
    ArUtil::sleep(100);
    robot->lock();
    //robot->setHeading(180);
    done = robot->isMoveDone(200);
    robot->unlock();
  } while (!done);

  printf("Pointing back in original direction.\n");
  robot->lock();
  robot->setHeading(0);
  robot->unlock();
  do {
    ArUtil::sleep(100);
    robot->lock();
    //robot->setHeading(0);
    done = robot->isHeadingDone(5);
    robot->unlock();
  } while (!done);


  robot->lock();
  printf("Final heading %.2f\n", robot->getTh());
  robot->disconnect();
  robot->unlock();
  // now exit
  Aria::shutdown();
  return 0;
}

