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
#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArMode.h"
#include "ArModes.h"
#include "ArKeyHandler.h"
#include "ArSonyPTZ.h"
#include "ArVCC4.h"
#include "ArDPPTU.h"
#include "ArAMPTU.h"
#include "ArRVisionPTZ.h"
//#include "ArSick.h"
#include "ArLaser.h"
#include "ArAnalogGyro.h"
#include "ArRobotConfigPacketReader.h"
#include "ArBatteryMTX.h"
#include "ariaInternal.h"

/// return unsigned byte as string of 8 '1' and '0' characters (MSB first, so
/// bit 0 will be last character in string, bit 7 will be first character.)
/// @todo generalize with others to any number of bits
std::string byte_as_bitstring(unsigned char byte) 
{
  char tmp[9];
  int bit; 
  int ch;
  for(bit = 7, ch = 0; bit >= 0; bit--,ch++)
    tmp[ch] = ((byte>>bit)&1) ? '1' : '0';
  tmp[8] = 0;
  return std::string(tmp);
}

/// return unsigned 16-bit value as string of 16 '1' and '0' characters (MSB first, so
/// bit 0 will be last character in string, bit 15 will be first character.)
/// @todo separate every 8 bits. 
/// @todo generalize with others to any number of bits
std::string int16_as_bitstring(ArTypes::Byte2 n) 
{
  char tmp[17];
  int bit;
  int ch;
  for(bit = 15, ch = 0; bit >= 0; bit--, ch++)
    tmp[ch] = ((n>>bit)&1) ? '1' : '0';
  tmp[16] = 0;
  return std::string(tmp);
}

/// @todo separate every 8 bits. 
/// @todo generalize with others to any number of bits
std::string int32_as_bitstring(ArTypes::Byte4 n)
{
  char tmp[33];
  int bit;
  int ch;
  for(bit = 31, ch = 0; bit >= 0; bit--, ch++)
    tmp[ch] = ((n>>bit)&1) ? '1' : '0';
  tmp[32] = 0;
  return std::string(tmp);
}

/** 
  @param robot ArRobot instance to be associate with
  @param name name of this mode
  @param key keyboard key that activates this mode
  @param key2 another keyboard key that activates this mode
*/
AREXPORT ArModeTeleop::ArModeTeleop(ArRobot *robot, const char *name, char key, char key2): 
  ArMode(robot, name, key, key2),
  myGroup(robot),
  myEnableMotorsCB(robot, &ArRobot::enableMotors)
{
  myGroup.deactivate();
}

AREXPORT ArModeTeleop::~ArModeTeleop()
{
  
}

AREXPORT void ArModeTeleop::activate(void)
{
  if (!baseActivate())
    return;
  addKeyHandler('e', &myEnableMotorsCB);
  myGroup.activateExclusive();
}

AREXPORT void ArModeTeleop::deactivate(void)
{
  remKeyHandler(&myEnableMotorsCB);
  if (!baseDeactivate())
    return;
  myGroup.deactivate();
}

AREXPORT void ArModeTeleop::help(void)
{
  ArLog::log(ArLog::Terse, 
	   "Teleop mode will drive under your joystick or keyboard control.");
  ArLog::log(ArLog::Terse, 
	     "It will not allow you to drive into obstacles it can see,");
  ArLog::log(ArLog::Terse, 
      "though if you are presistent you may be able to run into something.");
  ArLog::log(ArLog::Terse, "For joystick, hold in the trigger button and then move the joystick to drive.");
  ArLog::log(ArLog::Terse, "For keyboard control these are the keys and their actions:");
  ArLog::log(ArLog::Terse, "%13s:  speed up if forward or no motion, slow down if going backwards", "up arrow");
  ArLog::log(ArLog::Terse, "%13s:  slow down if going forwards, speed up if backward or no motion", "down arrow");
  ArLog::log(ArLog::Terse, "%13s:  turn left", "left arrow");
  ArLog::log(ArLog::Terse, "%13s:  turn right", "right arrow");
  if (myRobot->hasLatVel())
  {
    ArLog::log(ArLog::Terse, "%13s:  move left", "z");
    ArLog::log(ArLog::Terse, "%13s:  move right", "x");
  }
  ArLog::log(ArLog::Terse, "%13s:  stop", "space bar");
  ArLog::log(ArLog::Terse, "%13s:  (re)enable motors", "e");
  if (!myRobot->hasLatVel())
    printf("%10s %10s %10s %10s %10s %10s", "transVel", "rotVel", "x", "y", "th", "volts");
  else
    printf("%10s %10s %10s %10s %10s %10s %10s", "transVel", "rotVel", "latVel", "x", "y", "th", "volts");
  if(myRobot->haveStateOfCharge())
    printf(" %10s", "soc");
  printf(" %10s", ""); //flags
  printf("\n");
  fflush(stdout);
}

AREXPORT void ArModeTeleop::userTask(void)
{
  if (!myRobot->hasLatVel())
    printf("\r%10.0f %10.0f %10.0f %10.0f %10.1f %10.1f", myRobot->getVel(), 
	   myRobot->getRotVel(), myRobot->getX(), myRobot->getY(), 
	   myRobot->getTh(), myRobot->getRealBatteryVoltage());
  else
    printf("\r%9.0f %9.0f %9.0f %9.0f %9.0f %9.1f %9.1f", 
	   myRobot->getVel(), myRobot->getRotVel(), myRobot->getLatVel(),
	   myRobot->getX(), myRobot->getY(), myRobot->getTh(), 
	   myRobot->getRealBatteryVoltage());
  if(myRobot->haveStateOfCharge())
    printf(" %9.1f", myRobot->getStateOfCharge());
  if(myRobot->isEStopPressed()) printf(" [ESTOP]"); 
  if(myRobot->isLeftMotorStalled() || myRobot->isRightMotorStalled()) printf(" [STALL] "); 
  if(!myRobot->areMotorsEnabled()) printf(" [DISABLED] "); 
  
  // spaces to cover previous output
  if(!myRobot->isEStopPressed() || !(myRobot->isLeftMotorStalled() && myRobot->isRightMotorStalled()) || myRobot->areMotorsEnabled())
    printf("                 ");

  fflush(stdout);
}

AREXPORT ArModeUnguardedTeleop::ArModeUnguardedTeleop(ArRobot *robot,
						      const char *name, 
						      char key, char key2): 
  ArMode(robot, name, key, key2),
  myGroup(robot),
  myEnableMotorsCB(robot, &ArRobot::enableMotors)
{
  myGroup.deactivate();
}

AREXPORT ArModeUnguardedTeleop::~ArModeUnguardedTeleop()
{
  
}

AREXPORT void ArModeUnguardedTeleop::activate(void)
{
  if (!baseActivate())
    return;
  addKeyHandler('e', &myEnableMotorsCB);
  myGroup.activateExclusive();
}

AREXPORT void ArModeUnguardedTeleop::deactivate(void)
{
  remKeyHandler(&myEnableMotorsCB);
  if (!baseDeactivate())
    return;
  myGroup.deactivate();
}

AREXPORT void ArModeUnguardedTeleop::help(void)
{
  ArLog::log(ArLog::Terse, 
	   "Unguarded teleop mode will drive under your joystick or keyboard control.");
  ArLog::log(ArLog::Terse, 
	     "\n### THIS MODE IS UNGUARDED AND UNSAFE, BE CAREFUL DRIVING");
  ArLog::log(ArLog::Terse,
	     "\nAs it will allow you to drive into things or down stairs.");
  ArLog::log(ArLog::Terse, "For joystick, hold in the trigger button and then move the joystick to drive.");
  ArLog::log(ArLog::Terse, "For keyboard control these are the keys and their actions:");
  ArLog::log(ArLog::Terse, "%13s:  speed up if forward or no motion, slow down if going backwards", "up arrow");
  ArLog::log(ArLog::Terse, "%13s:  slow down if going forwards, speed up if backward or no motion", "down arrow");
  ArLog::log(ArLog::Terse, "%13s:  turn left", "left arrow");
  ArLog::log(ArLog::Terse, "%13s:  turn right", "right arrow");
  if (myRobot->hasLatVel())
  {
    ArLog::log(ArLog::Terse, "%13s:  move left", "z");
    ArLog::log(ArLog::Terse, "%13s:  move right", "x");
  }
  ArLog::log(ArLog::Terse, "%13s:  stop", "space bar");
  ArLog::log(ArLog::Terse, "%13s:  (re)enable motors", "e");
  if (!myRobot->hasLatVel())
    printf("%10s %10s %10s %10s %10s %10s", "transVel", "rotVel", "x", "y", "th", "volts");
  else
    printf("%10s %10s %10s %10s %10s %10s %10s", "transVel", "rotVel", "latVel", "x", "y", "th", "volts");
  if(myRobot->haveStateOfCharge())
    printf(" %10s", "soc");
  printf(" %10s", ""); //flags
  printf("\n");
  fflush(stdout);
}

AREXPORT void ArModeUnguardedTeleop::userTask(void)
{
  if (!myRobot->hasLatVel())
    printf("\r%9.0f %9.0f %9.0f %9.0f %9.1f %9.1f", myRobot->getVel(), 
	   myRobot->getRotVel(), myRobot->getX(), myRobot->getY(), 
	   myRobot->getTh(), myRobot->getRealBatteryVoltage());
  else
    printf("\r%9.0f %9.0f %9.0f %9.0f %9.0f %9.1f %9.1f", 
	   myRobot->getVel(), myRobot->getRotVel(), myRobot->getLatVel(),
	   myRobot->getX(), myRobot->getY(), myRobot->getTh(), 
	   myRobot->getRealBatteryVoltage());
  if(myRobot->haveStateOfCharge())
    printf(" %9.1f", myRobot->getStateOfCharge());
  if(myRobot->isEStopPressed()) printf(" [ESTOP] ");
  if(myRobot->isLeftMotorStalled() || myRobot->isRightMotorStalled()) printf(" [STALL] ");
  if(!myRobot->areMotorsEnabled()) printf(" [DISABLED] ");

  // spaces to cover previous output
  if(!myRobot->isEStopPressed() || !(myRobot->isLeftMotorStalled() && myRobot->isRightMotorStalled()) || myRobot->areMotorsEnabled())
    printf("                 ");

  fflush(stdout);
}

AREXPORT ArModeWander::ArModeWander(ArRobot *robot, const char *name, char key, char key2): 
  ArMode(robot, name, key, key2),
  myGroup(robot)
{
  myGroup.deactivate();
}

AREXPORT ArModeWander::~ArModeWander()
{
  
}

AREXPORT void ArModeWander::activate(void)
{
  if (!baseActivate())
    return;
  myGroup.activateExclusive();
}

AREXPORT void ArModeWander::deactivate(void)
{
  if (!baseDeactivate())
    return;
  myGroup.deactivate(); 
}

AREXPORT void ArModeWander::help(void)
{
  ArLog::log(ArLog::Terse, "Wander mode will simply drive around forwards until it finds an obstacle,");
  ArLog::log(ArLog::Terse, "then it will turn until its clear, and continue.");
  printf("%10s %10s %10s %10s %10s %10s\n", "transVel", "rotVel", "x", "y", "th", "volts");
  fflush(stdout);
}

AREXPORT void ArModeWander::userTask(void)
{
  printf("\r%10.0f %10.0f %10.0f %10.0f %10.1f %10.1f", myRobot->getVel(), 
	 myRobot->getRotVel(), myRobot->getX(), myRobot->getY(), 
	 myRobot->getTh(), myRobot->getRealBatteryVoltage());
  fflush(stdout);
}

AREXPORT ArModeGripper::ArModeGripper(ArRobot *robot, const char *name, 
				      char key, char key2): 
  ArMode(robot, name, key, key2),
  myGripper(robot),
  myOpenCB(this, &ArModeGripper::open),
  myCloseCB(this, &ArModeGripper::close),
  myUpCB(this, &ArModeGripper::up),
  myDownCB(this, &ArModeGripper::down),
  myStopCB(this, &ArModeGripper::stop),
  myExerciseCB(this, &ArModeGripper::exercise)
{
  myExercising = false;
}

AREXPORT ArModeGripper::~ArModeGripper()
{
  
}

AREXPORT void ArModeGripper::activate(void)
{
  if (!baseActivate())
    return;

  addKeyHandler(ArKeyHandler::UP, &myUpCB);
  addKeyHandler(ArKeyHandler::DOWN, &myDownCB);
  addKeyHandler(ArKeyHandler::RIGHT, &myOpenCB);
  addKeyHandler(ArKeyHandler::LEFT, &myCloseCB);  
  addKeyHandler(ArKeyHandler::SPACE, &myStopCB);
  addKeyHandler('e', &myExerciseCB);
  addKeyHandler('E', &myExerciseCB);
}

AREXPORT void ArModeGripper::deactivate(void)
{
  if (!baseDeactivate())
    return;

  remKeyHandler(&myUpCB);
  remKeyHandler(&myDownCB);
  remKeyHandler(&myOpenCB);
  remKeyHandler(&myCloseCB);
  remKeyHandler(&myStopCB);
  remKeyHandler(&myExerciseCB);
}

