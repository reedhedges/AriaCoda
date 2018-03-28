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
/** @example gyroExample.cpp Example program that activates an onboard gyro if it exists and uses its data to correct the robot pose. 
 *
 * This program uses ArActionKeydrive and ArActionJoydrive to allow
 * teleoperation with the keyboard or joystick, and displays gyro data.
 * Additional keys (numbers 0-9 and letters q, w, e, r, t, y, u, i, o, p) 
 * activate preset rotation velocities.
 *
*/

#include "Aria.h"
#include "ArAnalogGyro.h"


class GyroTask
{
public:
  // the constructor, it must use constructor chaining to intialize its base
  // class ArSimpleUserTask
  GyroTask(ArRobot *robot);
  // empty destructor
  ~GyroTask(void) {}
  
  // the task we want to do
  void doTask(void);

  bool handlePacket(ArRobotPacket *pkt);
protected:
  //double myHeading;
  ArAnalogGyro *myGyro;
  ArRobot *myRobot;
  ArFunctorC<GyroTask> myTaskCB;
  ArRetFunctor1C<bool, GyroTask, ArRobotPacket*> myPacketHandlerCB;
  bool gotGyroPacket;
};


// the constructor, note how it uses chaining to initialize the myTaskCB
GyroTask::GyroTask(ArRobot *robot) :
  myTaskCB(this, &GyroTask::doTask),
  myPacketHandlerCB(this, &GyroTask::handlePacket),
  gotGyroPacket(false)
{
  ArKeyHandler *keyHandler;
  myRobot = robot;
  // just add it to the robot
  myRobot->addUserTask("GyroTask", 50, &myTaskCB);
  myRobot->addPacketHandler(&myPacketHandlerCB, ArListPos::LAST);
  myRobot->comInt(ArCommands::GYRO, 2);
  myGyro = new ArAnalogGyro(myRobot);
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    keyHandler = new ArKeyHandler;
    Aria::setKeyHandler(keyHandler);
    if (myRobot != NULL)
      myRobot->attachKeyHandler(keyHandler);
    else
      ArLog::log(ArLog::Terse, "GyroTask: No robot to attach a keyHandler to, keyHandling won't work... either make your own keyHandler and drive it yourself, make a keyhandler and attach it to a robot, or give this a robot to attach to.");
  }  
  keyHandler->addKeyHandler('1', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, 10));
  keyHandler->addKeyHandler('2', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, 20));
  keyHandler->addKeyHandler('3', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, 30));
  keyHandler->addKeyHandler('4', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, 40));
  keyHandler->addKeyHandler('5', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, 50));
  keyHandler->addKeyHandler('6', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, 60));
  keyHandler->addKeyHandler('7', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, 70));
  keyHandler->addKeyHandler('8', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, 80));
  keyHandler->addKeyHandler('9', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, 90));
  keyHandler->addKeyHandler('0', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, 100));

  keyHandler->addKeyHandler('q', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, -10));
  keyHandler->addKeyHandler('w', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, -20));
  keyHandler->addKeyHandler('e', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, -30));
  keyHandler->addKeyHandler('r', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, -40));
  keyHandler->addKeyHandler('t', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, -50));
  keyHandler->addKeyHandler('y', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, -60));
  keyHandler->addKeyHandler('u', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, -70));
  keyHandler->addKeyHandler('i', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, -80));
  keyHandler->addKeyHandler('o', new ArFunctor1C<ArRobot, double>(myRobot,&ArRobot::setRotVel, -90));
  keyHandler->addKeyHandler('p', new ArFunctor1C<ArRobot, double>(myRobot, &ArRobot::setRotVel, -100));

  keyHandler->addKeyHandler('a', new ArFunctor1C<ArRobot, double>(myRobot, &ArRobot::setHeading, 0));
  keyHandler->addKeyHandler('s', new ArFunctor1C<ArRobot, double>(myRobot, &ArRobot::setHeading, 90));
  keyHandler->addKeyHandler('d', new ArFunctor1C<ArRobot, double>(myRobot, &ArRobot::setHeading, 180));
  keyHandler->addKeyHandler('f', new ArFunctor1C<ArRobot, double>(myRobot, &ArRobot::setHeading, 270));

}

void GyroTask::doTask(void)
{
  /*
  double degrees = -((myRobot->getAnalog() * 5.0 / 255) - 2.509) * 150 / 2.5 * 1.265;
  if (fabs(degrees) < 2)
    degrees = 0;
  myHeading += degrees * .025;
  printf("%10f %10f %10f %10f\n", myRobot->getAnalog() * 5.0 / 255, degrees,
	 myRobot->getRotVel(), myHeading);
  fflush(stdout);
  */
  printf("gyro th (mode 1 only):%8.4f  encoder th:%8.4f   ArRobot mixed th:%8.4f  temp:%d  ave:%g  gyro packets:%s\n", myGyro->getHeading(), myRobot->getRawEncoderPose().getTh(), myRobot->getTh(), myGyro->getTemperature(), myGyro->getAverage(), gotGyroPacket?"received":"not received");
}


bool GyroTask::handlePacket(ArRobotPacket *pkt)
{
	if(pkt->getID() == 152) 
		gotGyroPacket = true;
	return true;
}


int main(int argc, char **argv)
{
  Aria::init();
  ArRobot robot;
  
  // the joydrive action
  ArActionJoydrive joydriveAct;
  // the keydrive action
  ArActionKeydrive keydriveAct;

  GyroTask gyro(&robot);

  // sonar device, so the limiter will work, this must be added to the robot
  ArSonarDevice sonar;

  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobotConnector connector(&parser, &robot);
  if (!Aria::parseArgs() || argc > 1)
  {
    Aria::logOptions();
    Aria::exit(1);
    return 1;
  }
  

  printf("This program will allow you to use a joystick or keyboard to control the robot.\nYou can use the arrow keys to drive, and the spacebar to stop.\nFor joystick control press the trigger button and then drive.\nPress escape to exit.\n");


  // if we don't have a joystick, let 'em know
  if (!joydriveAct.joystickInited())
    printf("Do not have a joystick, only the arrow keys on the keyboard will work.\n");
  
  // set the joystick so it won't do anything if the button isn't pressed
  joydriveAct.setStopIfNoButtonPressed(false);

  // add the sonar to the robot
  robot.addRangeDevice(&sonar);

  // try to connect, if we fail exit
  if (!connector.connectRobot())
  {
    printf("Could not connect to robot... exiting\n");
    Aria::exit(1);
    return 1;
  }

  robot.comInt(ArCommands::ENABLE, 1);

  robot.addAction(&joydriveAct, 50);
  robot.addAction(&keydriveAct, 45);

  // set the joydrive action so it'll let the keydrive action fire if
  // there is no button pressed
  joydriveAct.setStopIfNoButtonPressed(false);

  
  // run the robot, true here so that the run will exit if connection lost
  robot.run(true);
  
  // now exit
  Aria::exit(0);
  return 0;
}
