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

/** @example gripperExample.cpp Example program demonstrating use of the Pioneer gripper.
 *
 *  Shows how to control the Pioneer 2DOF gripper accessory.
 *  In addition to the arrow keys to teleoperate the robot,
 *  Use the following keyboard keys to control it:
 *    u     Lift the gripper up
 *    d     Lift the gripper down
 *    o     Open the gripper
 *    c     Close the gripper
 *    s     Stop gripper movement.
 *
 *
 *  @sa ArModeGripper (the gripper control mode used in the "demo" program)
 *
*/

#include "Aria.h"

// Adds robot callback to print gripper status.
class PrintGripStatus
{
  ArGripper* myGripper;
  ArFunctorC<PrintGripStatus> myPrintCB;
public:
  PrintGripStatus(ArGripper* gripper) : 
    myGripper(gripper),
    myPrintCB(this, &PrintGripStatus::printStatus)
  {
  }

  void addRobotTask(ArRobot* robot)
  {
    robot->addUserTask("PrintGripStatus", 10, &myPrintCB);
  }

  void printStatus()
  {
    myGripper->logState();
  }
};

// Adds key handler callbacks for controlling the gripper
class GripperControlHandler
{
  ArGripper* myGripper;
  ArFunctorC<GripperControlHandler> myUpCB;
  ArFunctorC<GripperControlHandler> myDownCB;
  ArFunctorC<GripperControlHandler> myOpenCB;
  ArFunctorC<GripperControlHandler> myCloseCB;
  ArFunctorC<GripperControlHandler> myStopCB;
public:
  GripperControlHandler(ArGripper* gripper) : 
    myGripper(gripper),
    myUpCB(this, &GripperControlHandler::liftUp),
    myDownCB(this, &GripperControlHandler::liftDown),
    myOpenCB(this, &GripperControlHandler::open),
    myCloseCB(this, &GripperControlHandler::close),
    myStopCB(this, &GripperControlHandler::stop)
  {
  }

  void addKeyHandlers(ArRobot *robot)
  {
    ArKeyHandler *keyHandler = Aria::getKeyHandler();
    if(keyHandler == NULL)
    {
      keyHandler = new ArKeyHandler();
      Aria::setKeyHandler(keyHandler);
      robot->attachKeyHandler(keyHandler);
    }
    keyHandler->addKeyHandler(ArKeyHandler::PAGEUP, &myUpCB);
    keyHandler->addKeyHandler('u', &myUpCB);
    keyHandler->addKeyHandler(ArKeyHandler::PAGEDOWN, &myDownCB);
    keyHandler->addKeyHandler('d', &myDownCB);
    keyHandler->addKeyHandler('o', &myOpenCB);
    keyHandler->addKeyHandler('c', &myCloseCB);
    keyHandler->addKeyHandler('s', &myStopCB);
  }

  void liftUp()
  {
    ArLog::log(ArLog::Normal, "Moving gripper lift up...");
    myGripper->liftUp();
  }

  void liftDown()
  {
    ArLog::log(ArLog::Normal, "Moving gripper lift down...");
    myGripper->liftDown();
  }

  void stop()
  {
    ArLog::log(ArLog::Normal, "Stopping gripper...");
    myGripper->gripperHalt(); // stops both lift an grip
    //myGripper->liftStop(); // stops just the lift
    //myGripper->gripStop(); // stops just the gripper
  }

  void close()
  {
    ArLog::log(ArLog::Normal, "Closing gripper...");
    myGripper->gripClose();
  }

  void open()
  {
    ArLog::log(ArLog::Normal, "Opening gripper...");
    myGripper->gripOpen();
  }

};

int main(int argc, char **argv) 
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;
  ArGripper gripper(&robot);

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  ArRobotConnector robotConnector(&parser, &robot);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "gripperExample: Could not connect to the robot.");
    if(parser.checkHelpAndWarnUnparsed())
    {
        // -help not given
        Aria::logOptions();
        Aria::exit(1);
    }
  }

  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(1);
  }

  ArLog::log(ArLog::Normal, "gripperExample: Connected to robot.");


  ArLog::log(ArLog::Normal, "gripperExample: GripperType=%d", gripper.getType());
  gripper.logState();
  if(gripper.getType() == ArGripper::NOGRIPPER)
  {
    ArLog::log(ArLog::Terse, "gripperExample: Error: Robot does not have a gripper. Exiting.");
    Aria::exit(-1);
    return -1;
  }

  // Teleoperation actions with obstacle-collision avoidance
  ArActionLimiterTableSensor tableLimit;
  robot.addAction(&tableLimit, 110);
  ArActionLimiterForwards limitNearAction("near", 300, 600, 250);
  robot.addAction(&limitNearAction, 100);
  ArActionLimiterForwards limitFarAction("far", 300, 1100, 400);
  robot.addAction(&limitFarAction, 90);
  ArActionLimiterBackwards limitBackAction;
  robot.addAction(&limitBackAction, 50);
  ArActionJoydrive joydriveAction("joydrive", 400, 15);
  robot.addAction(&joydriveAction, 40);
  joydriveAction.setStopIfNoButtonPressed(false);
  ArActionKeydrive keydriveAction;
  robot.addAction(&keydriveAction, 30);
  

  // Handlers to control the gripper and print out info (classes defined above)
  PrintGripStatus printStatus(&gripper);
  GripperControlHandler gripControl(&gripper);
  printStatus.addRobotTask(&robot);
  gripControl.addKeyHandlers(&robot);

  // enable motors and run (if we lose connection to the robot, exit)
  ArLog::log(ArLog::Normal, "You may now operate the robot with arrow keys or joystick. Operate the gripper with the u, d, o, c, and page up/page down keys.");
  robot.enableMotors();
  robot.run(true);
  
  Aria::exit(0);
  return 0;
}