AREXPORT void ArModeGripper::userTask(void)
{
  int val;
  printf("\r");
  if (myGripper.getBreakBeamState() & 2) // outer 
    printf("%13s", "blocked");
  else 
    printf("%13s", "clear");
  if (myGripper.getBreakBeamState() & 1) // inner
    printf("%13s", "blocked");
  else 
    printf("%13s", "clear");
  val = myGripper.getGripState(); // gripper portion
  if (val == 0)
    printf("%13s", "between");
  else if (val == 1)
    printf("%13s", "open");
  else if (val == 2)
    printf("%13s", "closed");
  if (myGripper.isLiftMaxed()) // lift
    printf("%13s", "maxed");
  else
    printf("%13s", "clear");
  val = myGripper.getPaddleState(); // paddle section
  if (val & 1) // left paddle
    printf("%13s", "triggered");
  else
    printf("%13s", "clear");
  if (val & 2) // right paddle
    printf("%13s", "triggered");
  else
    printf("%13s", "clear");
  fflush(stdout);

  // exercise the thing
  if (myExercising)
  {
    switch (myExerState) {
    case UP_OPEN:
      if ((myLastExer.mSecSince() > 3000 && myGripper.isLiftMaxed()) ||
	  myLastExer.mSecSince() > 30000)
      {
	myGripper.gripClose();
	myExerState = UP_CLOSE;
	myLastExer.setToNow();
	if (myLastExer.mSecSince() > 30000)
	  ArLog::log(ArLog::Terse, "\nLift took more than thirty seconds to raise, there is probably a problem with it.\n");
      }
      break;
    case UP_CLOSE:
      if (myGripper.getGripState() == 2 || myLastExer.mSecSince() > 10000)
      {
	myGripper.liftDown();
	myExerState = DOWN_CLOSE;
	myLastExer.setToNow();
	if (myLastExer.mSecSince() > 10000)
	  ArLog::log(ArLog::Terse, "\nGripper took more than 10 seconds to close, there is probably a problem with it.\n");
      }
      break;
    case DOWN_CLOSE:
      if ((myLastExer.mSecSince() > 3000 && myGripper.isLiftMaxed()) ||
	  myLastExer.mSecSince() > 30000)
      {
	myGripper.gripOpen();
	myExerState = DOWN_OPEN;
	myLastExer.setToNow();
	if (myLastExer.mSecSince() > 30000)
	  ArLog::log(ArLog::Terse, "\nLift took more than thirty seconds to raise, there is probably a problem with it.\n");
      }
      break;
    case DOWN_OPEN:
      if (myGripper.getGripState() == 1 || myLastExer.mSecSince() > 10000)
      {
	myGripper.liftUp();
	myExerState = UP_OPEN;
	myLastExer.setToNow();
	if (myLastExer.mSecSince() > 10000)
	  ArLog::log(ArLog::Terse, "\nGripper took more than 10 seconds to open, there is probably a problem with it.\n");
      }
      break;
    }      
    
  }
}

AREXPORT void ArModeGripper::open(void)
{
  if (myExercising == true)
  {
    myExercising = false;
    myGripper.gripperHalt();
  }
  myGripper.gripOpen();
}

AREXPORT void ArModeGripper::close(void)
{
  if (myExercising == true)
  {
    myExercising = false;
    myGripper.gripperHalt();
  }
  myGripper.gripClose();
}

AREXPORT void ArModeGripper::up(void)
{
  if (myExercising == true)
  {
    myExercising = false;
    myGripper.gripperHalt();
  }
  myGripper.liftUp();
}

AREXPORT void ArModeGripper::down(void)
{
  if (myExercising == true)
  {
    myExercising = false;
    myGripper.gripperHalt();
  }
  myGripper.liftDown();
}

AREXPORT void ArModeGripper::stop(void)
{
  if (myExercising == true)
  {
    myExercising = false;
    myGripper.gripperHalt();
  }
  myGripper.gripperHalt();
}

AREXPORT void ArModeGripper::exercise(void)
{
  if (myExercising == false)
  {
    ArLog::log(ArLog::Terse, 
       "\nGripper will now be exercised until another command is given.");
    myExercising = true;
    myExerState = UP_OPEN;
    myGripper.liftUp();
    myGripper.gripOpen();
    myLastExer.setToNow();
  }
}

AREXPORT void ArModeGripper::help(void)
{
  ArLog::log(ArLog::Terse, 
	     "Gripper mode will let you control or exercise the gripper.");
  ArLog::log(ArLog::Terse, 
      "If you start exercising the gripper it will stop your other commands.");
  ArLog::log(ArLog::Terse, 
	     "If you use other commands it will interrupt the exercising.");
  ArLog::log(ArLog::Terse, "%13s:  raise lift", "up arrow");
  ArLog::log(ArLog::Terse, "%13s:  lower lift", "down arrow");
  ArLog::log(ArLog::Terse, "%13s:  close gripper paddles", "left arrow");
  ArLog::log(ArLog::Terse, "%13s:  open gripper paddles", "right arrow");
  ArLog::log(ArLog::Terse, "%13s:  stop gripper paddles and lift", 
	     "space bar");
  ArLog::log(ArLog::Terse, "%13s:  exercise the gripper", "'e' or 'E'");
  ArLog::log(ArLog::Terse, "\nGripper status:");
  ArLog::log(ArLog::Terse, "%13s%13s%13s%13s%13s%13s", "BB outer", "BB inner",
	     "Paddles", "Lift", "LeftPaddle", "RightPaddle");
  
}



AREXPORT ArModeCamera::ArModeCamera(ArRobot *robot, const char *name, 
				      char key, char key2): 
  ArMode(robot, name, key, key2),
  myUpCB(this, &ArModeCamera::up),
  myDownCB(this, &ArModeCamera::down),
  myLeftCB(this, &ArModeCamera::left),
  myRightCB(this, &ArModeCamera::right),
  myCenterCB(this, &ArModeCamera::center),
  myZoomInCB(this, &ArModeCamera::zoomIn),  
  myZoomOutCB(this, &ArModeCamera::zoomOut),
  myExerciseCB(this, &ArModeCamera::exercise),
  mySonyCB(this, &ArModeCamera::sony),
  myCanonCB(this, &ArModeCamera::canon),
  myDpptuCB(this, &ArModeCamera::dpptu),
  myAmptuCB(this, &ArModeCamera::amptu),
  myCanonInvertedCB(this, &ArModeCamera::canonInverted),
  mySonySerialCB(this, &ArModeCamera::sonySerial),
  myCanonSerialCB(this, &ArModeCamera::canonSerial),
  myDpptuSerialCB(this, &ArModeCamera::dpptuSerial),
  myAmptuSerialCB(this, &ArModeCamera::amptuSerial),
  myCanonInvertedSerialCB(this, &ArModeCamera::canonInvertedSerial),
  myRVisionSerialCB(this, &ArModeCamera::rvisionSerial),
  myCom1CB(this, &ArModeCamera::com1),
  myCom2CB(this, &ArModeCamera::com2),
  myCom3CB(this, &ArModeCamera::com3),
  myCom4CB(this, &ArModeCamera::com4),
  myUSBCom0CB(this, &ArModeCamera::usb0),
  myUSBCom9CB(this, &ArModeCamera::usb9),
  myAux1CB(this, &ArModeCamera::aux1),
  myAux2CB(this, &ArModeCamera::aux2),
  myPanAmount(5),
  myTiltAmount(3),
  myAutoFocusOn(true),
  myToggleAutoFocusCB(this, &ArModeCamera::toggleAutoFocus)
{
  myState = STATE_CAMERA;
  myExercising = false;
}

AREXPORT ArModeCamera::~ArModeCamera()
{
  
}

AREXPORT void ArModeCamera::activate(void)
{
  ArKeyHandler *keyHandler;
  if (!baseActivate())
    return;
  // see if there is already a keyhandler, if not something is wrong
  // (since constructor should make one if there isn't one yet
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    ArLog::log(ArLog::Terse,"ArModeCamera::activate: There should already be a key handler, but there isn't... mode won't work");
    return;
  }
  if (myState == STATE_CAMERA)
    takeCameraKeys();
  else if (myState == STATE_PORT)
    takePortKeys();
  else if (myState == STATE_MOVEMENT)
    takeMovementKeys();
  else
    ArLog::log(ArLog::Terse,"ArModeCamera in bad state.");
  
}

AREXPORT void ArModeCamera::deactivate(void)
{
  if (!baseDeactivate())
    return;
  if (myState == STATE_CAMERA)
    giveUpCameraKeys();
  else if (myState == STATE_PORT)
    giveUpPortKeys();
  else if (myState == STATE_MOVEMENT)
    giveUpMovementKeys();
  else
    ArLog::log(ArLog::Terse,"ArModeCamera in bad state.");
}

AREXPORT void ArModeCamera::userTask(void)
{
  if (myExercising && myCam != NULL && myLastExer.mSecSince() > 7000)
  {
    switch (myExerState) {
    case CENTER:
      myCam->panTilt(myCam->getMaxNegPan(), myCam->getMaxPosTilt());
      myExerState = UP_LEFT;
      myLastExer.setToNow();
      break;
    case UP_LEFT:
      myCam->panTilt(myCam->getMaxPosPan(), myCam->getMaxPosTilt());
      myExerState = UP_RIGHT;
      myLastExer.setToNow();
      break;
    case UP_RIGHT:
      myCam->panTilt(myCam->getMaxPosPan(), myCam->getMaxNegTilt());
      myExerState = DOWN_RIGHT;
      myLastExer.setToNow();
      break;
    case DOWN_RIGHT:
      myCam->panTilt(myCam->getMaxNegPan(), myCam->getMaxNegTilt());
      myExerState = DOWN_LEFT;
      myLastExer.setToNow();
      break;
    case DOWN_LEFT:
      myCam->panTilt(0, 0);
      myExerState = CENTER;
      myLastExer.setToNow();
      break;
    }      
  }
  if (myExercising && myCam != NULL && myCam->canZoom() && 
      myLastExerZoomed.mSecSince() > 35000)
  {
    if (myExerZoomedIn)
      myCam->zoom(myCam->getMinZoom());
    else
      myCam->zoom(myCam->getMaxZoom());
    myLastExerZoomed.setToNow();
  }
}

AREXPORT void ArModeCamera::left(void)
{
  if (myExercising == true)
    myExercising = false;
  myCam->panRel(-myPanAmount);
}

AREXPORT void ArModeCamera::right(void)
{
  if (myExercising == true)
    myExercising = false;
  myCam->panRel(myPanAmount);
}

AREXPORT void ArModeCamera::up(void)
{
  if (myExercising == true)
    myExercising = false;
  myCam->tiltRel(myTiltAmount);
}

AREXPORT void ArModeCamera::down(void)
{  
  if (myExercising == true)
    myExercising = false;
  myCam->tiltRel(-myTiltAmount);
}

AREXPORT void ArModeCamera::center(void)
{
  if (myExercising == true)
    myExercising = false;
  myCam->panTilt(0, 0);
  myCam->zoom(myCam->getMinZoom());
}

AREXPORT void ArModeCamera::exercise(void)
{
  if (myExercising == false)
  {
    ArLog::log(ArLog::Terse, 
       "Camera will now be exercised until another command is given.");
    myExercising = true;
    myExerState = UP_LEFT;
    myLastExer.setToNow();
    myCam->panTilt(myCam->getMaxNegPan(), myCam->getMaxPosTilt());
    myLastExerZoomed.setToNow();
    myExerZoomedIn = true;
    if (myCam->canZoom())
      myCam->zoom(myCam->getMaxZoom());
  }
}

AREXPORT void ArModeCamera::toggleAutoFocus()
{
  ArLog::log(ArLog::Terse, "Turning autofocus %s", myAutoFocusOn?"off":"on");
  if(myCam->setAutoFocus(!myAutoFocusOn))
    myAutoFocusOn = !myAutoFocusOn;
}

AREXPORT void ArModeCamera::help(void)
{
  ArLog::log(ArLog::Terse, 
	     "Camera mode will let you control or exercise the camera.");
  ArLog::log(ArLog::Terse, 
      "If you start exercising the camera it will stop your other commands.");
  if (myState == STATE_CAMERA)
    helpCameraKeys();
  else if (myState == STATE_PORT)
    helpPortKeys();
  else if (myState == STATE_MOVEMENT)
    helpMovementKeys();
  else
    ArLog::log(ArLog::Terse, "Something is horribly wrong and mode camera is in no state.");
}

AREXPORT void ArModeCamera::zoomIn(void)
{
  if (myCam->canZoom())
  {
    myCam->zoom(myCam->getZoom() + 
	 ArMath::roundInt((myCam->getMaxZoom() - myCam->getMinZoom()) * .01));
  }
}

AREXPORT void ArModeCamera::zoomOut(void)
{
  if (myCam->canZoom())
  {
    myCam->zoom(myCam->getZoom() - 
	ArMath::roundInt((myCam->getMaxZoom() - myCam->getMinZoom()) * .01));
  }
}

AREXPORT void ArModeCamera::sony(void)
{
  myCam = new ArSonyPTZ(myRobot);
  ArLog::log(ArLog::Terse, "\nSony selected, now need to select the aux port.");
  cameraToAux();
}

AREXPORT void ArModeCamera::canon(void)
{
  myCam = new ArVCC4(myRobot);
  ArLog::log(ArLog::Terse, "\nCanon selected, now need to select the aux port.");
  cameraToAux();
}

AREXPORT void ArModeCamera::dpptu(void)
{
  myCam = new ArDPPTU(myRobot);
  ArLog::log(ArLog::Terse, "\nDPPTU selected, now need to select the aux port.");
  cameraToAux();
}

AREXPORT void ArModeCamera::amptu(void)
{
  myCam = new ArAMPTU(myRobot);
  ArLog::log(ArLog::Terse, 
	     "\nActivMedia Pan Tilt Unit selected, now need to select the aux port.");
  cameraToAux();
}

AREXPORT void ArModeCamera::canonInverted(void)
{
  myCam = new ArVCC4(myRobot, true);
  ArLog::log(ArLog::Terse, "\nInverted Canon selected, now need to select the aux port.");
  cameraToAux();
}

AREXPORT void ArModeCamera::sonySerial(void)
{
  myCam = new ArSonyPTZ(myRobot);
  ArLog::log(ArLog::Terse, "\nSony selected, now need to select serial port.");
  cameraToPort();
}

