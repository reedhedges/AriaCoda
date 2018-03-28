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

class JoydriveEnc
{
public:
  JoydriveEnc(ArRobot *robot, int num);
  ~JoydriveEnc(void) {}

  void drive(void);

  
  double originEncoder(ArPoseWithTime delta);
  double defaultEncoder(ArPoseWithTime delta);
  double stayEncoder(ArPoseWithTime delta);
  
protected:
  ArJoyHandler myJoyHandler;
  ArRobot *myRobot;
  int myTest;
  time_t myLastPress;
  ArRetFunctor1C<double, JoydriveEnc, ArPoseWithTime> myOriginCB;
  ArRetFunctor1C<double, JoydriveEnc, ArPoseWithTime> myDefaultCB;
  ArRetFunctor1C<double, JoydriveEnc, ArPoseWithTime> myStayCB;
};

JoydriveEnc::JoydriveEnc(ArRobot *robot, int num) :
  myOriginCB(this, &JoydriveEnc::originEncoder),
  myDefaultCB(this, &JoydriveEnc::defaultEncoder),
  myStayCB(this, &JoydriveEnc::stayEncoder)
{
  myTest = num;
  myLastPress = 0;
  myRobot = robot;
  myJoyHandler.init();
  myJoyHandler.setSpeeds(100, 700);

  if (myJoyHandler.haveJoystick())
  {
    printf("Have a joystick\n\n");
  }
  else
  {
    printf("Do not have a joystick, set up the joystick then rerun the program\n\n");
    Aria::shutdown();
    exit(0);    
  }
}

void JoydriveEnc::drive(void)
{
  int trans, rot;
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
    if (myRobot->getEncoderCorrectionCallback() != NULL)
    {
      printf("\nClearing Encoder Correction.\n");
      myRobot->setEncoderCorrectionCallback(NULL);
    }
    else
    {
      if (myTest == 1)
      {
	printf("\nSetting encoder correction so it sits at the origin\n");
	myRobot->setEncoderCorrectionCallback(&myOriginCB);
      }
      else if (myTest == 2)
      {
	printf("\nSetting encodre correction so that it sits in one spot\n");
	myRobot->setEncoderCorrectionCallback(&myStayCB);
      }
      else if (myTest == 3)
      {
	printf("\nSetting encoder correction so that it behaves like the default\n");
	myRobot->setEncoderCorrectionCallback(&myDefaultCB);
      }
      else
	printf("\nNo encoder correction for test %d\n", myTest);
    }
  }
}

double JoydriveEnc::originEncoder(ArPoseWithTime delta)
{
  return 0;
}

double JoydriveEnc::defaultEncoder(ArPoseWithTime delta)
{
  return delta.getTh();
}

double JoydriveEnc::stayEncoder(ArPoseWithTime delta)
{
  return 0;
}

int main(int argc, char **argv)
{
  std::string str;
  int ret;
  int num;

  if (argc != 2)
  {
    printf("Usage is '%s <num>' where <num> is one of:\n", argv[0]);
    printf("1   keep the robot at 0, 0, 0\n");
    printf("2   keep the robbot from moving\n");
    printf("3   let the robot move normally\n");
    exit(0);
  }
  num = atoi(argv[1]);

  ArSerialConnection con;
  ArRobot robot(NULL, false);
  JoydriveEnc joyd(&robot, num);
  ArFunctorC<JoydriveEnc> driveCB(&joyd, &JoydriveEnc::drive);


  Aria::init();
  if ((ret = con.open()) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    Aria::shutdown();
    return 1;
  }

  robot.addUserTask("joydrive", 50, &driveCB);
  robot.setDeviceConnection(&con);
  if (!robot.blockingConnect())
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    return 1;
  }

  robot.comInt(ArCommands::SONAR, 0);
  robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  robot.run(true);
  Aria::shutdown();
  return 0;
}

