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
#include <time.h>

class Joydrive
{
public:
  Joydrive(ArRobot *robot, int test);
  ~Joydrive(void) {}

  void drive(void);

protected:
  ArJoyHandler myJoyHandler;
  ArRobot *myRobot;
  int myTest;
  time_t myLastPress;
};

Joydrive::Joydrive(ArRobot *robot, int test)
{
  myRobot = robot;
  myJoyHandler.init();
  myJoyHandler.setSpeeds(100, 700);
  
  myTest = test;
  myLastPress = 0;
  if (myJoyHandler.haveJoystick())
  {
    printf("Have a joystick\n\n");
  }
  else
  {
    printf("Do not have a joystick, set up the joystick then rerun the program\n\n");
    exit(0);    
  }
}

void Joydrive::drive(void)
{
  int trans, rot;
  ArPose pose;
  ArPose rpose;
  ArTransform transform;
  ArRangeDevice *dev;
  ArSensorReading *son;

  if (!myRobot->isConnected())
  {
    printf("Lost connection to the robot, exiting\n");
    exit(0);
  }
  printf("\rx %6.1f  y %6.1f  th  %6.1f", 
	 myRobot->getX(), myRobot->getY(), myRobot->getTh());
  fflush(stdout);
  if (myJoyHandler.haveJoystick() && myJoyHandler.getButton(1))
  {
    if (ArMath::fabs(myRobot->getVel()) < 10.0)
      myRobot->comInt(ArCommands::ENABLE, 1);
    myJoyHandler.getAdjusted(&rot, &trans);
    myRobot->setVel(trans);
    myRobot->setRotVel(-rot);
  }
  else
  {
    myRobot->setVel(0);
    myRobot->setRotVel(0);
  }
  if (myJoyHandler.haveJoystick() && myJoyHandler.getButton(2) &&
      time(NULL) - myLastPress > 1)
  {
    myLastPress = time(NULL);
    printf("\n");
    switch (myTest)
    {
    case 1:
      printf("Moving back to the origin.\n");
      pose.setPose(0, 0, 0);
      myRobot->moveTo(pose);
      break;
    case 2:
      printf("Moving over a meter.\n");
      pose.setPose(myRobot->getX() + 1000, myRobot->getY(), 0);
      myRobot->moveTo(pose);
      break;
    case 3:
      printf("Doing a transform test....\n");
      printf("\nOrigin should be transformed to the robots coords.\n");
      transform = myRobot->getToGlobalTransform();
      pose.setPose(0, 0, 0);
      pose = transform.doTransform(pose);
      rpose = myRobot->getPose();
      printf("Pos:  ");
      pose.log();
      printf("Robot:  ");
      rpose.log();

      if (pose.findDistanceTo(rpose) < .1)
	printf("Success\n");
      else
	printf("#### FAILURE\n");
    
      printf("\nRobot coords should be transformed to the origin.\n");
      transform = myRobot->getToLocalTransform();
      pose = myRobot->getPose();
      pose = transform.doTransform(pose);
      rpose.setPose(0, 0, 0);
      printf("Pos:  ");
      pose.log();
      printf("Robot:  ");
      rpose.log();
      if (pose.findDistanceTo(rpose) < .1)
	printf("Success\n");
      else
	printf("#### FAILURE\n");
      break;
    case 4:
      printf("Doing a tranform test...\n");
      printf("A point 1 meter to the -x from the robot (in local coords) should be transformed into global coordinates.\n");
      transform = myRobot->getToGlobalTransform();
      pose.setPose(-1000, 0, 0);
      pose = transform.doTransform(pose);
      rpose = myRobot->getPose();
      printf("Pos:  ");
      pose.log();
      printf("Robot:  ");
      rpose.log();

      if (ArMath::fabs(pose.findDistanceTo(rpose) - 1000.0) < .1)
	printf("Probable Success\n");
      else
	printf("#### FAILURE\n");
      break;
    case 5:
      printf("Doing a transform test on range devices..\n");
      printf("Moving the robot +4 meters x and +4 meters y and seeing if the moveTo will move the sonar readings along with it.\n");
      dev = myRobot->findRangeDevice("sonar");
      if (dev == NULL)
      {
	printf("No sonar on the robot, can't do the test.\n");
	break;
      }
      printf("Closest sonar reading to the robot is %.0f away\n", dev->currentReadingPolar(1, 0));
      printf("Sonar 0 reading is at ");
      son = myRobot->getSonarReading(0);
      if (son != NULL)
      {
	pose = son->getPose();
	pose.log();
      }
      pose = myRobot->getPose();
      pose.setX(pose.getX() + 4000);
      pose.setY(pose.getY() + 4000);
      myRobot->moveTo(pose);
      printf("Moved robot.\n");
      printf("Closest sonar reading to the robot is %.0f away\n", dev->currentReadingPolar(1, 0));
      printf("Sonar 0 reading is at ");
      son = myRobot->getSonarReading(0);
      if (son != NULL)
      {
	pose = son->getPose();
	pose.log();
      }

      break;
    case 6:
      printf("Robot position now is:\n");
      pose = myRobot->getPose();
      pose.log();
      printf("Disconnecting from the robot, then reconnecting.\n");
      myRobot->disconnect();
      myRobot->blockingConnect();      
      printf("Robot position now is:\n");
      pose = myRobot->getPose();
      pose.log();
      break;
    default:
      printf("No test for second button.\n");
      break;
    } 
  }
}

int main(int argc, char **argv) 
{
  std::string str;
  int ret;
  int num;
  ArPose pose;

  Aria::init();

  if (argc != 2)
  {
    printf("Usage is '%s <num>' where <num> is one of:\n", argv[0]);
    printf("1     Moves the robot back to the origin.\n");
    printf("2     Moves the robot +1 meter in X direction.\n");
    printf("3     Does a transform test, going from the origin to robot and back.\n");
    printf("4     Does a transform test with a point 1 meter to the -x from the robot (ie left).\n");
    printf("5     Does a transform, prints out sonar plar reading before and after.\n");
    printf("6     Disconnects the robot, and reconnects to it.\n");
    exit(1);
  }

  num = atoi(argv[1]);
  ArTcpConnection con;
  ArRobot robot(NULL, false);
  Joydrive joyd(&robot, num);
  ArFunctorC<Joydrive> driveCB(&joyd, &Joydrive::drive);
  ArSonarDevice sonar;

  if ((ret = con.open()) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    Aria::shutdown();
    return 1;
  }

  robot.addUserTask("joydrive", 50, &driveCB);
  robot.addRangeDevice(&sonar);
  robot.setDeviceConnection(&con);
  pose.setPose(4000, 2000, 90);
  robot.moveTo(pose);
  pose = robot.getPose();
  printf("Position set to, before connect.\n");
  pose.log();
  if (!robot.blockingConnect())
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    return 1;
  }
  pose = robot.getPose();
  printf("Position robot at, after connect.\n");
  pose.log();
  //printf("Position being reset to 0.\n");
  //pose.setPose(0, 0, 0);
  //robot.moveTo(pose);
  //robot.comInt(ArCommands::SONAR, 0);
  robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  robot.run(true);
  Aria::shutdown();
  return 0;
}