AREXPORT void ArModeCamera::canonSerial(void)
{
  myCam = new ArVCC4(myRobot);
  ArLog::log(ArLog::Terse, 
	     "\nCanon VCC4 selected, now need to select serial port.");
  cameraToPort();
}

AREXPORT void ArModeCamera::dpptuSerial(void)
{
  myCam = new ArDPPTU(myRobot);
  ArLog::log(ArLog::Terse, "\nDPPTU selected, now need to select serial port.");
  cameraToPort();
}

AREXPORT void ArModeCamera::amptuSerial(void)
{
  myCam = new ArAMPTU(myRobot);
  ArLog::log(ArLog::Terse, "\nAMPTU selected, now need to select serial port.");
  cameraToPort();
}

AREXPORT void ArModeCamera::canonInvertedSerial(void)
{
  myCam = new ArVCC4(myRobot, true);
  ArLog::log(ArLog::Terse, 
	     "\nInverted Canon VCC4 selected, now need to select serial port.");
  cameraToPort();
}

AREXPORT void ArModeCamera::rvisionSerial(void)
{
  myCam = new ArRVisionPTZ(myRobot);
  ArLog::log(ArLog::Terse, "\nRVision selected, now need to select serial port.");
  cameraToPort();
}

AREXPORT void ArModeCamera::com1(void)
{
  myConn.setPort(ArUtil::COM1);
  portToMovement();
}

AREXPORT void ArModeCamera::com2(void)
{
  myConn.setPort(ArUtil::COM2);
  portToMovement();
}

AREXPORT void ArModeCamera::com3(void)
{
  myConn.setPort(ArUtil::COM3);
  portToMovement();
}

AREXPORT void ArModeCamera::com4(void)
{
  myConn.setPort(ArUtil::COM4);
  portToMovement();
}

AREXPORT void ArModeCamera::usb0(void)
{
  myConn.setPort("/dev/ttyUSB0");
  portToMovement();
}

AREXPORT void ArModeCamera::usb9(void)
{
  myConn.setPort("/dev/ttyUSB9");
  portToMovement();
}

AREXPORT void ArModeCamera::aux1(void)
{
  myCam->setAuxPort(1);
  auxToMovement();
}
AREXPORT void ArModeCamera::aux2(void)
{
  myCam->setAuxPort(2);
  auxToMovement();
}

void ArModeCamera::cameraToMovement(void)
{
  myState = STATE_MOVEMENT;
  myCam->init();
  myRobot->setPTZ(myCam);
  giveUpCameraKeys();
  takeMovementKeys();
  helpMovementKeys();
}

void ArModeCamera::cameraToPort(void)
{
  myState = STATE_PORT;
  giveUpCameraKeys();
  takePortKeys();
  helpPortKeys();
}

void ArModeCamera::cameraToAux(void)
{
  giveUpCameraKeys();
  takeAuxKeys();
  helpAuxKeys();
}

void ArModeCamera::portToMovement(void)
{
  ArLog::log(ArLog::Normal, "ArModeCamera: Opening connection to camera on port %s", myConn.getPortName());
  if (!myConn.openSimple())
  {
    ArLog::log(ArLog::Terse, 
	       "\n\nArModeCamera: Could not open camera on that port, try another port.\n");
    helpPortKeys();
    return;
  }
  if(!myCam->setDeviceConnection(&myConn))
  {
    ArLog::log(ArLog::Terse, "\n\nArModeCamera: Error setting device connection!\n");
    return;
  }
  myCam->init();
  myRobot->setPTZ(myCam);
  myState = STATE_MOVEMENT;
  giveUpPortKeys();
  takeMovementKeys();
  helpMovementKeys();
}

void ArModeCamera::auxToMovement(void)
{
  myCam->init();
  myRobot->setPTZ(myCam);
  myState = STATE_MOVEMENT;
  giveUpAuxKeys();
  takeMovementKeys();
  helpMovementKeys();
}

void ArModeCamera::takeCameraKeys(void)
{
  addKeyHandler('1', &mySonyCB);
  addKeyHandler('2', &myCanonCB);
  addKeyHandler('3', &myDpptuCB);
  addKeyHandler('4', &myAmptuCB);
  addKeyHandler('5', &myCanonInvertedCB);
  addKeyHandler('!', &mySonySerialCB);
  addKeyHandler('@', &myCanonSerialCB);
  addKeyHandler('#', &myDpptuSerialCB);
  addKeyHandler('$', &myAmptuSerialCB);
  addKeyHandler('%', &myCanonInvertedSerialCB);
  addKeyHandler('^', &myRVisionSerialCB);
}

void ArModeCamera::giveUpCameraKeys(void)
{
  remKeyHandler(&myCanonCB);
  remKeyHandler(&mySonyCB);
  remKeyHandler(&myDpptuCB);
  remKeyHandler(&myAmptuCB);
  remKeyHandler(&myCanonInvertedCB);
  remKeyHandler(&mySonySerialCB);
  remKeyHandler(&myCanonSerialCB);
  remKeyHandler(&myDpptuSerialCB);
  remKeyHandler(&myAmptuSerialCB);
  remKeyHandler(&myCanonInvertedSerialCB);
  remKeyHandler(&myRVisionSerialCB);
}

void ArModeCamera::helpCameraKeys(void)
{
  ArLog::log(ArLog::Terse, 
	     "You now need to select what type of camera you have.");
  ArLog::log(ArLog::Terse, 
	     "%13s: select a SONY PTZ camera attached to the robot", "'1'");
  ArLog::log(ArLog::Terse, 
	     "%13s: select a Canon VCC4 camera attached to the robot", "'2'");
  ArLog::log(ArLog::Terse, 
	     "%13s: select a DPPTU camera attached to the robot", "'3'");
  ArLog::log(ArLog::Terse, 
	     "%13s: select an AMPTU camera attached to the robot", "'4'");
  ArLog::log(ArLog::Terse, 
	     "%13s: select an inverted Canon VCC4 camera attached to the robot", "'5'");

  ArLog::log(ArLog::Terse, 
	     "%13s: select a SONY PTZ camera attached to a serial port", 
	     "'!'");
  ArLog::log(ArLog::Terse, 
	     "%13s: select a Canon VCC4 camera attached to a serial port", 
	     "'@'");
  ArLog::log(ArLog::Terse, 
	     "%13s: select a DPPTU camera attached to a serial port",
	     "'#'");
  ArLog::log(ArLog::Terse, 
	     "%13s: select an AMPTU camera attached to a serial port", 
	     "'$'");
  ArLog::log(ArLog::Terse, 
	     "%13s: select an inverted Canon VCC4 camera attached to a serial port", 
	     "'%'");
  ArLog::log(ArLog::Terse,
	     "%13s: select an RVision camera attached to a serial port",
	     "'^'");
}

void ArModeCamera::takePortKeys(void)
{
  addKeyHandler('1', &myCom1CB);
  addKeyHandler('2', &myCom2CB);
  addKeyHandler('3', &myCom3CB);
  addKeyHandler('4', &myCom4CB);
  addKeyHandler('5', &myUSBCom0CB);
  addKeyHandler('6', &myUSBCom9CB);
}

void ArModeCamera::giveUpPortKeys(void)
{
  remKeyHandler(&myCom1CB);
  remKeyHandler(&myCom2CB);
  remKeyHandler(&myCom3CB);
  remKeyHandler(&myCom4CB);
  remKeyHandler(&myUSBCom0CB);
  remKeyHandler(&myUSBCom9CB);
}

void ArModeCamera::helpPortKeys(void)
{
  ArLog::log(ArLog::Terse, 
	     "You now need to select what port your camera is on.");
  ArLog::log(ArLog::Terse, "%13s:  select COM1 or /dev/ttyS0", "'1'");
  ArLog::log(ArLog::Terse, "%13s:  select COM2 or /dev/ttyS1", "'2'");
  ArLog::log(ArLog::Terse, "%13s:  select COM3 or /dev/ttyS2", "'3'");
  ArLog::log(ArLog::Terse, "%13s:  select COM4 or /dev/ttyS3", "'4'");
  ArLog::log(ArLog::Terse, "%13s:  select /dev/ttyUSB0", "'5'");
  ArLog::log(ArLog::Terse, "%13s:  select /dev/ttyUSB9", "'6'");
}

void ArModeCamera::takeAuxKeys(void)
{
  addKeyHandler('1', &myAux1CB);
  addKeyHandler('2', &myAux2CB);
}

void ArModeCamera::giveUpAuxKeys(void)
{
  remKeyHandler(&myAux1CB);
  remKeyHandler(&myAux2CB);
}

void ArModeCamera::helpAuxKeys(void)
{
  ArLog::log(ArLog::Terse,
             "You now need to select what aux port your camera is on.");
  ArLog::log(ArLog::Terse, "%13s:  select AUX1", "'1'");
  ArLog::log(ArLog::Terse, "%13s:  select AUX2", "'2'");
}

void ArModeCamera::takeMovementKeys(void)
{
  addKeyHandler(ArKeyHandler::UP, &myUpCB);
  addKeyHandler(ArKeyHandler::DOWN, &myDownCB);
  addKeyHandler(ArKeyHandler::LEFT, &myLeftCB);
  addKeyHandler(ArKeyHandler::RIGHT, &myRightCB);
  addKeyHandler(ArKeyHandler::SPACE, &myCenterCB);
  addKeyHandler('e', &myExerciseCB);
  addKeyHandler('E', &myExerciseCB);
  if (myCam->canZoom())
  {
    addKeyHandler('z', &myZoomInCB);
    addKeyHandler('Z', &myZoomInCB);
    addKeyHandler('x', &myZoomOutCB);
    addKeyHandler('X', &myZoomOutCB);
  }
  addKeyHandler('f', &myToggleAutoFocusCB);
  addKeyHandler('F', &myToggleAutoFocusCB);
}

void ArModeCamera::giveUpMovementKeys(void)
{
  remKeyHandler(&myUpCB);
  remKeyHandler(&myDownCB);
  remKeyHandler(&myLeftCB);
  remKeyHandler(&myRightCB);
  remKeyHandler(&myCenterCB);
  remKeyHandler(&myExerciseCB);
  if (myCam->canZoom())
  {
    remKeyHandler(&myZoomInCB);
    remKeyHandler(&myZoomOutCB);
  }
  remKeyHandler(&myToggleAutoFocusCB);
}

void ArModeCamera::helpMovementKeys(void)
{
  ArLog::log(ArLog::Terse, 
	     "Camera mode will now let you move the camera.");
  ArLog::log(ArLog::Terse, "%13s:  tilt camera up by %d", "up arrow", myTiltAmount);
  ArLog::log(ArLog::Terse, "%13s:  tilt camera down by %d", "down arrow", myTiltAmount);
  ArLog::log(ArLog::Terse, "%13s:  pan camera left by %d", "left arrow", myPanAmount);
  ArLog::log(ArLog::Terse, "%13s:  pan camera right by %d", "right arrow", myPanAmount);
  ArLog::log(ArLog::Terse, "%13s:  center camera and zoom out", 
	     "space bar");
  ArLog::log(ArLog::Terse, "%13s:  exercise the camera", "'e' or 'E'");
  if (myCam->canZoom())
  {
    ArLog::log(ArLog::Terse, "%13s:  zoom in", "'z' or 'Z'");
    ArLog::log(ArLog::Terse, "%13s:  zoom out", "'x' or 'X'");
  }
  ArLog::log(ArLog::Terse, "%13s:  toggle auto/fixed focus", "'f' or 'F'");
}

AREXPORT ArModeSonar::ArModeSonar(ArRobot *robot, const char *name, char key,
				  char key2) :
  ArMode(robot, name, key, key2),
  myAllSonarCB(this, &ArModeSonar::allSonar),
  myFirstSonarCB(this, &ArModeSonar::firstSonar),
  mySecondSonarCB(this, &ArModeSonar::secondSonar),
  myThirdSonarCB(this, &ArModeSonar::thirdSonar),
  myFourthSonarCB(this, &ArModeSonar::fourthSonar)
{
  myState = STATE_FIRST;
}

AREXPORT ArModeSonar::~ArModeSonar()
{

}

AREXPORT void ArModeSonar::activate(void)
{
  if (!baseActivate())
    return;
  addKeyHandler('1', &myAllSonarCB);
  addKeyHandler('2', &myFirstSonarCB);
  addKeyHandler('3', &mySecondSonarCB);
  addKeyHandler('4', &myThirdSonarCB);
  addKeyHandler('5', &myFourthSonarCB);
}

AREXPORT void ArModeSonar::deactivate(void)
{
  if (!baseDeactivate())
    return;
  remKeyHandler(&myAllSonarCB);
  remKeyHandler(&myFirstSonarCB);
  remKeyHandler(&mySecondSonarCB);
  remKeyHandler(&myThirdSonarCB);
  remKeyHandler(&myFourthSonarCB);
}

