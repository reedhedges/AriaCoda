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
#include "ArP2Arm.h"
#include "ariaUtil.h"
#include "ArLog.h"
#include "ariaInternal.h"

int ArP2Arm::NumJoints=6;
const unsigned int ArP2Arm::ARMpac=0xa0;
const unsigned int ArP2Arm::ARMINFOpac=0xa1;
const unsigned char ArP2Arm::ComArmInfo=70;
const unsigned char ArP2Arm::ComArmStats=71;
const unsigned char ArP2Arm::ComArmInit=72;
const unsigned char ArP2Arm::ComArmCheckArm=73;
const unsigned char ArP2Arm::ComArmPower=74;
const unsigned char ArP2Arm::ComArmHome=75;
const unsigned char ArP2Arm::ComArmPark=76;
const unsigned char ArP2Arm::ComArmPos=77;
const unsigned char ArP2Arm::ComArmSpeed=78;
const unsigned char ArP2Arm::ComArmStop=79;
const unsigned char ArP2Arm::ComArmAutoPark=80;
const unsigned char ArP2Arm::ComArmGripperPark=81;
const int ArP2Arm::ArmJoint1=0x1;
const int ArP2Arm::ArmJoint2=0x2;
const int ArP2Arm::ArmJoint3=0x4;
const int ArP2Arm::ArmJoint4=0x8;
const int ArP2Arm::ArmJoint5=0x10;
const int ArP2Arm::ArmJoint6=0x20;
const int ArP2Arm::ArmGood=0x100;
const int ArP2Arm::ArmInited=0x200;
const int ArP2Arm::ArmPower=0x400;
const int ArP2Arm::ArmHoming=0x800;


AREXPORT ArP2Arm::ArP2Arm() :
  myInited(false),
  myRobot(0),
  //  myModel(),
  myLastStatusTime(),
  myLastInfoTime(),
  myVersion(),
  myStatusRequest(ArP2Arm::StatusOff),
  myLastStatus(0),
  myStatus(0),
  myCon(),
  myAriaUninitCB(this, &ArP2Arm::uninit),
  myArmPacketHandler(this, &ArP2Arm::armPacketHandler),
  myPacketCB(0),
  myStoppedCB(0)
{
  myArmPacketHandler.setName("ArP2Arm");
}

AREXPORT ArP2Arm::~ArP2Arm()
{
  //uninit();
  if (myRobot != NULL)
    myRobot->remPacketHandler(&myArmPacketHandler);
}

/**
   Initialize the P2 Arm class. This must be called before anything else. The
   setRobot() must be called to let ArP2Arm know what instance of an ArRobot
   to use. It talks to the robot and makes sure that there is an arm on it
   and it is in a good condition. The AROS/P2OS arm servers take care of AUX
   port serial communications with the P2 Arm controller.
*/
AREXPORT ArP2Arm::State ArP2Arm::init()
{
  ArLog::log(ArLog::Normal, "Initializing the arm.");

  ArTime now;

  if (myInited)
    return(ALREADY_INITED);

  if (!myRobot || !myRobot->isRunning() || !myRobot->isConnected())
    return(ROBOT_NOT_SETUP);

  Aria::addUninitCallBack(&myAriaUninitCB, ArListPos::FIRST);
  ArLog::log(ArLog::Verbose, "Adding the P2 Arm packet handler.");
  myRobot->addPacketHandler(&myArmPacketHandler, ArListPos::FIRST);
  now.setToNow();
  if (!comArmStats(StatusSingle))
    return(COMM_FAILED);
  ArUtil::sleep(100);
  if (!comArmInfo())
    return(COMM_FAILED);
  ArUtil::sleep(300);

  if (!now.isAfter(myLastStatusTime) || !now.isAfter(myLastInfoTime))
    return(COMM_FAILED);

  if (!(myStatus & ArmGood))
    return(NO_ARM_FOUND);

  myInited=true;

  return(SUCCESS);
}

