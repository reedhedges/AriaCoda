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


int main(int argc, char** argv)
{
  system("echo joy of joys");
  // mandatory init

  //ArSignalHandler::block(ArSignalHandler::SigABRT);
  Aria::init();
  
  /*
  ArSignalHandler::addHandlerCB(new ArGlobalFunctor1<int>(&Aria::signalHandlerCB), ArListPos::LAST);
  ArSignalHandler::blockCommon();
  ArSignalHandler::block(ArSignalHandler::SigABRT);
  ArSignalHandler::handle(ArSignalHandler::SigHUP);
  ArSignalHandler::handle(ArSignalHandler::SigINT);
  ArSignalHandler::handle(ArSignalHandler::SigQUIT);
  ArSignalHandler::handle(ArSignalHandler::SigTERM);
  ArSignalHandler::handle(ArSignalHandler::SigPIPE);
  ArSignalHandler::blockCommonThisThread();
  ArSignalHandler::createHandlerThreaded();
  */

  //ArUtil::sleep(1000);
  system("echo joy of joys");
  // set up our parser
  ArArgumentParser parser(&argc, argv);
  // set up our simple connector
  ArSimpleConnector simpleConnector(&parser);
  // robot
  ArRobot robot;
  // a laser in case one is used
  ArSick sick;
  // sonar, must be added to the robot, for teleop and wander
  ArSonarDevice sonarDev;


  // load the default arguments 
  parser.loadDefaultArguments();

  // parse the command line... fail and print the help if the parsing fails
  // or if the help was requested
  if (!simpleConnector.parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {    
    simpleConnector.logOptions();
    exit(1);
  }
  
  //ArLog::init(ArLog::StdOut, ArLog::Verbose);

  // if you're using a program at startup in linux you must NOT
  // include a keyhandler or you'll get lockups when trying to read
  // keys with no terminal

  // a key handler so we can do our key handling
  ArKeyHandler keyHandler;
  // let the global aria stuff know about it
  Aria::setKeyHandler(&keyHandler);
  // toss it on the robot
  robot.attachKeyHandler(&keyHandler);
  printf("You may press escape to exit\n");

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
    Aria::exit(1);
  }

  // start the robot running, true so that if we lose connection the run stops
  robot.runAsync(true);

  // set up the laser before handing it to the laser mode
  sick.runAsync();

  // connect the laser if it was requested
  if (!simpleConnector.connectLaser(&sick))
  {
    printf("Could not connect to laser... exiting\n");
    Aria::exit(2);
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
  ArModeCommand command(&robot, "command", 'd', 'D');
  ArModeTCM2 tcm2(&robot, "tcm2", 'm', 'M');

  // activate the default mode
  teleop.activate();

  // turn on the motors
  robot.comInt(ArCommands::ENABLE, 1);
  //robot.comInt(ArCommands::JOYDRIVE, 1);

  robot.unlock();
  
  while (robot.isRunning() && Aria::getRunning())
  {
    system("echo starting; sleep 3; echo done");
    ArUtil::sleep(3000);
  }
  
  // now exit
  Aria::exit(0);


}

