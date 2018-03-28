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

ArTime startTime;
ArTime responseTime;
bool enabled = true;
bool responded;
int numPacketsGotten = 0;
int numPacketsMissed = 0;


void userTask(ArRobot *robot)
{
  // if it's set to how it should set the time
  if ((enabled && robot->areMotorsEnabled()) || 
      (!enabled && !robot->areMotorsEnabled()))
  {
    responded = true;
    printf("responded %d\n", enabled);
    responseTime.setToNow();
  }

  int timeTaken = (startTime.mSecSince() - 
		   responseTime.mSecSince());

  if (timeTaken > 500)
  {
    ArLog::log(ArLog::Terse, "\nits been: %ld giving up and trying again (missed %d got %d)", timeTaken, numPacketsMissed, numPacketsGotten);
    numPacketsMissed++;
    robot->comInt(ArCommands::ENABLE, enabled);
    startTime.setToNow();
  }
  
  if (responded)
  {
    if (timeTaken > 30)
    {
      ArLog::log(ArLog::Terse, "\nlast packet time: %ld", timeTaken);
    }
    printf("\r%d packets gotten %d missed       ", numPacketsGotten, 
	   numPacketsMissed);
    numPacketsGotten++;
    fflush(stdout);
    enabled = !enabled;
    robot->comInt(ArCommands::ENABLE, enabled);
    startTime.setToNow();
    responded = false;
  }
}

int main(int argc, char **argv)
{

  // mandatory init
  Aria::init();

  // This object parses program options from the command line
  ArArgumentParser parser(&argc, argv);

  // Load some default values for command line arguments from /etc/Aria.args
  // (Linux) or the ARIAARGS environment variable.
  parser.loadDefaultArguments();



  // Central object that is an interface to the robot and its integrated
  // devices, and which manages control of the robot by the rest of the program.
  ArRobot robot;

  // Object that connects to the robot or simulator using program options
  ArRobotConnector robotConnector(&parser, &robot);

  // If the robot has an Analog Gyro, this object will activate it, and 
  // if the robot does not automatically use the gyro to correct heading,
  // this object reads data from it and corrects the pose in ArRobot
  ArAnalogGyro gyro(&robot);

  robot.setConnectWithNoParams(true);

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  if (!robotConnector.connectRobot())
  {
    // Error connecting:
    // if the user gave the -help argumentp, then just print out what happened,
    // and continue so options can be displayed later.
    if (!parser.checkHelpAndWarnUnparsed())
    {
      ArLog::log(ArLog::Terse, "Could not connect to robot, will not have parameter file so options displayed later may not include everything");
    }
    // otherwise abort
    else
    {
      ArLog::log(ArLog::Terse, "Error, could not connect to robot.");
      Aria::logOptions();
      Aria::exit(1);
    }
  }

  // start the robot running, true so that if we lose connection the run stops
  robot.runAsync(true);

  robot.lock();
  ArGlobalFunctor1<ArRobot *> userTaskCB(&userTask, &robot);
  robot.addUserTask("connectionTest", 100, &userTaskCB);
  startTime.setToNow();
  enabled = true;
  robot.comInt(ArCommands::ENABLE, 1);
  robot.unlock();

  robot.waitForRunExit();
  // now exit
  Aria::shutdown();
  return 0;
}