/**
   Uninitialize the arm class. This simply asks the arm to park itself
   and cleans up its internal state. To completely uninitialize the P2 Arm
   itself have the ArRobot disconnect from P2OS.
*/
AREXPORT ArP2Arm::State ArP2Arm::uninit()
{
  bool ret;

  if (!myInited)
    return(NOT_INITED);

  ret=comArmPark();

  myInited=false;
  myVersion="";
  myStatusRequest=ArP2Arm::StatusOff;
  myLastStatus=0;
  myStatus=0;
  if (ret)
    return(SUCCESS);
  else
    return(COMM_FAILED);
}

/**
   Powers on the arm. The arm will shake for up to 2 seconds after powering
   on. If the arm is told to move before it stops shaking, that vibration
   can be amplified by moving. The default is to wait the 2 seconds for the
   arm to settle down.
   @param doSleep if true, sleeps 2 seconds to wait for the arm to stop shaking
*/
AREXPORT ArP2Arm::State ArP2Arm::powerOn(bool doSleep)
{
  if (isGood())
  {
    ArLog::log(ArLog::Normal, "ArP2Arm::powerOn: Powering arm.");
    if (!comArmPower(true))
      return(COMM_FAILED);
    // Sleep for 2 seconds to wait for the arm to stop shaking from the
    // effort of turning on
    if (doSleep)
      ArUtil::sleep(2000);
    return(SUCCESS);
  }
  else
    return(NOT_CONNECTED);
}

/**
   Powers off the arm. This should only be called when the arm is in a good
   position to power off. Due to the design, it will go limp when the power
   is turned off. A more safe way to power off the arm is to use the
   ArP2Arm::park() function. Which will home the arm, then power if off.
   @see park
*/
AREXPORT ArP2Arm::State ArP2Arm::powerOff()
{
  if (isGood())
  {
    ArLog::log(ArLog::Normal, "ArP2Arm::powerOff: Powering off arm.");
    if (comArmPower(false))
      return(SUCCESS);
    else
      return(COMM_FAILED);
  }
  else
    return(NOT_CONNECTED);
}

/**
   Requests the arm info packet from P2OS and immediately returns. This packet
   will be sent during the next 100ms cycle of P2OS. Since there is a very
   noticable time delay, the user should use the ArP2Arm::setPacketCB() to set
   a callback so the user knows when the packet has been received.
   @see setPacketCB
*/
AREXPORT ArP2Arm::State ArP2Arm::requestInfo()
{
  if (isGood())
  {
    if (comArmInfo())
      return(SUCCESS);
    else
      return(COMM_FAILED);
  }
  else
    return(NOT_CONNECTED);
}

/**
   Requests the arm status packet from P2OS and immediately returns. This
   packet will be sent during the next 100ms cycle of P2OS. Since there is a
   very noticable time delay, the user should use the ArP2Arm::setPacketCB() to
   set a callback so the user knows when the packet has been received.
   @see setPacketCB
*/
AREXPORT ArP2Arm::State ArP2Arm::requestStatus(StatusType status)
{
  if (isGood())
  {
    if (comArmStats(status))
      return(SUCCESS);
    else
      return(COMM_FAILED);
  }
  else
    return(NOT_CONNECTED);
}

/**
   Requests that P2OS initialize the arm and immediately returns. The arm
   initialization procedure takes about 700ms to complete and a little more
   time for the status information to be relayed back to the client. Since
   there is a very noticable time delay, the user should use the
   ArP2Arm::setPacketCB() to set a callback so the user knows when the arm info
   packet has been received. Then wait about 800ms, and send a 
   ArP2Arm::requestStatus() to get the results of the init request. While the
   init is proceding, P2OS will ignore all arm related commands except
   requests for arm status and arm info packets.

   ArP2Arm::checkArm() can be used to periodicly check to make sure that the
   arm controller is still alive and responding.

   @see checkArm
   @see setPacketCB
*/
AREXPORT ArP2Arm::State ArP2Arm::requestInit()
{
  if (isGood())
  {
    if (comArmInit())
      return(SUCCESS);
    else
      return(COMM_FAILED);
  }
  else
    return(NOT_CONNECTED);
}