AREXPORT void ArModeSonar::help(void)
{
  int i;
  ArLog::log(ArLog::Terse, "This mode displays different segments of sonar.");
  ArLog::log(ArLog::Terse, 
	     "You can use these keys to switch what is displayed:");
  ArLog::log(ArLog::Terse, "%13s: display all sonar", "'1'");
  ArLog::log(ArLog::Terse, "%13s: display sonar 0 - 7", "'2'");
  ArLog::log(ArLog::Terse, "%13s: display sonar 8 - 15", "'3'");
  ArLog::log(ArLog::Terse, "%13s: display sonar 16 - 23", "'4'");
  ArLog::log(ArLog::Terse, "%13s: display sonar 24 - 31", "'5'");
  ArLog::log(ArLog::Terse, "Sonar readings:");
  if (myState == STATE_ALL)
  {
    ArLog::log(ArLog::Terse, "Displaying all sonar.");
    for (i = 0; i < myRobot->getNumSonar(); ++i)
      printf("%6d", i); 
  }
  else if (myState == STATE_FIRST)
  {
    ArLog::log(ArLog::Terse, "Displaying 0-7 sonar.");
    for (i = 0; i < myRobot->getNumSonar() && i <= 7; ++i)
      printf("%6d", i); 
  }
  else if (myState == STATE_SECOND)
  {
    ArLog::log(ArLog::Terse, "Displaying 8-15 sonar.");
    for (i = 8; i < myRobot->getNumSonar() && i <= 15; ++i)
      printf("%6d", i); 
  }
  else if (myState == STATE_THIRD)
  {
    ArLog::log(ArLog::Terse, "Displaying 16-23 sonar.");
    for (i = 16; i < myRobot->getNumSonar() && i <= 23; ++i)
      printf("%6d", i); 
  }
  else if (myState == STATE_FOURTH)
  {
    ArLog::log(ArLog::Terse, "Displaying 24-31 sonar.");
    for (i = 24; i < myRobot->getNumSonar() && i <= 31; ++i)
      printf("%6d", i); 
  }
  printf("\n");
}

AREXPORT void ArModeSonar::userTask(void)
{
  int i;
  printf("\r");
  if (myState == STATE_ALL)
  {
    for (i = 0; i < myRobot->getNumSonar(); ++i)
      printf("%6d", myRobot->getSonarRange(i)); 
  }
  else if (myState == STATE_FIRST)
  {
     for (i = 0; i < myRobot->getNumSonar() && i <= 7; ++i)
      printf("%6d", myRobot->getSonarRange(i));
  }
  else if (myState == STATE_SECOND)
  {
     for (i = 8; i < myRobot->getNumSonar() && i <= 15; ++i)
      printf("%6d", myRobot->getSonarRange(i));
  }
  else if (myState == STATE_THIRD)
  {
    for (i = 16; i < myRobot->getNumSonar() && i <= 23; ++i)
      printf("%6d", myRobot->getSonarRange(i)); 
  }
  else if (myState == STATE_FOURTH)
  {
    for (i = 24; i < myRobot->getNumSonar() && i <= 31; ++i)
      printf("%6d", myRobot->getSonarRange(i)); 
  }
  fflush(stdout);
}

AREXPORT void ArModeSonar::allSonar(void)
{
  myState = STATE_ALL;
  printf("\n");
  help();
}

AREXPORT void ArModeSonar::firstSonar(void)
{
  myState = STATE_FIRST;
  printf("\n");
  help();
}

AREXPORT void ArModeSonar::secondSonar(void)
{
  myState = STATE_SECOND;
  printf("\n");
  help();
}

AREXPORT void ArModeSonar::thirdSonar(void)
{
  myState = STATE_THIRD;
  printf("\n");
  help();
}

AREXPORT void ArModeSonar::fourthSonar(void)
{
  myState = STATE_FOURTH;
  printf("\n");
  help();
}

AREXPORT ArModeBumps::ArModeBumps(ArRobot *robot, const char *name, char key, char key2): 
  ArMode(robot, name, key, key2)
{
}

AREXPORT ArModeBumps::~ArModeBumps()
{
  
}

AREXPORT void ArModeBumps::activate(void)
{
  if (!baseActivate())
    return;
}

AREXPORT void ArModeBumps::deactivate(void)
{
  if (!baseDeactivate())
    return;
}

AREXPORT void ArModeBumps::help(void)
{
  unsigned int i;
  ArLog::log(ArLog::Terse, "Bumps mode will display whether bumpers are triggered or not...");
  ArLog::log(ArLog::Terse, "keep in mind it is assuming you have a full bump ring... so you should");
  ArLog::log(ArLog::Terse, "ignore readings for where there aren't bumpers.");
  ArLog::log(ArLog::Terse, "Bumper readings:");
  for (i = 0; i < myRobot->getNumFrontBumpers(); i++)
  {
    printf("%6d", i + 1);
  }
  printf(" |");
  for (i = 0; i < myRobot->getNumRearBumpers(); i++)
  {
    printf("%6d", i + 1);
  }
  printf("\n");
}

AREXPORT void ArModeBumps::userTask(void)
{
  unsigned int i;
  int val;
  int bit;
  if (myRobot == NULL)
    return;
  printf("\r");
  val = ((myRobot->getStallValue() & 0xff00) >> 8);
  for (i = 0, bit = 2; i < myRobot->getNumFrontBumpers(); i++, bit *= 2)
  {
    if (val & bit)
      printf("%6s", "trig");
    else
      printf("%6s", "clear");
  }
  printf(" |");
  val = ((myRobot->getStallValue() & 0xff));
  for (i = 0, bit = 2; i < myRobot->getNumRearBumpers(); i++, bit *= 2)
  {
    if (val & bit)
      printf("%6s", "trig");
    else
      printf("%6s", "clear");
  }

}

AREXPORT ArModePosition::ArModePosition(ArRobot *robot, const char *name, char key, char key2, ArAnalogGyro *gyro): 
  ArMode(robot, name, key, key2),
  myUpCB(this, &ArModePosition::up),
  myDownCB(this, &ArModePosition::down),
  myLeftCB(this, &ArModePosition::left),
  myRightCB(this, &ArModePosition::right),
  myStopCB(this, &ArModePosition::stop),
  myResetCB(this, &ArModePosition::reset),
  myModeCB(this, &ArModePosition::mode),
  myGyroCB(this, &ArModePosition::gyro),
  myIncDistCB(this, &ArModePosition::incDistance),
  myDecDistCB(this, &ArModePosition::decDistance)
{
  myGyro = gyro;
  myMode = MODE_BOTH;
  myModeString = "both";
  myInHeadingMode = false;
  myDistance = 1000;

  if (myGyro != NULL && !myGyro->hasNoInternalData())
    myGyroZero = myGyro->getHeading();
  myRobotZero = myRobot->getRawEncoderPose().getTh();
  myInHeadingMode = true;
  myHeading = myRobot->getTh();
}

AREXPORT ArModePosition::~ArModePosition()
{
  
}

AREXPORT void ArModePosition::activate(void)
{
  if (!baseActivate())
    return;

  addKeyHandler(ArKeyHandler::UP, &myUpCB);
  addKeyHandler(ArKeyHandler::DOWN, &myDownCB);
  addKeyHandler(ArKeyHandler::LEFT, &myLeftCB);  
  addKeyHandler(ArKeyHandler::RIGHT, &myRightCB);
  addKeyHandler(ArKeyHandler::SPACE, &myStopCB);
  addKeyHandler(ArKeyHandler::PAGEUP, &myIncDistCB);
  addKeyHandler(ArKeyHandler::PAGEDOWN, &myDecDistCB);
  addKeyHandler('r', &myResetCB);
  addKeyHandler('R', &myResetCB);
  addKeyHandler('x', &myModeCB);
  addKeyHandler('X', &myModeCB);
  addKeyHandler('z', &myGyroCB);
  addKeyHandler('Z', &myGyroCB);
}

AREXPORT void ArModePosition::deactivate(void)
{
  if (!baseDeactivate())
    return;

  remKeyHandler(&myUpCB);
  remKeyHandler(&myDownCB);
  remKeyHandler(&myLeftCB);
  remKeyHandler(&myRightCB);
  remKeyHandler(&myStopCB);
  remKeyHandler(&myResetCB);
  remKeyHandler(&myModeCB);
  remKeyHandler(&myGyroCB);
  remKeyHandler(&myIncDistCB);
  remKeyHandler(&myDecDistCB);
}

AREXPORT void ArModePosition::up(void)
{
  myRobot->move(myDistance);
  if (myInHeadingMode)
  {
    myInHeadingMode = false;
    myHeading = myRobot->getTh();
  }
}

AREXPORT void ArModePosition::down(void)
{
  myRobot->move(-myDistance);
  if (myInHeadingMode)
  {
    myInHeadingMode = false;
    myHeading = myRobot->getTh();
  }
}

AREXPORT void ArModePosition::incDistance(void)
{
  myDistance += 500;
  puts("\n");
  help();
}

AREXPORT void ArModePosition::decDistance(void)
{
  myDistance -= 500;
  if(myDistance < 500) myDistance = 500;
  puts("\n");
  help();
}

AREXPORT void ArModePosition::left(void)
{
  myRobot->setDeltaHeading(90);
  myInHeadingMode = true;
}

AREXPORT void ArModePosition::right(void)
{
  myRobot->setDeltaHeading(-90);
  myInHeadingMode = true;
}

AREXPORT void ArModePosition::stop(void)
{
  myRobot->stop();
  myInHeadingMode = true;
}

AREXPORT void ArModePosition::reset(void)
{
  myRobot->stop();
  myRobot->moveTo(ArPose(0, 0, 0));
  if (myGyro != NULL && !myGyro->hasNoInternalData())
    myGyroZero = myGyro->getHeading();
  myRobotZero = myRobot->getRawEncoderPose().getTh();
  myInHeadingMode = true;
  myHeading = myRobot->getTh();
}

AREXPORT void ArModePosition::mode(void)
{
  if (myMode == MODE_BOTH)
  {
    myMode = MODE_EITHER;
    myModeString = "either";
    myInHeadingMode = true;
    myRobot->stop();
  }
  else if (myMode == MODE_EITHER)
  {
    myMode = MODE_BOTH;
    myModeString = "both";
  }
}

AREXPORT void ArModePosition::gyro(void)
{
  if (myGyro == NULL || !myGyro->haveGottenData())
    return;

  if (myGyro != NULL && myGyro->isActive())
    myGyro->deactivate();
  else if (myGyro != NULL && !myGyro->isActive() && 
	   myGyro->hasGyroOnlyMode() && !myGyro->isGyroOnlyActive())
    myGyro->activateGyroOnly();
  else if (myGyro != NULL && !myGyro->isActive())
    myGyro->activate();

  help();
}

AREXPORT void ArModePosition::help(void)
{
  ArLog::log(ArLog::Terse, "Mode is one of two values:");
  ArLog::log(ArLog::Terse, "%13s: heading and move can happen simultaneously", 
	     "both");
  ArLog::log(ArLog::Terse, "%13s: only heading or move is active (move holds heading)", "either");
  ArLog::log(ArLog::Terse, "");
  ArLog::log(ArLog::Terse, "%13s:  forward %.1f meter(s)", "up arrow", myDistance/1000.0);
  ArLog::log(ArLog::Terse, "%13s:  backward %.1f meter(s)", "down arrow", myDistance/1000.0);
  ArLog::log(ArLog::Terse, "%13s:  increase distance by 1/2 meter", "page up");
  ArLog::log(ArLog::Terse, "%13s:  decrease distance by 1/2 meter", "page down");
  ArLog::log(ArLog::Terse, "%13s:  turn left 90 degrees", "left arrow");
  ArLog::log(ArLog::Terse, "%13s:  turn right 90 degrees", "right arrow");
  ArLog::log(ArLog::Terse, "%13s:  stop", "space bar");
  ArLog::log(ArLog::Terse, "%13s:  reset ARIA position to (0, 0, 0)", "'r' or 'R'");
  ArLog::log(ArLog::Terse, "%13s:  switch heading/velocity mode","'x' or 'X'");
  if (myGyro != NULL && myGyro->haveGottenData() && !myGyro->hasGyroOnlyMode())
    ArLog::log(ArLog::Terse, "%13s:  turn gyro on or off (stays this way in other modes)","'z' or 'Z'");
  if (myGyro != NULL && myGyro->haveGottenData() && myGyro->hasGyroOnlyMode())
    ArLog::log(ArLog::Terse, "%13s:  turn gyro on or off or gyro only (stays this way in other modes)","'z' or 'Z'");

  ArLog::log(ArLog::Terse, "");
  ArLog::log(ArLog::Terse, "Position mode shows the position stats on a robot.");
  if (myGyro != NULL && myGyro->haveGottenData() && 
      !myGyro->hasNoInternalData())
    ArLog::log(ArLog::Terse, "%7s%7s%9s%7s%8s%7s%8s%6s%10s%10s%10s", "x", "y", "th", "comp", "volts", "mpacs", "mode", "gyro", "gyro_th", "robot_th", "raw");
  else if (myGyro != NULL && myGyro->haveGottenData() && 
	   myGyro->hasNoInternalData())
    ArLog::log(ArLog::Terse, "%7s%7s%9s%7s%8s%7s%8s%6s%10s", "x", "y", "th", "comp", "volts", "mpacs", "mode", "gyro", "raw");
  else
    ArLog::log(ArLog::Terse, "%7s%7s%9s%7s%8s%7s%8s%10s", "x", "y", "th", "comp", "volts", "mpacs", "mode", "raw");

  
}

