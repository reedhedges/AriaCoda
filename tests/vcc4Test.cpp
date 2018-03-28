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
  This is a demo of the Canon VC-C4 Pan/Tilt/Zoom camera.  It uses the same slew for the pan and tilt axes.
It defaults to use the microcontroller port Aux1.  See the setport comments below to use the computer's serial port, or a different aux port on the microcontroller.

This is a slightly modified version of the examples/vcc4Demo.cpp file.  This has an added exercise command, and auto-update is toggleable.


Commands:
_________________

UP,DOWN     -- tilt up/down by 1 degree
LEFT,RIGHT  -- pan left/right by 1 degree
X,C         -- zoom in/out by 10 units (80 total)
I           -- initialize PTU to default settings
>,<         -- increase/decrease the positional increment by one 1 degree
+,-         -- increase/decrease the slew by 5 degrees/sec
Z           -- move pan and tilt axes to zero
H           -- Halt all motion
S           -- Status of camera position and variable values
P           -- Power on/off the camera
ESC         -- Exit program
*/

/*
  This class is the core of this demo, it adds itself to the robot given
  as a user task, then drives the PTZ camera from the keyboard
*/
class KeyPTU
{
public:
  // constructor
  KeyPTU(ArRobot *robot);
  // destructor, its just empty
  ~KeyPTU(void) {}
  
  void up(void);
  void down(void);
  void left(void);
  void right(void);

  void plus(void);
  void minus(void);
  void greater(void);
  void less(void);
  void question(void);
  void status(void);

  void c(void);
  void h(void);
  void i(void);
  void p(void);
  void x(void);
  void z(void);

  // the callback function
  void drive(void);

protected:

  ArTime myExerciseTime;

  int myPanSlew;
  int myTiltSlew;
  int myPosIncrement;
  int mySlewIncrement;
  int myZoomIncrement;

  bool myMonitor;
  bool myInit;
  bool myAbsolute;
  bool myExercise;

  void exercise(void) { myExercise = !myExercise; }
  void autoupdate(void);

  ArFunctorC<KeyPTU> myUpCB;
  ArFunctorC<KeyPTU> myDownCB;
  ArFunctorC<KeyPTU> myLeftCB;
  ArFunctorC<KeyPTU> myRightCB;

  ArFunctorC<KeyPTU> myPlusCB;
  ArFunctorC<KeyPTU> myMinusCB;
  ArFunctorC<KeyPTU> myGreaterCB;
  ArFunctorC<KeyPTU> myLessCB;
  ArFunctorC<KeyPTU> myQuestionCB;
  ArFunctorC<KeyPTU> mySCB;

  ArFunctorC<KeyPTU> myECB;
  ArFunctorC<KeyPTU> myACB;

  ArFunctorC<KeyPTU> myCCB;
  ArFunctorC<KeyPTU> myHCB;
  ArFunctorC<KeyPTU> myICB;
  ArFunctorC<KeyPTU> myPCB;
  ArFunctorC<KeyPTU> myXCB;
  ArFunctorC<KeyPTU> myZCB;

  // the PTU
  ArVCC4 myPTU;

  // the serial connection, in case we are connected to a computer
  // serial port
  ArSerialConnection myCon;

  // whether or not we've requested that the Camera initialize itself
  bool myPTUInitRequested;
  // pointer to the robot
  ArRobot *myRobot;
  // callback for the drive function
  ArFunctorC<KeyPTU> myDriveCB;
};

