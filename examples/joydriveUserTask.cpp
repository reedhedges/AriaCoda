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

/*
  This program just drives the robot around with a joystick.

  This example uses a user task which has a joystick handler to drive the
  robot.
*/

// the clas which'll have the task to drive the robot
class Joydrive
{
public:
  // the constructor
  Joydrive(ArRobot *robot);
  // the destructor
  ~Joydrive(void);

  // the function which'll drive the robot
  void drive(void);

protected:
  // the joystick handler
  ArJoyHandler myJoyHandler;
  // robot pointer
  ArRobot *myRobot;
  // the callback
  ArFunctorC<Joydrive> myDriveCB;
};

/*
  the constructor, note the use of constructor chaining
*/
Joydrive::Joydrive(ArRobot *robot) :
  myDriveCB(this, &Joydrive::drive)
{
  // set the robot, and add joydrive as a task
  myRobot = robot;
  if (myRobot != NULL)
    myRobot->addUserTask("joydrive", 50, &myDriveCB);

  // initialize the joystick handler
  myJoyHandler.init();
  // set the values we'll get back out of the joystick handler
  myJoyHandler.setSpeeds(100, 700);

  // see if we have the joystick, and let the user know
  if (myJoyHandler.haveJoystick())
  {
    printf("Have a joystick\n\n");
  }
  // we don't have a joystick, exit
  else
  {
    printf("Do not have a joystick, set up the joystick then rerun the program\n\n");
    Aria::exit(1);
  }
}

Joydrive::~Joydrive(void)
{
  // remove the user task from the robot
  if (myRobot != NULL)
    myRobot->remUserTask(&myDriveCB);
}

void Joydrive::drive(void)
{
  int trans, rot;

  // print out some data about the robot
  printf("\rx %6.1f  y %6.1f  tth  %6.1f vel %7.1f mpacs %3d   ", 
	 myRobot->getX(), myRobot->getY(), myRobot->getTh(), 
	 myRobot->getVel(), myRobot->getMotorPacCount());
  fflush(stdout);

  // see if a joystick butotn is pushed, if so drive
  if (myJoyHandler.haveJoystick() && (myJoyHandler.getButton(1) ||
				    myJoyHandler.getButton(2)))
  {
    // get the values out of the joystick handler
    myJoyHandler.getAdjusted(&rot, &trans);
    // drive the robot
    myRobot->setVel(trans);
    myRobot->setRotVel(-rot);
  }
  // if a button isn't pushed, stop the robot
  else
  {
    myRobot->setVel(0);
    myRobot->setRotVel(0);
  }
}

int main(int argc, char **argv) 
{
  std::string str;
  int ret;
  
  // connection
  ArTcpConnection con;
  // robot, this starts it with state reflecting turned off
  ArRobot robot(NULL, false);
  // make the joydrive instance, which adds its task to the robot
  Joydrive joyd(&robot);

  // mandatory aria initialization
  Aria::init();
  
  // open the connection, if it fails, exit
  if ((ret = con.open()) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    Aria::exit(1);
    return 1;
  }

  // set the robots connection
  robot.setDeviceConnection(&con);
  // try to connect, if we fail exit
  if (!robot.blockingConnect())
  {
    printf("Could not connect to robot... exiting\n");
    Aria::exit(1);
    return 1;
  }

  // turn off sonar, turn the motors on, turn off amigobot sound
  robot.comInt(ArCommands::SONAR, 0);
  robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  // run the robot, true so that if connection is lost the run stops
  robot.run(true);
  // now exit
  Aria::exit(0);   // exit program
  return 0;
}