AREXPORT void ArModePosition::userTask(void)
{
  if (myRobot == NULL)
    return;
  // if we're in either mode and not in the heading mode try to keep the
  // same heading (in heading mode its controlled by those commands)
  if (myMode == MODE_EITHER && !myInHeadingMode)
  {
    myRobot->setHeading(myHeading);
  }
  double voltage;
  if (myRobot->getRealBatteryVoltage() > 0)
    voltage = myRobot->getRealBatteryVoltage();
  else
    voltage = myRobot->getBatteryVoltage();

  std::string gyroString;
  if (myGyro == NULL)
    gyroString = "none";
  else if (myGyro->isActive())
    gyroString = "on";
  else if (myGyro->hasGyroOnlyMode() && myGyro->isGyroOnlyActive())
    gyroString = "only";
  else
    gyroString = "off";

  ArPose raw = myRobot->getRawEncoderPose();

  if (myGyro != NULL && myGyro->haveGottenData() && 
      !myGyro->hasNoInternalData())
    printf("\r%7.0f%7.0f%9.2f%7.0f%8.2f%7d%8s%6s%10.2f%10.2f %10.2f,%.2f,%.2f", 
	   myRobot->getX(),  myRobot->getY(), myRobot->getTh(), 
	   myRobot->getCompass(), voltage,
	   myRobot->getMotorPacCount(),
	   myMode == MODE_BOTH ? "both" : "either", 
	   gyroString.c_str(),
	   ArMath::subAngle(myGyro->getHeading(), myGyroZero), 
	   ArMath::subAngle(myRobot->getRawEncoderPose().getTh(),myRobotZero),
	   raw.getX(), raw.getY(), raw.getTh()
	);
  else if (myGyro != NULL && myGyro->haveGottenData() && 
      myGyro->hasNoInternalData())
    printf("\r%7.0f%7.0f%9.2f%7.0f%8.2f%7d%8s%6s%10.2f,%.2f,%.2f", 
	   myRobot->getX(),  myRobot->getY(), myRobot->getTh(), 
	   myRobot->getCompass(), voltage,
	   myRobot->getMotorPacCount(),
	   myMode == MODE_BOTH ? "both" : "either", 
	   gyroString.c_str(),
	   raw.getX(), raw.getY(), raw.getTh()

  );
  else
    printf("\r%7.0f%7.0f%9.2f%7.0f%8.2f%7d%8s%10.2f,%.2f,%.2f", myRobot->getX(), 
	   myRobot->getY(), myRobot->getTh(), myRobot->getCompass(), 
	   voltage, myRobot->getMotorPacCount(), 
	   myMode == MODE_BOTH ? "both" : "either",
	   raw.getX(), raw.getY(), raw.getTh()
    );
  fflush(stdout);
}

AREXPORT ArModeIO::ArModeIO(ArRobot *robot, const char *name, char key, char key2): 
  ArMode(robot, name, key, key2),
  myTog1CB(this, &ArModeIO::toggleOutput, 1),
  myTog2CB(this, &ArModeIO::toggleOutput, 2),
  myTog3CB(this, &ArModeIO::toggleOutput, 3),
  myTog4CB(this, &ArModeIO::toggleOutput, 4),
  myTog5CB(this, &ArModeIO::toggleOutput, 5),
  myTog6CB(this, &ArModeIO::toggleOutput, 6),
  myTog7CB(this, &ArModeIO::toggleOutput, 7),
  myTog8CB(this, &ArModeIO::toggleOutput, 8)
{
}

AREXPORT ArModeIO::~ArModeIO()
{
  
}

AREXPORT void ArModeIO::activate(void)
{
  if (!baseActivate())
    return;
  if (myRobot == NULL)
    return;
  myRobot->comInt(ArCommands::IOREQUEST, 2);
  myOutput[0] = '\0';
  myLastPacketTime = myRobot->getIOPacketTime();
  addKeyHandler('1', &myTog1CB);
  addKeyHandler('2', &myTog2CB);
  addKeyHandler('3', &myTog3CB);
  addKeyHandler('4', &myTog4CB);
  addKeyHandler('5', &myTog5CB);
  addKeyHandler('6', &myTog6CB);
  addKeyHandler('7', &myTog7CB);
  addKeyHandler('8', &myTog8CB);
}

AREXPORT void ArModeIO::deactivate(void)
{
  if (!baseDeactivate())
    return;
  if (myRobot == NULL)
    return;
  myRobot->comInt(ArCommands::IOREQUEST, 0);
  remKeyHandler(&myTog1CB);
  remKeyHandler(&myTog2CB);
  remKeyHandler(&myTog3CB);
  remKeyHandler(&myTog4CB);
  remKeyHandler(&myTog5CB);
  remKeyHandler(&myTog6CB);
  remKeyHandler(&myTog7CB);
  remKeyHandler(&myTog8CB);
}

AREXPORT void ArModeIO::help(void)
{
  ArLog::log(ArLog::Terse, 
	     "IO mode shows the IO (digin, digout, a/d) from the robot.");
  myExplanationReady = false;
  myExplained = false;
}

void ArModeIO::toggleOutput(int which)
{
  printf("toggling output %d. Current output is %s\n", which, byte_as_bitstring(myRobot->getDigOut()).c_str());
  ArTypes::UByte mask = (1 << (which-1));
  ArTypes::UByte bits = ~(myRobot->getDigOut());
  printf("-> DIGOUT %s %s\n", byte_as_bitstring(mask).c_str(), byte_as_bitstring(bits).c_str());
  myRobot->com2Bytes(ArCommands::DIGOUT, mask, bits);
}

AREXPORT void ArModeIO::userTask(void)
{
  int num;
  int i, j;
  unsigned int value;
  int bit;
  char label[256];
  myOutput[0] = '\0';

  //if (myLastPacketTime.mSecSince(myRobot->getIOPacketTime()) == 0)
  //  return;

  if (!myExplanationReady)
    myExplanation[0] = '\0';

  value = myRobot->getFlags();
  if (!myExplanationReady)
  {
    sprintf(label, "flags");
    sprintf(myExplanation, "%s%17s  ", myExplanation, label);
  }
  for (j = 0, bit = 1; j < 16; ++j, bit *= 2)
  {
    if (j == 8)
      sprintf(myOutput, "%s ", myOutput);
    if (value & bit)
      sprintf(myOutput, "%s%d", myOutput, 1);
    else
      sprintf(myOutput, "%s%d", myOutput, 0);
  }
  sprintf(myOutput, "%s  ", myOutput);

  if (myRobot->hasFaultFlags())
  {
    value = myRobot->getFaultFlags();
    if (!myExplanationReady)
    {
      sprintf(label, "fault_flags");
      sprintf(myExplanation, "%s%17s  ", myExplanation, label);
    }
    for (j = 0, bit = 1; j < 16; ++j, bit *= 2)
    {
      if (j == 8)
        sprintf(myOutput, "%s ", myOutput);
      if (value & bit)
        sprintf(myOutput, "%s%d", myOutput, 1);
      else
        sprintf(myOutput, "%s%d", myOutput, 0);
    }
    sprintf(myOutput, "%s  ", myOutput);
  }

  num = myRobot->getIODigInSize();
  for (i = 0; i < num; ++i)
  {
    value = myRobot->getIODigIn(i);
    if (!myExplanationReady)
    {
      sprintf(label, "digin%d", i);
      sprintf(myExplanation, "%s%8s  ", myExplanation, label);
    }
    for (j = 0, bit = 1; j < 8; ++j, bit *= 2)
    {
      if (value & bit)
        sprintf(myOutput, "%s%d", myOutput, 1);
      else
        sprintf(myOutput, "%s%d", myOutput, 0);
    }
    sprintf(myOutput, "%s  ", myOutput);
  }
  if(num == 0) // use default Pioneer IO from SIP only if no IODigIns
  {
    value = myRobot->getDigIn();
    if(!myExplanationReady)
    {
      sprintf(label, "digin");
      sprintf(myExplanation, "%s%8s  ", myExplanation, label);
    }
    for (j = 0, bit = 1; j < 8; ++j, bit *= 2)
    {
      if (value & bit)
        sprintf(myOutput, "%s%d", myOutput, 1);
      else
        sprintf(myOutput, "%s%d", myOutput, 0);
    }
    sprintf(myOutput, "%s  ", myOutput);
  }


  num = myRobot->getIODigOutSize();
  for (i = 0; i < num; ++i)
  {
    value = myRobot->getIODigOut(i);
    if (!myExplanationReady)
    {
      sprintf(label, "digout%d", i);
      sprintf(myExplanation, "%s%8s  ", myExplanation, label);
    }
    for (j = 0, bit = 1; j < 8; ++j, bit *= 2)
    {
      if (value & bit)
        sprintf(myOutput, "%s%d", myOutput, 1);
      else
        sprintf(myOutput, "%s%d", myOutput, 0);
    }
    sprintf(myOutput, "%s  ", myOutput);
  }
  if(num == 0) // use default Pioneer IO from SIP only if no IODigIns
  {
    value = myRobot->getDigOut();
    if(!myExplanationReady)
    {
      sprintf(label, "digout");
      sprintf(myExplanation, "%s%8s  ", myExplanation, label);
    }
    for (j = 0, bit = 1; j < 8; ++j, bit *= 2)
    {
      if (value & bit)
        sprintf(myOutput, "%s%d", myOutput, 1);
      else
        sprintf(myOutput, "%s%d", myOutput, 0);
    }
    sprintf(myOutput, "%s  ", myOutput);
  }

  num = myRobot->getIOAnalogSize();
  for (i = 0; i < num; ++i)
  {
    if (!myExplanationReady)
    {
      sprintf(label, "a/d%d", i);
      sprintf(myExplanation, "%s%6s", myExplanation, label);
    }
    
    /*
      int ad = myRobot->getIOAnalog(i);
      double adVal;
    ad &= 0xfff;
    adVal = ad * .0048828;
    sprintf(myOutput, "%s%6.2f", myOutput,adVal);
    */
    sprintf(myOutput, "%s%6.2f", myOutput, myRobot->getIOAnalogVoltage(i));
    
  }

  if (!myExplained)
  {
    printf("Robot has %d input bits, %d output bits, %d analog io. Robot %s have fault flags.\n",
      myRobot->getIODigInSize(),
      myRobot->getIODigOutSize(),
      myRobot->getIOAnalogSize(),
      myRobot->hasFaultFlags() ? "does" : "does not"
    );
    printf("\n%s\n", myExplanation);
    myExplained = true;
  }

  printf("\r%s", myOutput);
  fflush(stdout);
}

AREXPORT ArModeLaser::ArModeLaser(ArRobot *robot, const char *name, 
				  char key, char key2, ArSick *obsolete) :
  ArMode(robot, name, key, key2),
  myTogMiddleCB(this, &ArModeLaser::togMiddle)
{
  myPrintMiddle = false;

  ArLaser *laser;
  int i;
  for (i = 1; i <= 10; i++)
  {
    if ((laser = myRobot->findLaser(i)) != NULL)
    {
      myLaserCallbacks[i] = new ArFunctor1C<ArModeLaser, 
      int>(this, &ArModeLaser::switchToLaser, i),
      myLasers[i] = laser;
    }
  }
  
  myLaser = NULL;
  myState = STATE_UNINITED;
}

AREXPORT ArModeLaser::~ArModeLaser()
{
}

AREXPORT void ArModeLaser::activate(void)
{
  // this is here because there needs to be the laser set up for the
  // help to work right
  std::map<int, ArLaser *>::iterator it;
  if (myLaser == NULL)
  {
    if ((it = myLasers.begin()) == myLasers.end())
    {
      ArLog::log(ArLog::Normal, "Laser mode tried to activate, but has no lasers");
    }
    else
    {
      myLaser = (*it).second;
      myLaserNumber = (*it).first;
    }
  }


  bool alreadyActive = false;

  if (ourActiveMode == this)
    alreadyActive = true;

  if (!alreadyActive && !baseActivate())
    return;

  if (myRobot == NULL)
  {
    ArLog::log(ArLog::Verbose, "Laser mode activated but there is no robot.");
    return;
  }

  if (myLaser == NULL)
  {
    ArLog::log(ArLog::Verbose, "Laser mode activated but there are no lasers.");
	return;
  }

  if (!alreadyActive)
  {
    
    addKeyHandler('z', &myTogMiddleCB);
    addKeyHandler('Z', &myTogMiddleCB);
    
    std::map<int, ArFunctor1C<ArModeLaser, int> *>::iterator kIt;
    for (kIt = myLaserCallbacks.begin(); kIt != myLaserCallbacks.end(); kIt++)
    {
      if ((*kIt).first >= 1 || (*kIt).first <= 9)
	addKeyHandler('0' + (*kIt).first, (*kIt).second);
    }   
  }

  if (myState == STATE_UNINITED)
  {
    myLaser->lockDevice();
    if (myLaser->isConnected())
    {
      ArLog::log(ArLog::Verbose, 
		 "\nArModeLaser using already existing and connected laser.");
      myState = STATE_CONNECTED;
    }
    else if (myLaser->isTryingToConnect())
    {
      ArLog::log(ArLog::Terse, "\nArModeLaser already connecting to %s.",
		 myLaser->getName());
    }
    else
    {
      ArLog::log(ArLog::Terse,
		 "\nArModeLaser is connecting to %s.",
		 myLaser->getName());
      myLaser->asyncConnect();
      myState = STATE_CONNECTING;
    }
    myLaser->unlockDevice();
  } 
}

AREXPORT void ArModeLaser::deactivate(void)
{
  if (!baseDeactivate())
    return;

  remKeyHandler(&myTogMiddleCB);
  
  std::map<int, ArFunctor1C<ArModeLaser, int> *>::iterator it;
  for (it = myLaserCallbacks.begin(); it != myLaserCallbacks.end(); it++)
  {
    remKeyHandler((*it).second);
  }   
}

AREXPORT void ArModeLaser::help(void)
{
  if (myLaser == NULL)
  {
    ArLog::log(ArLog::Terse, 
	       "There are no lasers, this mode cannot do anything");
    return;
  }

  ArLog::log(ArLog::Terse, 
	     "Laser mode connects to a laser, or uses a previously established connection.");
  ArLog::log(ArLog::Terse,
	     "Laser mode then displays the closest and furthest reading from the laser.");
  ArLog::log(ArLog::Terse, "%13s:  toggle between far reading and middle reading with reflectivity", "'z' or 'Z'");

  std::map<int, ArFunctor1C<ArModeLaser, int> *>::iterator it;
  for (it = myLaserCallbacks.begin(); it != myLaserCallbacks.end(); it++)
  {
    ArLog::log(ArLog::Terse, "%13d:  %s", (*it).first, 
	       myLasers[(*it).first]->getName());
  }   
}