/*
  Constructor, sets the robot pointer, and some initial values, also note the
  use of constructor chaining on myPTU and myDriveCB.
*/
KeyPTU::KeyPTU(ArRobot *robot) :
  myUpCB(this, &KeyPTU::up),
  myDownCB(this, &KeyPTU::down),
  myLeftCB(this, &KeyPTU::left),
  myRightCB(this, &KeyPTU::right),
  myPlusCB(this, &KeyPTU::plus),
  myMinusCB(this, &KeyPTU::minus),
  myGreaterCB(this, &KeyPTU::greater),
  myLessCB(this, &KeyPTU::less),
  myQuestionCB(this, &KeyPTU::question),
  mySCB(this, &KeyPTU::status),
  myECB(this, &KeyPTU::exercise),
  myACB(this, &KeyPTU::autoupdate),
  myCCB(this, &KeyPTU::c),
  myHCB(this, &KeyPTU::h),
  myICB(this, &KeyPTU::i),
  myPCB(this, &KeyPTU::p),
  myXCB(this, &KeyPTU::x),
  myZCB(this, &KeyPTU::z),

  myPTU(robot),
  myDriveCB(this, &KeyPTU::drive)
{
  // set the robot pointer and add the KeyPTU as user task
  ArKeyHandler *keyHandler;
  myRobot = robot;
  myRobot->addSensorInterpTask("KeyPTU", 50, &myDriveCB);

  myExerciseTime.setToNow();
  myExercise = true;

//  SETPORT Uncomment the following to run the camera off
//  of the computer's serial port, rather than the microcontroller

// uncomment below here
/*
#ifdef WIN32
  myCon.setPort("COM2");
#else
  myCon.setPort("/dev/ttyS0");
#endif
  myPTU.setDeviceConnection(&myCon);
*/
// to here

  // or use this next line to set the aux port 
 //myPTU.setAuxPort(2);


  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    keyHandler = new ArKeyHandler;
    Aria::setKeyHandler(keyHandler);
    myRobot->attachKeyHandler(keyHandler);
  }

  if (!keyHandler->addKeyHandler(ArKeyHandler::UP, &myUpCB))
    ArLog::log(ArLog::Terse, "The key handler already has a key for up, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::DOWN, &myDownCB))
    ArLog::log(ArLog::Terse, "The key handler already has a key for down, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::LEFT, &myLeftCB))
    ArLog::log(ArLog::Terse,  
"The key handler already has a key for left, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::RIGHT, &myRightCB))
    ArLog::log(ArLog::Terse,  
"The key handler already has a key for right, keydrive will not work correctly.");

  if (!keyHandler->addKeyHandler('+', &myPlusCB))
    ArLog::log(ArLog::Terse,
"The key handler already has a key for '+', keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler('-', &myMinusCB))
    ArLog::log(ArLog::Terse,
"The key handler already has a key for '-', keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler('>', &myGreaterCB))
    ArLog::log(ArLog::Terse,
"The key handler already has a key for '>', keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler('<', &myLessCB))
    ArLog::log(ArLog::Terse,
"The key handler already has a key for '<', keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler('?', &myQuestionCB))
    ArLog::log(ArLog::Terse,
"The key handler already has a key for '?', keydrive will not work correctly.");

  if (!keyHandler->addKeyHandler('c', &myCCB))
    ArLog::log(ArLog::Terse,
"The key handler already has a key for 'C', keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler('h', &myHCB))
    ArLog::log(ArLog::Terse,
"The key handler already has a key for 'H', keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler('i', &myICB))
    ArLog::log(ArLog::Terse,
"The key handler already has a key for 'I', keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler('p', &myPCB))
    ArLog::log(ArLog::Terse,
"The key handler already has a key for 'P', keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler('s', &mySCB))
    ArLog::log(ArLog::Terse,
"The key handler already has a key for 'S', keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler('x', &myXCB))
    ArLog::log(ArLog::Terse,
"The key handler already has a key for 'X', keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler('z', &myZCB))
    ArLog::log(ArLog::Terse,
"The key handler already has a key for 'Z', keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler('a', &myACB))
    ArLog::log(ArLog::Terse, "The key handler already has a key for 'A', keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler('e', &myECB))
    ArLog::log(ArLog::Terse, "The key handler already has a key for 'E', keydrive will not work correctly.");



  // initialize some variables

  myPTUInitRequested = false;
  myPosIncrement = 1;
  mySlewIncrement = 5;
  myZoomIncrement = 50;

}

void KeyPTU::autoupdate(void)
{
  if (myPTU.getAutoUpdate())
    myPTU.disableAutoUpdate();
  else
    myPTU.enableAutoUpdate();
}

void KeyPTU::right(void)
{
  myPTU.panRel(myPosIncrement);
}

void KeyPTU::left(void)
{
  myPTU.panRel(-myPosIncrement);
}

void KeyPTU::up(void)
{
  myPTU.tiltRel(myPosIncrement);
}

void KeyPTU::down(void)
{
  myPTU.tiltRel(-myPosIncrement);
}

void KeyPTU::x(void)
{
  myPTU.zoom(myPTU.getZoom() + myZoomIncrement);
}

void KeyPTU::c(void)
{
  myPTU.zoom(myPTU.getZoom() - myZoomIncrement);
}

void KeyPTU::i(void)
{
  myPTU.init();
}

void KeyPTU::plus(void)
{
  myPTU.panSlew(myPTU.getPanSlew() + mySlewIncrement);
  myPTU.tiltSlew(myPTU.getTiltSlew() + mySlewIncrement);

  status();
}

void KeyPTU::minus(void)
{
  myPTU.panSlew(myPTU.getPanSlew() - mySlewIncrement);
  myPTU.tiltSlew(myPTU.getTiltSlew() - mySlewIncrement);

  status();
}

void KeyPTU::greater(void)
{
  myPosIncrement += 1;
  
  if (myPosIncrement > myPTU.getMaxPosPan())			//Use pan range as reference for largest allowable positional increment
    myPosIncrement = myPTU.getMaxPosPan();

  status();
}

void KeyPTU::less(void)
{
  myPosIncrement -= 1;

  if (myPosIncrement < 0)
    myPosIncrement = 0;

  status();
}


void KeyPTU::z(void)
{
  myPTU.panTilt(0,0);
  myPTU.zoom(0);
  status();
}

void KeyPTU::question(void)
{
  ArLog::log(ArLog::Normal, "\r\nCommands:\r\n_________________\r\n");
  ArLog::log(ArLog::Normal, "UP,DOWN     -- tilt up/down by 1 increment");
  ArLog::log(ArLog::Normal, "LEFT,RIGHT  -- pan left/right by 1 increment");
  ArLog::log(ArLog::Normal, "X,C         -- zoom in/out by 50 units (2140 max)");
  ArLog::log(ArLog::Normal, "I           -- initialize PTU to default settings");
  ArLog::log(ArLog::Normal, ">,<         -- increase/decrease the positional increment by 1 degree");
  ArLog::log(ArLog::Normal, "+,-         -- increase/decrease the slew by 5 degrees/sec");
  ArLog::log(ArLog::Normal, "Z           -- move pan and tilt axes to zero");
  ArLog::log(ArLog::Normal, "H           -- Halt all motion");
  ArLog::log(ArLog::Normal, "S           -- Status of camera position and variable values");
  ArLog::log(ArLog::Normal, "P           -- Power on/off the camera");
  ArLog::log(ArLog::Normal, "ESC         -- Exit program");
  ArLog::log(ArLog::Normal, "\r\n");
}

void KeyPTU::status(void)
{
  ArLog::log(ArLog::Normal, "\r\nStatus:\r\n_________________________\r\n");
  ArLog::log(ArLog::Normal, "Pan Position       =  %d deg", myPTU.getPan());
  ArLog::log(ArLog::Normal, "Tilt Position      =  %d deg", myPTU.getTilt());
  ArLog::log(ArLog::Normal, "Zoom Position      =  %d", myPTU.getZoom());
  ArLog::log(ArLog::Normal, "Pan Slew           =  %d deg/s", myPTU.getPanSlew());
  ArLog::log(ArLog::Normal, "Tilt Slew          =  %d deg/s", myPTU.getTiltSlew());
  ArLog::log(ArLog::Normal, "Position Increment =  %d deg", myPosIncrement);
  if (myPTU.getPower())
    ArLog::log(ArLog::Normal, "Power is ON");
  else
    ArLog::log(ArLog::Normal, "Power is OFF");
  ArLog::log(ArLog::Normal, "\r\n");
}

void KeyPTU::h(void)
{
  myPTU.haltPanTilt();
  myPTU.haltZoom();
}

void KeyPTU::p(void)
{
  if (myPTU.getPower())
    myPTU.power(0);
  else
    myPTU.power(1);

  status();
}

// the important function
void KeyPTU::drive(void)
{
  // if the PTU isn't initialized, initialize it here... it has to be 
  // done here instead of above because it needs to be done when the 
  // robot is connected
  if (!myPTUInitRequested && !myPTU.isInitted() && myRobot->isConnected())
  {
    printf("\nWaiting for Camera to Initialize\n");
    myAbsolute = true;
    myPTUInitRequested = true;
    myPTU.init();
  }

  // if the camera hasn't initialized yet, then just return
  if (myPTUInitRequested && !myPTU.isInitted())
  {
    return;
  }

  if (myPTUInitRequested && myPTU.isInitted())
  {
    myPTUInitRequested = false;
    myPanSlew = myPTU.getPanSlew();
    myTiltSlew = myPTU.getTiltSlew();
    printf("Done.\n");
    question();
  }

  if (myExerciseTime.secSince() > 5 && myExercise)
  {
    int pan,tilt;

    if (ArMath::random()%2)
      pan = ArMath::random()%((long) myPTU.getMaxPosPan());
    else
      pan = -ArMath::random()%((long) myPTU.getMaxNegPan());

    if (ArMath::random()%2)
      tilt = ArMath::random()%((long) myPTU.getMaxPosTilt());
    else
      tilt = -ArMath::random()%((long) myPTU.getMaxNegTilt());

    myPTU.panTilt(pan, tilt);
    myExerciseTime.setToNow();
  }

}

int main(int argc, char **argv) 
{
  // just some stuff for returns
  std::string str;
  // robots connection
  ArSerialConnection con;

  // the robot, this turns state reflection off
  ArRobot robot(NULL, false);
  // the joydrive as defined above, this also adds itself as a user task
  KeyPTU ptu(&robot);

  // mandatory init
  Aria::init();

  ArLog::init(ArLog::StdOut, ArLog::Terse, NULL, true);

  con.setPort(ArUtil::COM1);
  // set the connection on the robot
  robot.setDeviceConnection(&con);

  // connect, if we fail, exit
  if (!robot.blockingConnect())
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    return 1;
  }


  // turn off the sonar, enable the motors, turn off amigobot sounds
  robot.comInt(ArCommands::SONAR, 0);
  robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  printf("Press '?' for available commands\r\n");
  // run, if we lose connection to the robot, exit
  robot.run(true);
  
  // shutdown and go away
  Aria::shutdown();
  return 0;
}