/**
   Requests that P2OS checks to see if the arm is still alive and immediately
   exits. This is not a full init and differs that P2OS will still accept
   arm commands and the arm will not be parked. If P2OS fails to find the arm
   it will change the status byte accordingly and stop accepting arm related
   commands except for init commands. If the parameter waitForResponse is true
   then checkArm() will wait the appropriate amoutn of time and check the
   status of the arm. If you wish to do the waiting else where the arm check
   sequence takes about 200ms, so the user should wait 300ms then send a
   ArP2Arm::requestStatus() to get the results of the check arm request. Since
   there is a very noticable time delay, the user should use the 
   ArP2Arm::setPacketCB() to set a callback so the user knows when the packet
   has been received.

   This can be usefull for telling if the arm is still alive. The arm
   controller can be powered on/off separately from the robot. 

   @param waitForResponse cause the function to block until their is a response
   @see requestInit
   @see setPacketCB
*/
AREXPORT ArP2Arm::State ArP2Arm::checkArm(bool waitForResponse)
{
  ArTime now;

  if (isGood())
  {
    now.setToNow();
    if (!comArmInfo())
      return(COMM_FAILED);
    if (waitForResponse)
    {
      ArUtil::sleep(300);
      if (!myLastInfoTime.isAfter(now))
	return(COMM_FAILED);
      if (isGood())
	return(SUCCESS);
      else
	return(NO_ARM_FOUND);
    }
    else
      return(SUCCESS);
  }
  else
    return(NOT_CONNECTED);
}

/**
   Tells the arm to go to the home position. While the arm is homing, the
   status byte will reflect it with the ArP2Arm::ArmHoming flag. If joint is set
   to -1, then all the joints will be homed at a safe speed. If a single joint
   is specified, that joint will be told to go to its home position at the
   current speed its set at.

   @param joint home only that joint
*/
AREXPORT ArP2Arm::State ArP2Arm::home(int joint)
{
  if (!isGood())
    return(NOT_INITED);

  if ((joint < 0) && !comArmHome(0xff))
    return(COMM_FAILED);
  else if ((joint > 0) && (joint <= NumJoints) && !comArmHome(joint))
    return(COMM_FAILED);
  else
    return(INVALID_JOINT);

  return(SUCCESS);
}

/**
   Move the joint to the position at the given speed. If vel is 0, then the
   currently set speed will be used. The position is in degrees. Each joint
   has about a +-90 degree range, but they all differ due to the design.

   See ArP2Arm::moveToTicks() for a description of how positions are defined.
   See ArP2Arm::moveVel() for a description of how speeds are defined.

   @param joint the joint to move
   @param pos the position in degrees to move to
   @param vel the speed at which to move. 0 will use the currently set speed
   @see moveToTicks
   @see moveVel
*/

AREXPORT ArP2Arm::State ArP2Arm::moveTo(int joint, float pos,  unsigned char vel)
{
  unsigned char ticks;

  if (!isGood())
    return(NOT_INITED);
  else if ((joint <= 0) || (joint > NumJoints))
    return(INVALID_JOINT);

  //  if ((vel < 0) && !comArmSpeed(joint, 0-vel))
  //  return(COMM_FAILED);
  else if ((vel > 0) && !comArmSpeed(joint, vel))
    return(COMM_FAILED);

  if (!convertDegToTicks(joint, pos, &ticks))
    return(INVALID_POSITION);

  return(moveToTicks(joint, ticks));
}

