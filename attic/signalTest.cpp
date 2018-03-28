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

void sigHandlerTest(int signal)
{
  printf("Sig %d %s\n", signal, ArSignalHandler::nameSignal(signal));
}

int main(int argc, char** argv)
{
  // set up our simpleConnector
  ArSimpleConnector simpleConnector(&argc, argv);
  // robot
  ArRobot robot;
  // a laser in case one is used
  ArSick sick;
  // a key handler so we can do our key handling
  ArKeyHandler keyHandler;
  // sonar, must be added to the robot, for teleop and wander
  ArSonarDevice sonarDev;

  // parse the args and if there are more arguments left then it means
  // we didn't understand an option
  if (!simpleConnector.parseArgs() || argc > 1)
  {    
    simpleConnector.logOptions();
    keyHandler.restore();
    exit(1);
  }

  // mandatory init
  Aria::init();
  //ArLog::init(ArLog::StdOut, ArLog::Verbose);

  ArGlobalFunctor1<int> signalHandlerCB(&sigHandlerTest);
  ArSignalHandler::addHandlerCB(&signalHandlerCB, ArListPos::FIRST);

  // let the global aria stuff know about it
  Aria::setKeyHandler(&keyHandler);
  // toss it on the robot
  robot.attachKeyHandler(&keyHandler);

  // add the sonar to the robot
  robot.addRangeDevice(&sonarDev);
  // add the laser to the robot
  robot.addRangeDevice(&sick);

 

  // add a gyro, it'll see if it should attach to the robot or not
  ArAnalogGyro gyro(&robot);

  // set up the robot for connecting
  if (!simpleConnector.connectRobot(&robot))
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    keyHandler.restore();
    return 1;
  }

  simpleConnector.setupLaser(&sick);

  // start the robot running, true so that if we lose connection the run stops
  robot.runAsync(true);

  // set up the laser before handing it to the laser mode
  sick.runAsync();

  // connect the laser if it was requested
  if (!simpleConnector.connectLaser(&sick))
  {
    printf("Could not connect to laser... exiting\n");
    Aria::shutdown();
    keyHandler.restore();
    return 1;
  }
  
  // we need to lock the robot since we'll be setting up these modes
  // while the robot is already running and don't want anything to
  // break
  robot.lock();
  
  // now add all the modes
  ArModeLaser laser(&robot, "laser", 'l', 'L', &sick);
  ArModeTeleop teleop(&robot, "teleop", 't', 'T');
  ArModeUnguardedTeleop unguardedTeleop(&robot, "unguarded teleop", 'u', 'U');
  ArModeWander wander(&robot, "wander", 'w', 'W');
  ArModeGripper gripper(&robot, "gripper", 'g', 'G');
  ArModeCamera camera(&robot, "camera", 'c', 'C');
  ArModeSonar sonar(&robot, "sonar", 's', 'S');
  ArModeBumps bumps(&robot, "bumps", 'b', 'B');
  ArModePosition position(&robot, "position", 'p', 'P', &gyro);
  ArModeIO io(&robot, "io", 'i', 'I');
  ArModeActs actsMode(&robot, "acts", 'a', 'A');

  // activate the default mode
  teleop.activate();

  // turn on the motors
  robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::JOYDRIVE, 1);

  robot.unlock();
  
  robot.waitForRunExit();
  // now exit
  Aria::shutdown();
  return 0;


}