AREXPORT void ArModeLaser::userTask(void)
{
  double dist = HUGE_VAL, angle = -1;
  int reflec = -1;
  double midDist = HUGE_VAL, midAngle = -1;
  int midReflec = -1;
  double farDist = -HUGE_VAL, farAngle = -1;

  if (myRobot == NULL || myLaser == NULL)
    return;


  if (myState == STATE_CONNECTED && !myPrintMiddle)
  {
    const std::list<ArPoseWithTime *> *readings;
    std::list<ArPoseWithTime *>::const_iterator it;
    bool found = false;
  
    myLaser->lockDevice();
    if (!myLaser->isConnected())
    {
      ArLog::log(ArLog::Terse, "\n\nLaser mode lost connection to the laser.");
      ArLog::log(ArLog::Terse, "Select that laser or laser mode again to try reconnecting to the laser.\n");
      myState = STATE_UNINITED;
    }
    dist = myLaser->currentReadingPolar(-90, 90, &angle);
    if (dist < myLaser->getMaxRange())
      printf("\rClose: %8.0fmm %5.1f deg   ", dist, angle);
    else
      printf("\rNo close reading.         ");

    readings = myLaser->getCurrentBuffer();
    for (it = readings->begin(), found = false; it != readings->end(); it++)
    {
      dist = myRobot->findDistanceTo(*(*it));
      angle = myRobot->findDeltaHeadingTo(*(*it));
      if (!found || dist > farDist)
      {
	found = true;
	farDist = dist;
	farAngle = angle;
      }
    }
    if (found)
      printf("Far: %8.0fmm %5.1f deg", 
	     farDist, farAngle);
    else
      printf("No far reading found");
    printf("         %lu readings   ", readings->size());
    myLaser->unlockDevice();
    fflush(stdout);
  }
  else if (myState == STATE_CONNECTED && myPrintMiddle)
  {
    const std::list<ArSensorReading *> *rawReadings;
    std::list<ArSensorReading *>::const_iterator rawIt;
    myLaser->lockDevice();
    if (!myLaser->isConnected())
    {
      ArLog::log(ArLog::Terse, "\n\nLaser mode lost connection to the laser.");
      ArLog::log(ArLog::Terse, "Switch out of this mode and back if you want to try reconnecting to the laser.\n");
      myState = STATE_UNINITED;
    }
    rawReadings = myLaser->getRawReadings();
    int middleReading = rawReadings->size() / 2;
    if (rawReadings->begin() != rawReadings->end())
    {
      int i;
      for (rawIt = rawReadings->begin(), i = 0; 
	   rawIt != rawReadings->end(); 
	   rawIt++, i++)
      {
	if ((*rawIt)->getIgnoreThisReading())
	  continue;
	if (rawIt == rawReadings->begin() || 
	    (*rawIt)->getRange() < dist)
	{
	  dist = (*rawIt)->getRange();
	  angle = (*rawIt)->getSensorTh();
	  reflec = (*rawIt)->getExtraInt();
	}
	if (i == middleReading)
	{
	  midDist = (*rawIt)->getRange();
	  midAngle = (*rawIt)->getSensorTh();
	  midReflec = (*rawIt)->getExtraInt();
	}
      }
      printf(
      "\rClose: %8.0fmm %5.1f deg %d refl          Middle: %8.0fmm %5.1fdeg, %d refl", 
	      dist, angle, reflec, midDist, midAngle, midReflec);
    }
    else
      printf("\rNo readings");
    myLaser->unlockDevice(); 
  }
  else if (myState == STATE_CONNECTING)
  {
    myLaser->lockDevice();
    if (myLaser->isConnected())
    {
      ArLog::log(ArLog::Terse, "\nLaser mode has connected to the laser.\n");
      myState = STATE_CONNECTED;
    }
    else if (!myLaser->isTryingToConnect())
    {
      ArLog::log(ArLog::Terse, "\nLaser mode failed to connect to the laser.\n");
      ArLog::log(ArLog::Terse, 
		 "Switch out of this mode and back to try reconnecting.\n");
      myState = STATE_UNINITED;
    }
    myLaser->unlockDevice();
  }
}


void ArModeLaser::togMiddle(void)
{
  myPrintMiddle = !myPrintMiddle;
}

AREXPORT void ArModeLaser::switchToLaser(int laserNumber)
{
  if (laserNumber == myLaserNumber && myLaser->isConnected())
  {
    ArLog::log(ArLog::Verbose, 
	       "ArModeLaser::switchToLaser: Already on laser %s", myLaser->getName());
    return;
  }

  std::map<int, ArLaser *>::iterator it;
  if ((it = myLasers.find(laserNumber)) == myLasers.end())
  {
    ArLog::log(ArLog::Normal, "ArModeLaser::switchToLaser: told to switch to laser %d but that laser does not exist"); 
    return;
  }
  myLaser = (*it).second;
  ArLog::log(ArLog::Normal, "\r\n\nSwitching to laser %s\n", 
	     myLaser->getName()); 
  myState = STATE_UNINITED;
  myLaserNumber = laserNumber;

  activate();
}

/**
  @param robot ArRobot instance to be associate with
  @param name name of this mode
  @param key keyboard key that activates this mode
  @param key2 another keyboard key that activates this mode
  @param acts ArACTS_1_2 instance to use. If not given, then an internally
maintained instance is created by ArModeActs.
 **/
AREXPORT ArModeActs::ArModeActs(ArRobot *robot, const char *name, char key, 
				char key2, ArACTS_1_2 *acts): 
  ArMode(robot, name, key, key2),
  myChannel1CB(this, &ArModeActs::channel1),
  myChannel2CB(this, &ArModeActs::channel2),
  myChannel3CB(this, &ArModeActs::channel3),
  myChannel4CB(this, &ArModeActs::channel4),
  myChannel5CB(this, &ArModeActs::channel5),
  myChannel6CB(this, &ArModeActs::channel6),
  myChannel7CB(this, &ArModeActs::channel7),
  myChannel8CB(this, &ArModeActs::channel8),
  myStopCB(this, &ArModeActs::stop),
  myStartCB(this, &ArModeActs::start),
  myToggleAcquireCB(this, &ArModeActs::toggleAcquire)
{
  if (acts != NULL)
    myActs = acts;
  else
    myActs = new ArACTS_1_2;
  myRobot = robot;
  myActs->openPort(myRobot);
  myGroup = new ArActionGroupColorFollow(myRobot, myActs, camera);
  myGroup->deactivate();
}

// Destructor
AREXPORT ArModeActs::~ArModeActs()
{
  
}

// Activate the mode
AREXPORT void ArModeActs::activate(void)
{
  // Activate the group
  if (!baseActivate())
    return;
  myGroup->activateExclusive();
  
  // Add key handlers for keyboard input
  addKeyHandler(ArKeyHandler::SPACE, &myStopCB);
  addKeyHandler('z', &myStartCB);
  addKeyHandler('Z', &myStartCB);
  addKeyHandler('x', &myToggleAcquireCB);
  addKeyHandler('X', &myToggleAcquireCB);
  addKeyHandler('1', &myChannel1CB);
  addKeyHandler('2', &myChannel2CB);
  addKeyHandler('3', &myChannel3CB);
  addKeyHandler('4', &myChannel4CB);
  addKeyHandler('5', &myChannel5CB);
  addKeyHandler('6', &myChannel6CB);
  addKeyHandler('7', &myChannel7CB);
  addKeyHandler('8', &myChannel8CB);

  // Set the camera
  camera = myRobot->getPTZ();
  
  // Tell us whether we are connected to ACTS or not
  if(myActs->isConnected())
    { 
      printf("\nConnected to ACTS.\n");
    }
  else printf("\nNot connected to ACTS.\n");

  // Tell us whether a camera is defined or not
  if(camera != NULL)
    {
      printf("\nCamera defined.\n\n");
      myGroup->setCamera(camera);
    }
  else
    {  
      printf("\nNo camera defined.\n");
      printf("The robot will not tilt its camera up or down until\n");
      printf("a camera has been defined in camera mode ('c' or 'C').\n\n");
    }
}

// Deactivate the group
AREXPORT void ArModeActs::deactivate(void)
{
  if (!baseDeactivate())
    return;

  // Remove the key handlers
  remKeyHandler(&myStopCB);
  remKeyHandler(&myStartCB);
  remKeyHandler(&myToggleAcquireCB);
  remKeyHandler(&myChannel1CB);
  remKeyHandler(&myChannel2CB);
  remKeyHandler(&myChannel3CB);
  remKeyHandler(&myChannel4CB);
  remKeyHandler(&myChannel5CB);
  remKeyHandler(&myChannel6CB);
  remKeyHandler(&myChannel7CB);
  remKeyHandler(&myChannel8CB);

  myGroup->deactivate();
}

// Display the available commands
AREXPORT void ArModeActs::help(void)
{
  ArLog::log(ArLog::Terse, 
	   "ACTS mode will drive the robot in an attempt to follow a color blob.\n");

  ArLog::log(ArLog::Terse, "%20s:  Pick a channel",     "1 - 8    ");
  ArLog::log(ArLog::Terse, "%20s:  toggle acquire mode", "'x' or 'X'");
  ArLog::log(ArLog::Terse, "%20s:  start movement",     "'z' or 'Z'");
  ArLog::log(ArLog::Terse, "%20s:  stop movement",      "space bar");
  ArLog::log(ArLog::Terse, "");
  
}

// Display data about this mode
AREXPORT void ArModeActs::userTask(void)
{
  int myChannel;

  const char *acquire;
  const char *move;
  const char *blob;

  myChannel = myGroup->getChannel();
  if(myGroup->getAcquire()) acquire = "actively acquiring";
  else acquire = "passively acquiring";
  
  if(myGroup->getMovement()) move = "movement on";
  else move = "movement off";

  if(myGroup->getBlob()) blob = "blob in sight";
  else blob = "no blob in sight";

  printf("\r Channel: %d  %15s %25s %20s", myChannel, move, acquire, blob);
  fflush(stdout);
}

// The channels
AREXPORT void ArModeActs::channel1(void)
{
  myGroup->setChannel(1);
}

AREXPORT void ArModeActs::channel2(void)
{
  myGroup->setChannel(2);
}

AREXPORT void ArModeActs::channel3(void)
{
  myGroup->setChannel(3);
}

AREXPORT void ArModeActs::channel4(void)
{
  myGroup->setChannel(4);
}

AREXPORT void ArModeActs::channel5(void)
{
  myGroup->setChannel(5);
}

AREXPORT void ArModeActs::channel6(void)
{
  myGroup->setChannel(6);
}

AREXPORT void ArModeActs::channel7(void)
{
  myGroup->setChannel(7);
}

AREXPORT void ArModeActs::channel8(void)
{
  myGroup->setChannel(8);
}

// Stop the robot from moving
AREXPORT void ArModeActs::stop(void)
{
  myGroup->stopMovement();
}

// Allow the robot to move
AREXPORT void ArModeActs::start(void)
{
  myGroup->startMovement();
}

// Toggle whether or not the robot is allowed
// to aquire anything
AREXPORT void ArModeActs::toggleAcquire()
{
  if(myGroup->getAcquire())
    myGroup->setAcquire(false);
  else myGroup->setAcquire(true);

}

AREXPORT ArModeCommand::ArModeCommand(ArRobot *robot, const char *name, char key, char key2): 
  ArMode(robot, name, key, key2),
  my0CB(this, &ArModeCommand::addChar, '0'),
  my1CB(this, &ArModeCommand::addChar, '1'),
  my2CB(this, &ArModeCommand::addChar, '2'),
  my3CB(this, &ArModeCommand::addChar, '3'),
  my4CB(this, &ArModeCommand::addChar, '4'),
  my5CB(this, &ArModeCommand::addChar, '5'),
  my6CB(this, &ArModeCommand::addChar, '6'),
  my7CB(this, &ArModeCommand::addChar, '7'),
  my8CB(this, &ArModeCommand::addChar, '8'),
  my9CB(this, &ArModeCommand::addChar, '9'),
  myMinusCB(this, &ArModeCommand::addChar, '-'),
  myBackspaceCB(this, &ArModeCommand::addChar, ArKeyHandler::BACKSPACE),
  mySpaceCB(this, &ArModeCommand::addChar, ArKeyHandler::SPACE),
  myEnterCB(this, &ArModeCommand::finishParsing)

{
  reset(false);
}

AREXPORT ArModeCommand::~ArModeCommand()
{
  
}

AREXPORT void ArModeCommand::activate(void)
{
  reset(false);
  if (!baseActivate())
    return;
  myRobot->stopStateReflection();
  takeKeys();
  reset(true);
}

AREXPORT void ArModeCommand::deactivate(void)
{
  if (!baseDeactivate())
    return;
  giveUpKeys();
}

AREXPORT void ArModeCommand::help(void)
{
  
  ArLog::log(ArLog::Terse, "Command mode has three ways to send commands");
  ArLog::log(ArLog::Terse, "%-30s: Sends com(<command>)", "<command>");
  ArLog::log(ArLog::Terse, "%-30s: Sends comInt(<command>, <integer>)", "<command> <integer>");
  ArLog::log(ArLog::Terse, "%-30s: Sends com2Bytes(<command>, <byte1>, <byte2>)", "<command> <byte1> <byte2>");
}

