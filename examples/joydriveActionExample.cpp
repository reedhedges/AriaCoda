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

/** @example joydriveActionExample.cpp Example ArAction object that uses
 * ArJoyHandler to get joystick input.
 *
  This program just drives the robot around with a joystick,
  by way of an ArAction class which reads data fram an ArJoyHandler.
  Its a practical example of actions.
*/

// the action which will drive the robot
class JoydriveAction : public ArAction
{
public:
  // constructor
  JoydriveAction(void);
  // empty destructor
  virtual ~JoydriveAction(void);
  //the fire which will actually tell the resolver what to do
  virtual ArActionDesired *fire(ArActionDesired currentDesired);
  // whether the joystick is initalized or not
  bool joystickInited(void);
protected:
  // action desired
  ArActionDesired myDesired;
  // joystick handler
  ArJoyHandler myJoyHandler;
};

/*
  Note the use of constructor chaining with ArAction.
*/  
JoydriveAction::JoydriveAction(void) :
  ArAction("Joydrive Action", "This action reads the joystick and sets the translational and rotational velocity based on this.")
{
  // initialize the joystick
  myJoyHandler.init();
  // set up the speed parameters on the joystick
  myJoyHandler.setSpeeds(50, 700);
}

JoydriveAction::~JoydriveAction(void)
{
}

// whether the joystick is there or not
bool JoydriveAction::joystickInited(void)
{
  return myJoyHandler.haveJoystick();
}

// the guts of the thing
ArActionDesired *JoydriveAction::fire(ArActionDesired currentDesired)
{
  int rot, trans;

  // print out some info about hte robot
  printf("\rx %6.1f  y %6.1f  tth  %6.1f vel %7.1f mpacs %3d", myRobot->getX(),
	 myRobot->getY(), myRobot->getTh(), myRobot->getVel(), 
	 myRobot->getMotorPacCount());
  fflush(stdout);

  // see if one of the buttons is pushed, if so drive
  if (myJoyHandler.haveJoystick() && (myJoyHandler.getButton(1) ||
				    myJoyHandler.getButton(2)))
  {
    // get the readings from the joystick
    myJoyHandler.getAdjusted(&rot, &trans);
    // set what we want to do
    myDesired.setVel(trans);
    myDesired.setDeltaHeading(-rot);
    // return the actionDesired
    return &myDesired;
  }
  else
  {
    // set what we want to do
    myDesired.setVel(0);
    myDesired.setDeltaHeading(0);
    // return the actionDesired
    return &myDesired;
  }
}


int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  ArRobotConnector robotConnector(&parser, &robot);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "joydriveActionExample: Could not connect to the robot.");
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

  ArLog::log(ArLog::Normal, "joydriveActionExample: Connected to robot.");

  // Instance of the JoydriveAction class defined above
  JoydriveAction jdAct;

  // if the joydrive action couldn't find the joystick, then exit.
  if (!jdAct.joystickInited())
  {
    printf("Do not have a joystick, set up the joystick then rerun the program\n\n");
    Aria::exit(1);
    return 1;
  }
  

  // disable sonar, enable motors, disable amigobot sound
  robot.comInt(ArCommands::SONAR, 0);
  robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  // add the action
  robot.addAction(&jdAct, 100);

  // run the robot, true so it'll exit if we lose connection
  robot.run(true);
  
  // now exit program
  Aria::exit(0);
  return 0;
}