/**
   Move the joint to the given position in ticks. A tick is the arbitrary
   position value that the arm controller uses. The arm controller uses a
   single unsigned byte to represent all the possible positions in the range
   of the servo for each joint. So the range of ticks is 0-255 which is 
   mapped to the physical range of the servo. Due to the design of the arm,
   certain joints range are limited by the arm itself. P2OS will bound the
   position to physical range of each joint. This is a lower level of
   controlling the arm position than using ArP2Arm::moveTo(). ArP2Arm::moveTo()
   uses a conversion factor which converts degrees to ticks.

   @param joint the joint to move
   @param pos the position, in ticks, to move to
   @see moveTo
*/
AREXPORT ArP2Arm::State ArP2Arm::moveToTicks(int joint, unsigned char pos)
{
  if (!isGood())
    return(NOT_INITED);
  else if ((joint <= 0) || (joint > NumJoints))
    return(INVALID_JOINT);

  if (!comArmPos(joint, pos))
    return(COMM_FAILED);

  return(SUCCESS);
}

/**
   Step the joint pos degrees from its current position at the given speed.
   If vel is 0, then the currently set speed will be used.

   See ArP2Arm::moveToTicks() for a description of how positions are defined.
   See ArP2Arm::moveVel() for a description of how speeds are defined.

   @param joint the joint to move
   @param pos the position in degrees to step
   @param vel the speed at which to move. 0 will use the currently set speed
   @see moveTo
   @see moveVel
*/
AREXPORT ArP2Arm::State ArP2Arm::moveStep(int joint, float pos, unsigned char vel)
{
  unsigned char ticks;

  if (!isGood())
    return(NOT_INITED);
  else if ((joint <= 0) || (joint > NumJoints))
    return(INVALID_JOINT);

  //  if ((vel < 0) && !comArmSpeed(joint, 0-vel))
  //  return(COMM_FAILED);
  else if ((vel > 0) && !comArmSpeed(joint, vel))
    return(COMM_FAILED);

  if (!convertDegToTicks(joint, pos, &ticks))
    return(INVALID_POSITION);

  return(moveStepTicks(joint, ticks));
}

/**
   Move the joint pos ticks from its current position. A tick is the arbitrary
   position value that the arm controller uses. The arm controller uses a
   single unsigned byte to represent all the possible positions in the range
   of the servo for each joint. So the range of ticks is 0-255 which is 
   mapped to the physical range of the servo. Due to the design of the arm,
   certain joints range are limited by the arm itself. P2OS will bound the
   position to physical range of each joint. This is a lower level of
   controlling the arm position than using ArP2Arm::moveTo(). ArP2Arm::moveStep()
   uses a conversion factor which converts degrees to ticks.

   @param joint the joint to move
   @param pos the position, in ticks, to move to
   @see moveStep
*/
AREXPORT ArP2Arm::State ArP2Arm::moveStepTicks(int joint, signed char pos)
{
  if (!isGood())
    return(NOT_INITED);
  else if ((joint <= 0) || (joint > NumJoints))
    return(INVALID_JOINT);

  if (!comArmPos(joint, getJoint(joint)->myPos + pos))
    return(COMM_FAILED);

  return(SUCCESS);
}

/**
   Set the joints velocity. The arm controller has no way of controlling the
   speed of the servos in the arm. So to control the speed of the arm, P2OS
   will incrementaly send a string of position commands to the arm controller
   to get the joint to move to its destination. To vary the speed, the amount
   of time to wait between each point in the path is varied. The velocity
   parameter is simply the number of milliseconds to wait between each point
   in the path. 0 is the fastest and 255 is the slowest. A reasonable range
   is around 10-40. 
   @param joint the joint to move
   @param vel the velocity to move at
*/
AREXPORT ArP2Arm::State ArP2Arm::moveVel(int joint, int vel)
{
  if (!isGood())
    return(NOT_INITED);
  else if ((joint <= 0) || (joint > NumJoints))
    return(INVALID_JOINT);

  if ((vel < 0) && (!comArmSpeed(joint, 0-vel) || !comArmPos(joint, 0)))
    return(COMM_FAILED);
  else if ((vel > 0) && (!comArmSpeed(joint, vel) || !comArmPos(joint, 255)))
    return(COMM_FAILED);

  return(SUCCESS);
}