void ArModeCommand::addChar(int ch)
{
  if (ch < '0' && ch > '9' && ch != '-' && ch != ArKeyHandler::BACKSPACE && 
      ch != ArKeyHandler::SPACE)
  {
    ArLog::log(ArLog::Terse, "Something horribly wrong in command mode since number is < 0 || > 9 (it is the value %d)", ch);
    return;
  }

  size_t size = sizeof(myCommandString);
  size_t len = strlen(myCommandString);

  if (ch == ArKeyHandler::BACKSPACE)
  {
    // don't overrun backwards
    if (len < 1)
      return;
    myCommandString[len-1] = '\0';
    printf("\r> %s  \r> %s", myCommandString, myCommandString);
    return;
  }
  if (ch == ArKeyHandler::SPACE)
  {
    // if we're at the start or have a space or - just return
    if (len < 1 || myCommandString[len-1] == ' ' || 
	myCommandString[len-1] == '-')
      return;
    myCommandString[len] = ' ';
    myCommandString[len+1] = '\0';
    printf(" ");
    return;
  }
  if (ch == '-')
  {
    // make sure it isn't the command trying to be negated or that its the start of the byte
    if (len < 1 || myCommandString[len-1] != ' ')
      return;
    printf("%c", '-');
    myCommandString[len] = '-';
    myCommandString[len+1] = '\0';
    return;
  }
  if (len + 1 >= size)
  {
    printf("\n");
    ArLog::log(ArLog::Terse, "Command is too long, abandoning command");
    reset();
    return;
  }
  else
  {
    printf("%c", ch);
    myCommandString[len] = ch;
    myCommandString[len+1] = '\0';
    return;
  }
}

void ArModeCommand::finishParsing(void)
{
  
  ArArgumentBuilder builder;
  builder.addPlain(myCommandString);
  int command;
  int int1;
  int int2;

  if (myCommandString[0] == '\0')
    return;

  printf("\n");
  if (builder.getArgc() == 0)
  {
    ArLog::log(ArLog::Terse, "Syntax error, no arguments.");
  }
  if (builder.getArgc() == 1)
  {
    command = builder.getArgInt(0);
    if (command < 0 || command > 255 || !builder.isArgInt(0))
    {
      ArLog::log(ArLog::Terse, 
		 "Invalid command, must be an integer between 0 and 255");
      reset();
      return;
    }
    else
    {
      ArLog::log(ArLog::Terse, "com(%d)", command);
      myRobot->com(command);
      reset();
      return;
    }
  }
  else if (builder.getArgc() == 2)
  {
    command = builder.getArgInt(0);
    int1 = builder.getArgInt(1);
    if (command < 0 || command > 255 || !builder.isArgInt(0))
    {
      ArLog::log(ArLog::Terse, 
		 "Invalid command, must be an integer between 0 and 255");
      reset();
      return;
    }
    else if (int1 < -32767 || int1 > 32767 || !builder.isArgInt(1))
    {
      ArLog::log(ArLog::Terse, 
	 "Invalid integer, must be an integer between -32767 and 32767");
      reset();
      return;
    }
    else
    {
      ArLog::log(ArLog::Terse, "comInt(%d, %d)", command,
		 int1);
      myRobot->comInt(command, int1);
      reset();
      return;
    }
  }
  else if (builder.getArgc() == 3)
  {
    command = builder.getArgInt(0);
    int1 = builder.getArgInt(1);
    int2 = builder.getArgInt(2);
    if (command < 0 || command > 255 || !builder.isArgInt(0))
    {
      ArLog::log(ArLog::Terse, 
		 "Invalid command, must be between 0 and 255");
      reset();
      return;
    }
    else if (int1 < -128 || int1 > 255 || !builder.isArgInt(1))
    {
      ArLog::log(ArLog::Terse, 
	 "Invalid byte1, must be an integer between -128 and 127, or between 0 and 255");
      reset();
      return;
    }
    else if (int2 < -128 || int2 > 255 || !builder.isArgInt(2))
    {
      ArLog::log(ArLog::Terse, 
	 "Invalid byte2, must be an integer between -128 and 127, or between 0 and 255");
      reset();
      return;
    }
    else
    {
      ArLog::log(ArLog::Terse, 
		 "com2Bytes(%d, %d, %d)", 
		 command, int1, int2);
      myRobot->com2Bytes(command, int1, int2);
      reset();
      return;
    }
  }
  else
  {
    ArLog::log(ArLog::Terse, "Syntax error, too many arguments");
    reset();
    return;
  }
}

void ArModeCommand::reset(bool print)
{
  myCommandString[0] = '\0';
  if (print)
  {
    ArLog::log(ArLog::Terse, "");
    printf("> ");
  }
}

void ArModeCommand::takeKeys(void)
{
  addKeyHandler('0', &my0CB);
  addKeyHandler('1', &my1CB);
  addKeyHandler('2', &my2CB);
  addKeyHandler('3', &my3CB);
  addKeyHandler('4', &my4CB);
  addKeyHandler('5', &my5CB);
  addKeyHandler('6', &my6CB);
  addKeyHandler('7', &my7CB);
  addKeyHandler('8', &my8CB);
  addKeyHandler('9', &my9CB);
  addKeyHandler('-', &myMinusCB);
  addKeyHandler(ArKeyHandler::BACKSPACE, &myBackspaceCB);
  addKeyHandler(ArKeyHandler::ENTER, &myEnterCB);
  addKeyHandler(ArKeyHandler::SPACE, &mySpaceCB);
}

void ArModeCommand::giveUpKeys(void)
{
  remKeyHandler(&my0CB);
  remKeyHandler(&my1CB);
  remKeyHandler(&my2CB);
  remKeyHandler(&my3CB);
  remKeyHandler(&my4CB);
  remKeyHandler(&my5CB);
  remKeyHandler(&my6CB);
  remKeyHandler(&my7CB);
  remKeyHandler(&my8CB);
  remKeyHandler(&my9CB);
  remKeyHandler(&myBackspaceCB);
  remKeyHandler(&myMinusCB);
  remKeyHandler(&myEnterCB);
  remKeyHandler(&mySpaceCB);
}

/**
  @param robot ArRobot instance to be associate with
  @param name name of this mode
  @param key keyboard key that activates this mode
  @param key2 another keyboard key that activates this mode
   @param tcm2 if a tcm2 class is passed in it'll use that instance
   otherwise it'll make its own ArTCMCompassRobot instance.
**/

AREXPORT ArModeTCM2::ArModeTCM2(ArRobot *robot, const char *name, char key, char key2, ArTCM2 *tcm2): 
  ArMode(robot, name, key, key2)
{
  if (tcm2 != NULL)
    myTCM2 = tcm2;
  else
    myTCM2 = new ArTCMCompassRobot(robot);

  myOffCB = new ArFunctorC<ArTCM2>(myTCM2, &ArTCM2::commandOff);
  myCompassCB = new ArFunctorC<ArTCM2>(myTCM2, &ArTCM2::commandJustCompass);
  myOnePacketCB = new ArFunctorC<ArTCM2>(myTCM2, &ArTCM2::commandOnePacket);
  myContinuousPacketsCB = new ArFunctorC<ArTCM2>(
	  myTCM2, &ArTCM2::commandContinuousPackets);
  myUserCalibrationCB = new ArFunctorC<ArTCM2>(
	  myTCM2, &ArTCM2::commandUserCalibration);
  myAutoCalibrationCB = new ArFunctorC<ArTCM2>(
	  myTCM2, &ArTCM2::commandAutoCalibration);
  myStopCalibrationCB = new ArFunctorC<ArTCM2>(
	  myTCM2, &ArTCM2::commandStopCalibration);
  myResetCB = new ArFunctorC<ArTCM2>(
	  myTCM2, &ArTCM2::commandSoftReset);

}

AREXPORT ArModeTCM2::~ArModeTCM2()
{
  
}

AREXPORT void ArModeTCM2::activate(void)
{
  if (!baseActivate())
    return;
  myTCM2->commandContinuousPackets();
  addKeyHandler('0', myOffCB);
  addKeyHandler('1', myCompassCB);
  addKeyHandler('2', myOnePacketCB);
  addKeyHandler('3', myContinuousPacketsCB);
  addKeyHandler('4', myUserCalibrationCB);
  addKeyHandler('5', myAutoCalibrationCB);
  addKeyHandler('6', myStopCalibrationCB);
  addKeyHandler('7', myResetCB);
}

AREXPORT void ArModeTCM2::deactivate(void)
{
  if (!baseDeactivate())
    return;
  myTCM2->commandJustCompass();
  remKeyHandler(myOffCB);
  remKeyHandler(myCompassCB);
  remKeyHandler(myOnePacketCB);
  remKeyHandler(myContinuousPacketsCB);
  remKeyHandler(myUserCalibrationCB);
  remKeyHandler(myAutoCalibrationCB);
  remKeyHandler(myStopCalibrationCB);
  remKeyHandler(myResetCB);
}

AREXPORT void ArModeTCM2::help(void)
{
  ArLog::log(ArLog::Terse, 
	     "TCM2 mode shows the data from the TCM2 compass and lets you send the TCM2 commands");
  ArLog::log(ArLog::Terse, "%20s:  turn TCM2 off", "'0'");  
  ArLog::log(ArLog::Terse, "%20s:  just get compass readings", "'1'");  
  ArLog::log(ArLog::Terse, "%20s:  get a single set of TCM2 data", "'2'");  
  ArLog::log(ArLog::Terse, "%20s:  get continuous TCM2 data", "'3'");  
  ArLog::log(ArLog::Terse, "%20s:  start user calibration", "'4'");  
  ArLog::log(ArLog::Terse, "%20s:  start auto calibration", "'5'");  
  ArLog::log(ArLog::Terse, "%20s:  stop calibration and get a single set of data", "'6'");  
  ArLog::log(ArLog::Terse, "%20s:  soft reset of compass", "'7'");  

  printf("%6s %5s %5s %6s %6s %6s %6s %10s %4s %4s %6s %3s\n", 
	 "comp", "pitch", "roll", "magX", "magY", "magZ", "temp", "error",
	 "calH", "calV", "calM", "cnt");
}

AREXPORT void ArModeTCM2::userTask(void)
{
  printf("\r%6.1f %5.1f %5.1f %6.2f %6.2f %6.2f %6.1f 0x%08x %4.0f %4.0f %6.2f %3d", 
	 myTCM2->getCompass(), myTCM2->getPitch(), myTCM2->getRoll(), 
	 myTCM2->getXMagnetic(), myTCM2->getYMagnetic(), 
	 myTCM2->getZMagnetic(), 
	 myTCM2->getTemperature(), myTCM2->getError(), 
	 myTCM2->getCalibrationH(), myTCM2->getCalibrationV(), 
	 myTCM2->getCalibrationM(), myTCM2->getPacCount());
  fflush(stdout);

}

AREXPORT ArModeConfig::ArModeConfig(ArRobot *robot, const char *name, char key1, char key2) :
  ArMode(robot, name, key1, key2),
  myRobot(robot),
  myConfigPacketReader(robot, false, &myGotConfigPacketCB),
  myGotConfigPacketCB(this, &ArModeConfig::gotConfigPacket)
{
}

AREXPORT void ArModeConfig::help()
{
  ArLog::log(ArLog::Terse, "Robot Config mode requests a CONFIG packet from the robot and displays the result.");
}

AREXPORT void ArModeConfig::activate()
{
  baseActivate();  // returns false on double activate, but we want to use this signal to request another config packet, so ignore.
  if(!myConfigPacketReader.requestPacket())
    ArLog::log(ArLog::Terse, "ArModeConfig: Warning: config packet reader did not request (another) CONFIG packet.");
}

AREXPORT void ArModeConfig::deactivate()
{
}

void ArModeConfig::gotConfigPacket()
{
  ArLog::log(ArLog::Terse, "\nRobot CONFIG packet received:");
  myConfigPacketReader.log();
  myConfigPacketReader.logMovement();
  ArLog::log(ArLog::Terse, "Additional robot information:");
  ArLog::log(ArLog::Terse, "HasStateOfCharge %d", myRobot->haveStateOfCharge());
  ArLog::log(ArLog::Terse, "StateOfChargeLow %f", myRobot->getStateOfChargeLow());
  ArLog::log(ArLog::Terse, "StateOfChargeShutdown %f", myRobot->getStateOfChargeShutdown());
  ArLog::log(ArLog::Terse, "HasFaultFlags %d", myRobot->hasFaultFlags());
  ArLog::log(ArLog::Terse, "HasTableIR %d", myRobot->hasTableSensingIR());
  ArLog::log(ArLog::Terse, "NumSonar (rec'd) %d", myRobot->getNumSonar());
  ArLog::log(ArLog::Terse, "HasTemperature (rec'd) %d", myRobot->hasTemperature());
  ArLog::log(ArLog::Terse, "HasSettableVelMaxes %d", myRobot->hasSettableVelMaxes());
  ArLog::log(ArLog::Terse, "HasSettableAccsDecs %d", myRobot->hasSettableAccsDecs());
  ArLog::log(ArLog::Terse, "HasLatVel %d", myRobot->hasLatVel());
  ArLog::log(ArLog::Terse, "HasMoveCommand %d", myRobot->getRobotParams()->hasMoveCommand());
  ArLog::log(ArLog::Terse, "Radius %f Width %f Length %f LengthFront %f LengthRear %f Diagonal %f", 
    myRobot->getRobotRadius(),
    myRobot->getRobotWidth(),
    myRobot->getRobotLength(),
    myRobot->getRobotLengthFront(),
    myRobot->getRobotLengthRear(),
    myRobot->getRobotDiagonal() 
  );
}


AREXPORT ArModeRobotStatus::ArModeRobotStatus(ArRobot *robot, const char *name, char key1, char key2) :
  ArMode(robot, name, key1, key2),
  myRobot(robot),
  myDebugMessageCB(this, &ArModeRobotStatus::handleDebugMessage),
  mySafetyStateCB(this, &ArModeRobotStatus::handleSafetyStatePacket),
  mySafetyWarningCB(this, &ArModeRobotStatus::handleSafetyWarningPacket),
  myBatteryShutdown(false),
  myToggleShutdownCB(this, &ArModeRobotStatus::toggleShutdown)
{
}

