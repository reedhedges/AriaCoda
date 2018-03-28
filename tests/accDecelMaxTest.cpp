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
  This is a test of the accelerations and decelerations in actions.
  
*/

class Tester : public ArAction
{
public:
  Tester(ArRobot *robot);
  virtual ~Tester() {}
  virtual ArActionDesired *fire(ArActionDesired currentDesired);
  void setRotVelMax(int rotVelMax) { myRotVelMax = rotVelMax; }
  void setRotAcc(int rotAccel) { myRotAcc = rotAccel; }
  void setRotDecel(int rotDecel) { myRotDecel = rotDecel; }
  void setTransVelMax(int transVelMax) { myTransVelMax = transVelMax; }
  void setTransAcc(int transAcc) { myTransAcc = transAcc; }
  void setTransDecel(int transDecel) { myTransDecel = transDecel; }
protected:
  ArActionDesired myActionDesired;
  int myRotVelMax;
  int myRotAcc;
  int myRotDecel;
  int myTransVelMax;
  int myTransAcc;
  int myTransDecel;
};

Tester::Tester(ArRobot *robot) : 
    ArAction("Tester")
{
  ArKeyHandler *keyHandler;
  
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    keyHandler = new ArKeyHandler;
    Aria::setKeyHandler(keyHandler);
    robot->attachKeyHandler(keyHandler);
  }
  myTransVelMax = 0;
  myTransAcc = 0;
  myTransDecel = 0;
  myRotVelMax = 0;
  myRotAcc = 0;
  myRotDecel = 0;
  keyHandler->addKeyHandler('q', new ArFunctor1C<Tester, int>(this, &Tester::setRotVelMax, 0));
  keyHandler->addKeyHandler('w', new ArFunctor1C<Tester, int>(this, &Tester::setRotVelMax, 20));
  keyHandler->addKeyHandler('e', new ArFunctor1C<Tester, int>(this, &Tester::setRotVelMax, 500));

  keyHandler->addKeyHandler('a', new ArFunctor1C<Tester, int>(this, &Tester::setRotAcc, 0));
  keyHandler->addKeyHandler('s', new ArFunctor1C<Tester, int>(this, &Tester::setRotAcc, 8));
  keyHandler->addKeyHandler('d', new ArFunctor1C<Tester, int>(this, &Tester::setRotAcc, 500));

  keyHandler->addKeyHandler('z', new ArFunctor1C<Tester, int>(this, &Tester::setRotDecel, 0));
  keyHandler->addKeyHandler('x', new ArFunctor1C<Tester, int>(this, &Tester::setRotDecel, 8));
  keyHandler->addKeyHandler('c', new ArFunctor1C<Tester, int>(this, &Tester::setRotDecel, 500));

  keyHandler->addKeyHandler('u', new ArFunctor1C<Tester, int>(this, &Tester::setTransVelMax, 0));
  keyHandler->addKeyHandler('i', new ArFunctor1C<Tester, int>(this, &Tester::setTransVelMax, 20));
  keyHandler->addKeyHandler('o', new ArFunctor1C<Tester, int>(this, &Tester::setTransVelMax, 500));

  keyHandler->addKeyHandler('j', new ArFunctor1C<Tester, int>(this, &Tester::setTransAcc, 0));
  keyHandler->addKeyHandler('k', new ArFunctor1C<Tester, int>(this, &Tester::setTransAcc, 8));
  keyHandler->addKeyHandler('l', new ArFunctor1C<Tester, int>(this, &Tester::setTransAcc, 500));

  keyHandler->addKeyHandler('m', new ArFunctor1C<Tester, int>(this, &Tester::setTransDecel, 0));
  keyHandler->addKeyHandler(',', new ArFunctor1C<Tester, int>(this, &Tester::setTransDecel, 8));
  keyHandler->addKeyHandler('.', new ArFunctor1C<Tester, int>(this, &Tester::setTransDecel, 500));

}

ArActionDesired *Tester::fire(ArActionDesired currDes)
{
  printf("%4.0f %4.0f Rot max: %3d acc: %3d dec: %3d Trans max: %3d acc: %3d dec %3d\n",
	 myRobot->getVel(), myRobot->getRotVel(),
	 myRotVelMax, myRotAcc, myRotDecel, myTransVelMax, myTransAcc, 
	 myTransDecel);
  myActionDesired.reset();

  if (myRotVelMax != 0)
    myActionDesired.setMaxRotVel(myRotVelMax);
  if (myRotAcc != 0)
    myActionDesired.setRotAccel(myRotAcc);
  if (myRotDecel != 0)
    myActionDesired.setRotDecel(myRotDecel);
  if (myTransVelMax != 0)
    myActionDesired.setMaxVel(myTransVelMax);
  if (myTransVelMax != 0)
    myActionDesired.setMaxNegVel(myTransVelMax);
  if (myTransAcc != 0)
    myActionDesired.setTransAccel(myTransAcc);
  if (myTransDecel != 0)
    myActionDesired.setTransDecel(myTransDecel);
  
  return &myActionDesired;
}



int main(int argc, char **argv)
{
  // robot
  ArRobot robot;
  
  // the joydrive action
  ArActionJoydrive joydriveAct;
  // the keydrive action
  ArActionKeydrive keydriveAct;
  // Testing action
  Tester tester(&robot);

  // sonar device, so the limiter will work, this must be added to the robot
  ArSonarDevice sonar;

  ArSimpleConnector connector(&argc, argv);
  if (!connector.parseArgs() || argc > 1)
  {
    connector.logOptions();
    exit(1);
  }
  
  // mandatory init
  Aria::init();

  printf("This program will allow you to use a joystick or keyboard to control the robot.\nYou can use the arrow keys to drive, and the spacebar to stop.\nFor joystick control press the trigger button and then drive.\nPress escape to exit.\n");

  // if we don't have a joystick, let 'em know
  if (!joydriveAct.joystickInited())
    printf("Do not have a joystick, only the arrow keys on the keyboard will work.\n");
  
  // set the joystick so it won't do anything if the button isn't pressed
  joydriveAct.setStopIfNoButtonPressed(false);

  // add the sonar to the robot
  robot.addRangeDevice(&sonar);

  // try to connect, if we fail exit
  if (!connector.connectRobot(&robot))
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    return 1;
  }

  // enable the motors
  robot.enableMotors();

  // add the cations, put the limiters on top, then have the action,
  // this will keep the action from being able to drive too fast and hit
  // something
  robot.addAction(&tester, 55);
  //robot.addAction(&joydriveAct, 50);
  robot.addAction(&keydriveAct, 45);

  // set the joydrive action so it'll let the keydrive action fire if
  // there is no button pressed
  joydriveAct.setStopIfNoButtonPressed(false);

  
  // run the robot, true here so that the run will exit if connection lost
  robot.run(true);
  
  // now exit
  Aria::shutdown();
  return 0;
}
