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

ArTime requestTime;
int n = 0;
int vel = 0;
int ABOUT_RIGHT = 5;
int numPacketsGotten = 0;
int numPacketsMissed = 0;

void userTask(ArRobot *robot)
{
  int timeTaken = (requestTime.mSecSince() - 
		   robot->getIOPacketTime().mSecSince());

  if (timeTaken > 500)
  {
    ArLog::log(ArLog::Terse, "\nits been: %ld giving up and trying again (missed %d got %d)", timeTaken, numPacketsMissed, numPacketsGotten);
    numPacketsMissed++;
    robot->comInt(ArCommands::IOREQUEST, 1);
    requestTime.setToNow();
  }
  
  if (robot->getIOPacketTime().mSecSince() > requestTime.mSecSince())
    return;
  else
  {
    if (timeTaken > 30)
    {
      ArLog::log(ArLog::Terse, "\nlast packet time: %ld", timeTaken);
    }
    printf("\r%d packets gotten %d missed       ", numPacketsGotten, 
	   numPacketsMissed);
    numPacketsGotten++;
    fflush(stdout);
    robot->comInt(ArCommands::IOREQUEST, 1);
    requestTime.setToNow();
  }
}

int main(int argc, char **argv)
{
  // robot
  ArRobot robot;
  // the laser
  ArSick sick;


  // sonar, must be added to the robot
  //ArSonarDevice sonar;

  // the actions we'll use to wander
  // recover from stalls
  //ArActionStallRecover recover;
  // react to bumpers
  //ArActionBumpers bumpers;
  // limiter for close obstacles
  ArActionLimiterForwards limiter("speed limiter near", 1600, 0, 0, 1.3);
  // limiter for far away obstacles
  //ArActionLimiterForwards limiterFar("speed limiter near", 300, 1000, 450, 1.1);
  //ArActionLimiterForwards limiterFar("speed limiter far", 300, 1100, 600, 1.1);
  // limiter for the table sensors
  //ArActionLimiterTableSensor tableLimiter;
  // actually move the robot
  ArActionConstantVelocity constantVelocity("Constant Velocity", 1500);
  // turn the orbot if its slowed down
  ArActionTurn turn;

  // mandatory init
  Aria::init();

  // Parse all our args
  ArSimpleConnector connector(&argc, argv);
  if (!connector.parseArgs() || argc > 1)
  {
    connector.logOptions();
    exit(1);
  }
  
  // add the sonar to the robot
  //robot.addRangeDevice(&sonar);
  // add the laser to the robot
  robot.addRangeDevice(&sick);

  // try to connect, if we fail exit
  if (!connector.connectRobot(&robot))
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    return 1;
  }

  robot.comInt(ArCommands::SONAR, 0);

  // turn on the motors, turn off amigobot sounds
  //robot.comInt(ArCommands::SONAR, 0);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  // add the actions
  //robot.addAction(&recover, 100);
  //robot.addAction(&bumpers, 75);
  robot.addAction(&limiter, 49);
  //robot.addAction(&limiter, 48);
  //robot.addAction(&tableLimiter, 50);
  robot.addAction(&turn, 30);
  robot.addAction(&constantVelocity, 20);

  robot.setStateReflectionRefreshTime(50);
  limiter.activate();
  turn.activate();
  constantVelocity.activate();

  robot.clearDirectMotion();
  //robot.setStateReflectionRefreshTime(50);
  robot.setRotVelMax(50);
  robot.setTransAccel(1500);
  robot.setTransDecel(100);

  // start the robot running, true so that if we lose connection the run stops
  robot.runAsync(true);

  connector.setupLaser(&sick);

  // now that we're connected to the robot, connect to the laser
  sick.runAsync();

  if (!sick.blockingConnect())
  {
    printf("Could not connect to SICK laser... exiting\n");
    Aria::shutdown();
    return 1;
  }
  
  sick.lockDevice();
  sick.setMinRange(250);
  sick.unlockDevice();
  robot.lock();
  ArGlobalFunctor1<ArRobot *> userTaskCB(&userTask, &robot);
  robot.addUserTask("iotest", 100, &userTaskCB);
  requestTime.setToNow();
  robot.comInt(ArCommands::IOREQUEST, 1);
  robot.comInt(ArCommands::ENABLE, 1);
  robot.unlock();

  robot.waitForRunExit();
  // now exit
  Aria::shutdown();
  return 0;
}