/**
   Stop the arm from moving. This overrides all other actions except for the
   arms initilization sequence.
*/
AREXPORT ArP2Arm::State ArP2Arm::stop()
{
  if (!isGood())
    return(NOT_INITED);

  if (!comArmStop())
    return(COMM_FAILED);

  return(SUCCESS);
}

AREXPORT float ArP2Arm::getJointPos(int joint)
{
  float val;

  if (isGood() && (joint > 0) && (joint <= NumJoints) &&
      convertTicksToDeg(joint, getJoint(joint)->myPos, &val))
    return(val);
  else
    return(0.0);
}

AREXPORT unsigned char ArP2Arm::getJointPosTicks(int joint)
{
  if (isGood() && (joint > 0) && (joint <= NumJoints))
    return(getJoint(joint)->myPos);
  else
    return(0);
}

AREXPORT P2ArmJoint * ArP2Arm::getJoint(int joint)
{
  if ((joint > 0) && (joint <= NumJoints))
    return(&myJoints[joint-1]);
  else
    return(0);
}

bool ArP2Arm::armPacketHandler(ArRobotPacket *packet)
{
  bool doWake;
  int i;

  if (packet->getID() == ARMpac)
  {
    myLastStatusTime.setToNow();
    myLastStatus=myStatus;
    myStatus=packet->bufToUByte2();
    for (i=1; i<=NumJoints; ++i)
      getJoint(i)->myPos=packet->bufToUByte();

    // Wake up all threads waiting for the arm to stop moving
    for (doWake=false, i=0; i<8; ++i)
    {
      if (((myLastStatus & (1 << i)) != (myStatus & (1 << i))) &&
	  (myStatus & (1 << i)))
	doWake=true;
    }
    if (doWake && myStoppedCB)
      myStoppedCB->invoke();
    if (myPacketCB)
      myPacketCB->invoke(StatusPacket);
    return(true);
  }
  else if (packet->getID() == ARMINFOpac)
  {
    char version[512];
    myLastInfoTime.setToNow();
    packet->bufToStr(version, 512);
    myVersion=version;
    NumJoints=packet->bufToUByte();
    for (i=1; i<=NumJoints; ++i)
    {
      getJoint(i)->myVel=packet->bufToUByte();
      getJoint(i)->myHome=packet->bufToUByte();
      getJoint(i)->myMin=packet->bufToUByte();
      getJoint(i)->myCenter=packet->bufToUByte();
      getJoint(i)->myMax=packet->bufToUByte();
      getJoint(i)->myTicksPer90=packet->bufToUByte();
    }
    if (myPacketCB)
      myPacketCB->invoke(InfoPacket);
    return(true);
    }
  else
    return(false);
}

bool ArP2Arm::comArmInfo()
{
  return(myRobot->com(ComArmInfo));
}

bool ArP2Arm::comArmStats(StatusType stats)
{
  return(myRobot->comInt(ComArmStats, (int)stats));
}

bool ArP2Arm::comArmInit()
{
  return(myRobot->com(ComArmInit));
}

bool ArP2Arm::comArmCheckArm()
{
  return(myRobot->com(ComArmCheckArm));
}

bool ArP2Arm::comArmPower(bool on)
{
  if (on)
    return(myRobot->comInt(ComArmPower, 1));
  else
    return(myRobot->comInt(ComArmPower, 0));
}

bool ArP2Arm::comArmHome(unsigned char joint)
{
  return(myRobot->comInt(ComArmHome, joint));
}

bool ArP2Arm::comArmPos(unsigned char joint, unsigned char pos)
{
  return(myRobot->com2Bytes(ComArmPos, joint, pos));
}

bool ArP2Arm::comArmSpeed(unsigned char joint, unsigned char speed)
{
  return(myRobot->com2Bytes(ComArmSpeed, joint, speed));
}

bool ArP2Arm::comArmStop(unsigned char joint)
{
  return(myRobot->comInt(ComArmStop, joint));
}

bool ArP2Arm::comArmPark()
{
  return(myRobot->com(ComArmPark));
}

