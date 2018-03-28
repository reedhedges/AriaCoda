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

int main(int argc, char **argv)
{
  bool done;

  bool shouldSpin = true;
  int spinTime = 10000;
  double distToTravel = 1000;

  double rotVelMax = 50;
  double rotAccel = 100;
  double rotDecel = 100;
  double transVelMax = 1500;
  double transAccel = 600;
  double transDecel = 600;

  Aria::init();


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


  parser.addDefaultArgument("-connectLaser -laserIncrement half");

  // load the default arguments 
  parser.loadDefaultArguments();

  // parse the command line... fail and print the help if the parsing fails
  // or if the help was requested
  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {    
    Aria::logOptions();
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

  robot.setRotVelMax(rotVelMax);
  robot.setRotAccel(rotAccel);
  robot.setRotDecel(rotDecel);
  robot.setTransVelMax(transVelMax);
  robot.setTransAccel(transAccel);
  robot.setTransDecel(transDecel);

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

  ArUtil::sleep(300);

  robot.lock();

  // set a default filename
  //std::string filename = "c:\\log\\1scans.2d";
  std::string filename = "1scans.2d";
  // see if we want to use a different filename
  //if (argc > 1)
  printf("Logging to file %s\n", filename.c_str());
  // start the logger with good values
  ArSickLogger logger(&robot, &sick, 300, 25, filename.c_str());

  //Make a couple action groups, one for driving in a straight line, one for turning
  
  ArActionGroup drive(&robot);
  ArActionDriveDistance driveAction("driveDistance", transVelMax, transDecel);
  ArActionDeceleratingLimiter limiterAction;
  limiterAction.setParameters(200, 100, 100, 0, 100, 100, 2000, 1000, true, 0);

  drive.addAction(&driveAction, 50);
  drive.addAction(&limiterAction, 75);

  ArActionGroup spin(&robot);
  ArActionInput spinAction;
  
  spin.addAction(&spinAction, 100);

		  

  robot.unlock();

#ifdef WIN32
  // wait until someone pushes the motor button to go (since vnc hoses
  // timing)
  while (1)
  {
    robot.lock();
    if (!robot.isRunning())
      exit(0);
    if (robot.areMotorsEnabled())
    {
      robot.unlock();
      break;
    }
    robot.unlock();
    ArUtil::sleep(100);
  }
#endif

  // basically from here on down the robot just cruises around a bit

  robot.lock();
  // enable the motors
  robot.comInt(ArCommands::ENABLE, 1);

  ArTime startTime;

  ArLog::log(ArLog::Normal, "Driving out");
  // move a couple meters
  drive.activateExclusive();
  driveAction.setDistance(distToTravel);
  robot.unlock();
  done = false;
  startTime.setToNow();
  do {
    ArUtil::sleep(100);
    robot.lock();
    done = (driveAction.haveAchievedDistance() || startTime.secSince() > 10 || 
	    (startTime.secSince() > 2 && fabs(robot.getVel() < 2)));
    robot.unlock();
  } while (!done);


  if (shouldSpin)
  {
    ArLog::log(ArLog::Normal, "Spinning counter clockwise");
    robot.lock();
    spin.activateExclusive();
    spinAction.setRotVel(rotVelMax);
    spinAction.setVel(0);
    robot.unlock();
    ArUtil::sleep(spinTime);
    ArLog::log(ArLog::Normal, "Spinning clockwise");
    robot.lock();
    spin.activateExclusive();
    spinAction.setRotVel(-rotVelMax);
    spinAction.setVel(0);
    robot.unlock();
    ArUtil::sleep(spinTime);
  }

  ArLog::log(ArLog::Normal, "Pointing back");
  robot.lock();
  spin.activateExclusive();
  spinAction.setHeading(180);
  robot.unlock();
  done = false;
  startTime.setToNow();
  do {
    ArUtil::sleep(100);
    robot.lock();
    done = (fabs(ArMath::subAngle(robot.getTh(), 180)) < 2 && 
	    fabs(robot.getRotVel() < 2)) || startTime.secSince() > 10;
    robot.unlock();
  } while (!done);

  ArLog::log(ArLog::Normal, "Driving back");
  robot.lock();
  drive.activateExclusive();
  driveAction.setDistance(distToTravel);
  robot.unlock();
  done = false;
  startTime.setToNow();
  do {
    ArUtil::sleep(100);
    robot.lock();
    done = (driveAction.haveAchievedDistance() || startTime.secSince() > 10 || 
	    (startTime.secSince() > 2 && fabs(robot.getVel() < 2)));
    robot.unlock();
  } while (!done);


  ArLog::log(ArLog::Normal, "Pointing out");
  robot.lock();
  spin.activateExclusive();
  spinAction.setHeading(0);
  robot.unlock();
  done = false;
  startTime.setToNow();
  do {
    ArUtil::sleep(100);
    robot.lock();
    done = (fabs(ArMath::subAngle(robot.getTh(), 0)) < 2 && 
	    fabs(robot.getRotVel() < 2)) || startTime.secSince() > 10;
    robot.unlock();
  } while (!done);

  ArLog::log(ArLog::Normal, "Done");
  sick.lockDevice();
  sick.disconnect();
  sick.unlockDevice();
  robot.lock();
  robot.disconnect();
  robot.unlock();
  // now exit
  Aria::shutdown();
  return 0;
}