AREXPORT void ArModeRobotStatus::help()
{
  puts("Robot diagnostic flags mode prints the current state of the robot's error and diagnostic flags."); 
  puts("Additional debug and status information will also be requested from the robot and logged if received.");
  puts("");
  puts("volt:        Reported battery voltage");
  puts("soc:         Reported battery state of charge (if available)");
  puts("temp:        Reported temperature (-127 if unavailable)");
  puts("motors:      Are motors enabled?");
  puts("estop:       Is any e-stop button or mechanism engaged?");
  puts("stall:       Is left (L) or right (R) motor stalled?");
  puts("stallflags:  0=left motor stall, 1-7=front bumper segments hit, 8=right motor stall, 9-15=rear bumpers" );
  puts("sip/s:       Robot status packets (SIP) received per second. Normally 10.");
  puts("flags:       0 = motors enabled, 1-4 = sonar array enabled, 5 = estop, 7-8 = legacy IR, 9 = joystick button, 11 = high temperature ");
  puts("faults:      0=power error, 1=high temperature, 2=power error, 3=undervoltage, 4=gyro fault, 5=battery overtemperature, \n"
       "             6=battery balance needed, 7=encoder degradation warning, 8=encoder failure, 9=drive fault, 10=estop mismatch,\n"
       "             11=estop fail, 12=speed zone fault, 13=safety system fault, 14=backed up too fast, 15=joydrive warning"
  );
  puts("chargestate: Battery charging status");
  puts("chargepower: Whether battery charge power is being received by battery (if this robot can measure this)");
  puts("flags3:      0=joystick override mode enabled, 1=amp comm error, 2=silent estop, 3=laser error N, 4=RCL disabled, 5=rotation saturated");
  puts("");

  printf("Key commands:\n%13s:  wait for battery SoC to fall below 30%% (or if SoC is unavailable, for voltage to fall below 11.5V) and shut down robot power\n\n", "x");

  printFlagsHeader();
}

AREXPORT void ArModeRobotStatus::activate()
{
  if(baseActivate())
  {
    // only do the following on the first activate. they remain activated.
    myRobot->lock();
    myRobot->addPacketHandler(&myDebugMessageCB);
    myRobot->addPacketHandler(&mySafetyStateCB);
    myRobot->addPacketHandler(&mySafetyWarningCB);
    myRobot->unlock();
  }
  else
  {
    // error activating base mode class
    return;
  }

  addKeyHandler('x', &myToggleShutdownCB);


  if(myBatteryShutdown) puts("Will send commands to shut down robot power if state of charge is <= 30%% or battery voltage is <= 11.5/23V. Press X to cancel.");

  puts("");
  printFlagsHeader();
  printFlags();
  puts("\n");
    
    
  myRobot->lock();
  int flags = myRobot->getFlags();
  int faults = myRobot->hasFaultFlags() ? myRobot->getFaultFlags() : 0;
  int flags3 = myRobot->hasFlags3() ? myRobot->getFlags3() : 0;
  int configflags = 0;
  const ArRobotConfigPacketReader *configreader = myRobot->getOrigRobotConfig();
  if(configreader)
    configflags = configreader->getConfigFlags();
  myRobot->unlock();


  if(flags)
  {
    puts("Active Flags:");
    if(flags & ArUtil::BIT0)
      puts("\tMotors are enabled (flag 0)");
    else
      puts("\tMotors are disabled (flag 0)");
    if(flags & ArUtil::BIT5)
      puts("\tESTOP (flag 5)");
    if(flags & ArUtil::BIT9)
      puts("\tJoystick button pressed (flag 9)");
    if(flags & ArUtil::BIT11)
      puts("\tHigh temperature. (flag 11)");
  }


  if(faults)
  {
    puts("Active Fault Flags:");
    if(faults & ArUtil::BIT0)
      puts("\tPDB Laser Status Error (fault 0)");
    if(faults & ArUtil::BIT1)
      puts("\tHigh Temperature (fault 1)");
    if(faults & ArUtil::BIT2)
      puts("\tPDB Error (fault 2)");
    if(faults & ArUtil::BIT3)
      puts("\tUndervoltage/Low Battery (fault 3)");
    if(faults & ArUtil::BIT4)
      puts("\tGyro Critical Fault (fault 4)");
    if(faults & ArUtil::BIT5)
      puts("\tBattery Overtemperature (fault 5)");
    if(faults & ArUtil::BIT6)
      puts("\tBattery balance required (fault 6)");
    if(faults & ArUtil::BIT7)
      puts("\tEncoder degradation (fault 7)");
    if(faults & ArUtil::BIT8)
      puts("\tEncoder failure (fault 8)");
    if(faults & ArUtil::BIT9)
      puts("\tCritical general driving fault (fault 9)");
    if(faults & ArUtil::BIT10)
      puts("\tESTOP Mismatch Warning. One ESTOP channel may be intermittent or failing. Check connections to control panel. (ESTOP_MISMATCH_FLAG, 10)");
    if(faults & ArUtil::BIT11)
      puts("\tESTOP Safety Fault. ESTOP circuitry has failed. Motors disabled until safety system recommision or disabled. (ESTOP_SAFETY_FAULT, 11)");
    if(faults & ArUtil::BIT12)
      puts("\tLaser/speed zone failure or zone mismatch. Speed limited until safety system recommisioa or disabled.  (SPEED_ZONE_SAFETY_FAULT, 12)");
    if(faults & ArUtil::BIT13)
      puts("\tSAFETY_UNKNOWN_FAULT (fault 13)");
    if(faults & ArUtil::BIT14)
      puts("\tBacked up too fast. Reduce speed to avoid or disable safety system to allow faster reverse motion. (fault 14)");
    if(faults & ArUtil::BIT15)
      puts("\tJoydrive unsafe mode warning (fault 15)");
  }

  
  if(flags3)
  {
    puts("Active Flags3:");
    if(flags3 & ArUtil::BIT0)
      puts("\tJoystick override mode enabled (0)");
    if(flags3 & ArUtil::BIT1)
      puts("\tAmp. comm. error (1)");
    if(flags3 & ArUtil::BIT2)
      puts("\tSilent E-Stop (2)");
    if(flags3 & ArUtil::BIT3)
      puts("\tLaser safety circuit error (S300 error 'n') (3)");
    if(~flags3 & ArUtil::BIT4)
      puts("\tRotation control loop not enabled (4)");
    if(flags3 & ArUtil::BIT5)
      puts("\tRotation integrator saturated (5)");
  }
    
  
  if(configflags & ArUtil::BIT0)
  {
    puts("ConfigFlags:");
    puts("\tFirmware boot error. Robot controller bootloader detected but no firmware. (config flag 0 set)");
  }

  fflush(stdout);

  // TODO keys to enable/disable: LogMovementSent, LogMovementReceived,
  // LogVelocitiesReceived, PacketsReceivedTracking, LogSIPContents,
  // PacketsSentTracking.
  // TODO keys to start/stop WHEEL_INFO
  
  // TODO get simulator info

  myRobot->comInt(214, 1); // request state of safety systems

  // print first header line for user task refresh
  puts("");
  printFlagsHeader();
}

AREXPORT void ArModeRobotStatus::deactivate()
{
  remKeyHandler(&myToggleShutdownCB);
  // commented out to keep packet handlers active so we can use other modes but
  // still see debug messages
  //myRobot->remPacketHandler(&myDebugMessageCB);
  //myRobot->remPacketHandler(&mySafetyStateCB);
  //myRobot->remPacketHandler(&mySafetyWarningCB);
  if(!baseDeactivate()) return;
}

void ArModeRobotStatus::toggleShutdown()
{
  myBatteryShutdown = !myBatteryShutdown;

  if(myBatteryShutdown)
  {
    puts("Will shut down robot power when state of charge is <= 30%% or battery voltage is <= 11.5/23V.");
  }
  else
  {
    puts("Cancelled shutdown");
  }
}


AREXPORT void ArModeRobotStatus::userTask()
{
  printFlags();
  printf("\r");
  fflush(stdout);
  
  if(myBatteryShutdown &&
    ( (myRobot->haveStateOfCharge() && (myRobot->getStateOfCharge() <= 30)) ||
      (!myRobot->haveStateOfCharge() && (myRobot->getBatteryVoltage() <= 11.5)) )
  )
  {
    puts("stopping robot");
    myRobot->com(ArCommands::STOP);
    ArUtil::sleep(50);
    myRobot->comInt(ArCommands::ESTOP, 1);
    puts("sending (various) commands to shut down robot");

    std::map<int, ArBatteryMTX*> *batMap = myRobot->getBatteryMap();
    if(batMap)
    {
      for(std::map<int, ArBatteryMTX*>::reverse_iterator i = batMap->rbegin(); i != batMap->rend(); ++i)
      {
        puts("telling an MTX battery to shut off");
        ArBatteryMTX *b = i->second;
        if(b && b->isConnected()) b->sendEmergencyPowerOff();
      }
    }

    puts("telling mt400 to shut off");
    myRobot->com2Bytes(31, 1, 1); // mt400/patrolbot

    puts("telling seekur to recenter and power off");
    myRobot->com(119); // seekur

  }
}
  

void ArModeRobotStatus::printFlagsHeader()
{
  const char* headerfmt = 
    "%-4s "  // volts
    "%-4s "  // soc
    "%-4s "     // temp
    "%-6s "     // mot.enable?
    "%-6s "      // estop?
    "%-6s "      // stallL?
    "%-2s "      // stallR?
    "%-16s "     // stallval
    "%-5s "      // #sip/sec    
    "%-16s "     // flags
    "%-16s "     // fault flags
    "%-13s "     // chargestate
    "%-7s "     // chargerpower
    "%-32s "     // flags3
    "\n";


  printf(headerfmt,
        "volt",
        "soc",
        "temp",
        "motors",
        "estop",
        "L stall",
        "R",
        "stallval",
        "sip/s",
        "flags",
        "faults",
        "chargestate",
        "charge",
        "flags3"
    );
}

void ArModeRobotStatus::printFlags()
{
  const char* datafmt = 
    "%-03.01f "  // volts
    "%-03.02f "  // soc
    "%- 04d "     // temp
    "%-6s "     // mot.enable?
    "%-6s "      // estop?
    "%-6s "      // stallL?
    "%-3s "      // stallR?
    "%-16s "     // stallval
    "%-5d "      // #sip/sec    
    "%-16s "     // flags
    "%-16s "     // fault flags
    "%-13s "     // chargestate
    "%-7s "     // chargerpower
    "%-32s "     // flags3
  ;

  myRobot->lock();

  printf(datafmt,
    myRobot->getRealBatteryVoltage(),
    myRobot->getStateOfCharge(),
    myRobot->getTemperature(),
    myRobot->areMotorsEnabled()?"yes":"NO",
    myRobot->isEStopPressed()?"YES":"no",
    myRobot->isLeftMotorStalled()?"YES":"no",
    myRobot->isRightMotorStalled()?"YES":"no",
    int16_as_bitstring(myRobot->getStallValue()).c_str(),
    myRobot->getMotorPacCount() ,
    int16_as_bitstring(myRobot->getFlags()).c_str(),
    myRobot->hasFaultFlags() ? int16_as_bitstring(myRobot->getFaultFlags()).c_str() : "n/a",
    myRobot->getChargeStateName(),
    myRobot->isChargerPowerGood() ? "YES" : "no",
    myRobot->hasFlags3() ? int32_as_bitstring(myRobot->getFlags3()).c_str() : "n/a"
  );
  myRobot->unlock();
}

bool ArModeRobotStatus::handleDebugMessage(ArRobotPacket *pkt)
{
  if(pkt->getID() != ArCommands::MARCDEBUG) return false;
  char msg[256];
  pkt->bufToStr(msg, sizeof(msg));
  msg[255] = 0;
  ArLog::log(ArLog::Terse, "Firmware Debug Message Received: %s", msg);
  return true;
}


const char *ArModeRobotStatus::safetyStateName(int state)
{
  switch (state) {
    case 0:
      return "unknown/initial";
    case 0x10:
      return "failure";
    case 0x20: 
      return "warning";
    case 0x40:
      return "commissioned";
    case 0x50:
      return "decommissioned/disabled";
  }
  return "invalid/unknown";
}

bool ArModeRobotStatus::handleSafetyStatePacket(ArRobotPacket *p)
{
  if(p->getID() != 214) return false;
  int state = p->bufToUByte();
  int estop_state = p->bufToUByte();
  int laser_state = p->bufToUByte();
  ArLog::log(ArLog::Normal, "Safety system state: 0x%x, system0(estop)=0x%x, %s, system1(laser)=0x%x, %s\n", state, estop_state, safetyStateName(estop_state), laser_state, safetyStateName(laser_state));
  if(state == 0x10) ArLog::log(ArLog::Terse, "Warning: Safety system enabled with failure detected, robot controller will not allow motion.");
  if(state == 0x20) ArLog::log(ArLog::Terse, "Warning: Safety system enabled with warning indicated, robot controller will limit robot motion.");
  if(estop_state == 0x10) ArLog::log(ArLog::Terse, "Warning: Safety estop subsystem enabled with failure detected, robot controller will not allow motion.");
  if(estop_state == 0x20) ArLog::log(ArLog::Terse, "Warning: Safety estop subsystem enabled with warning indicated, robot controller will limit robot motion.");
  if(laser_state == 0x10) ArLog::log(ArLog::Terse, "Warning: Safety laser subsystem enabled with failure detected, robot controller will not allow motion."); 
  if(laser_state == 0x20) ArLog::log(ArLog::Terse, "Warning: Safety laser subsystem enabled with warning indicated, robot controller will limit robot motion.");
  return true;
}

bool ArModeRobotStatus::handleSafetyWarningPacket(ArRobotPacket *p)
{
  if(p->getID() != 217) return false;
  ArLog::log(ArLog::Terse, "Safety system warning received!");
  return false; // let other stuff also handle it
}