bool ArP2Arm::comArmAutoPark(int waitSecs)
{
  return(myRobot->comInt(ComArmAutoPark, waitSecs));
}

bool ArP2Arm::comArmGripperPark(int waitSecs)
{
  return(myRobot->comInt(ComArmGripperPark, waitSecs));
}

AREXPORT bool ArP2Arm::getMoving(int joint)
{
  if ((joint < 0) && (myStatus & 0xf))
    return(true);
  else if (myStatus & (1 << joint))
    return(true);
  else
    return(false);
}

AREXPORT bool ArP2Arm::isPowered()
{
  if (myStatus & ArmPower)
    return(true);
  else
    return(false);
}

AREXPORT bool ArP2Arm::isGood()
{
  if (myRobot && myRobot->isRunning() && myRobot->isConnected() &&
      myInited && (myStatus & ArmGood) && (myStatus & ArmInited))
    return(true);
  else
    return(false);
}

AREXPORT ArP2Arm::State ArP2Arm::park()
{
  if (!isGood())
    return(NOT_INITED);

  if (comArmPark())
    return(SUCCESS);
  else
    return(COMM_FAILED);
}

/**
   P2OS will automaticly park the arm if it gets no arm related packets after
   waitSecs. This is to help protect the arm when the program looses
   connection with P2OS. Set the value to 0 to disable this timer. Default
   wait is 10 minutes.

   @param waitSecs seconds to wait till parking the arm when idle
*/
AREXPORT ArP2Arm::State ArP2Arm::setAutoParkTimer(int waitSecs)
{
  if (!isGood())
    return(NOT_INITED);

  if (comArmAutoPark(waitSecs))
    return(SUCCESS);
  else
    return(COMM_FAILED);
}

/**
   P2OS/AROS automatically park the gripper after its been closed for more than
   waitSecs. The gripper servo can overheat and burnout if it is holding
   something for more than 10 minutes. Care must be taken to ensure that this
   does not happen. If you wish to manage the gripper yourself, you can
   disable this timer by setting it to 0.

   @param waitSecs seconds to wait till parking the gripper once it has begun to grip something
*/
AREXPORT ArP2Arm::State ArP2Arm::setGripperParkTimer(int waitSecs)
{
  if (!isGood())
    return(NOT_INITED);

  if (comArmGripperPark(waitSecs))
    return(SUCCESS);
  else
    return(COMM_FAILED);
}

AREXPORT bool ArP2Arm::convertDegToTicks(int joint, float pos,
				       unsigned char *ticks)
{
  long val;

  if ((joint <= 0) || (joint > NumJoints))
    return(false);

  if (joint == 6)
    *ticks=(unsigned char)pos;
  else
  {
    val=ArMath::roundInt(getJoint(joint)->myTicksPer90*(pos/90.0));
    if ((joint >= 1) && (joint <= 3))
      val=-val;
    val+=getJoint(joint)->myCenter;
    if (val < getJoint(joint)->myMin)
      *ticks=getJoint(joint)->myMin;
    else if (val > getJoint(joint)->myMax)
      *ticks=getJoint(joint)->myMax;
    else
      *ticks=val;
  }

  return(true);
}

AREXPORT bool ArP2Arm::convertTicksToDeg(int joint, unsigned char pos,
				       float *degrees)
{
  long val;

  if ((joint <= 0) || (joint > NumJoints))
    return(false);

  if (joint == 6)
    *degrees=pos;
  else
  {
    val=ArMath::roundInt(90.0/getJoint(joint)->myTicksPer90*
			 (pos-getJoint(joint)->myCenter));
    if ((joint >= 1) && (joint <= 3))
      val=-val;
    *degrees=val;
  }

  return(true);
}

AREXPORT P2ArmJoint::P2ArmJoint() :
  myPos(0),
  myVel(0),
  myHome(0),
  myMin(0),
  myCenter(0),
  myMax(0),
  myTicksPer90(0)
{
}

AREXPORT P2ArmJoint::~P2ArmJoint()
{
}

