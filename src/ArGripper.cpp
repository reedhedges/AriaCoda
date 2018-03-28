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
#include "ArGripper.h"
#include "ArCommands.h"

/**
   @param robot The robot this gripper is attached to
   @param gripperType How to communicate with the gripper: see ArGripper::Type. 
     The default, QUERYTYPE, will work with most robot configurations with 
     a recent firmware version.
*/
AREXPORT ArGripper::ArGripper(ArRobot *robot, int gripperType) :
  myConnectCB(this, &ArGripper::connectHandler),
  myPacketHandlerCB(this, &ArGripper::packetHandler)
{
  myRobot = robot;
  myType = gripperType;
  if (myRobot != NULL) 
  {
    myRobot->addPacketHandler(&myPacketHandlerCB, ArListPos::FIRST);
    myRobot->addConnectCB(&myConnectCB, ArListPos::LAST);
    if (myRobot->isConnected() && (myType == GRIPPAC || myType == QUERYTYPE))
      myRobot->comInt(ArCommands::GRIPPERPACREQUEST, 2);
  }
  myLastDataTime.setToNow();
}

AREXPORT ArGripper::~ArGripper()
{
}

AREXPORT void ArGripper::connectHandler(void)
{
  if (myRobot != NULL && (myType == GRIPPAC || myType == QUERYTYPE))
    myRobot->comInt(ArCommands::GRIPPERPACREQUEST, 2);
}

/**
   @return whether the command was sent to the robot or not 
*/
AREXPORT bool ArGripper::gripOpen(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(ArCommands::GRIPPER, 
			   ArGripperCommands::GRIP_OPEN);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
AREXPORT bool ArGripper::gripClose(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(ArCommands::GRIPPER, 
			   ArGripperCommands::GRIP_CLOSE);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
AREXPORT bool ArGripper::gripStop(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(ArCommands::GRIPPER, 
			   ArGripperCommands::GRIP_STOP);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
AREXPORT bool ArGripper::liftUp(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(ArCommands::GRIPPER, 
			   ArGripperCommands::LIFT_UP);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
AREXPORT bool ArGripper::liftDown(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(ArCommands::GRIPPER, 
			   ArGripperCommands::LIFT_DOWN);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
AREXPORT bool ArGripper::liftStop(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(ArCommands::GRIPPER, 
			   ArGripperCommands::LIFT_STOP);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
AREXPORT bool ArGripper::gripperStore(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(ArCommands::GRIPPER, 
			   ArGripperCommands::GRIPPER_STORE);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
AREXPORT bool ArGripper::gripperDeploy(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(ArCommands::GRIPPER, 
			   ArGripperCommands::GRIPPER_DEPLOY);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
AREXPORT bool ArGripper::gripperHalt(void)
{
  if (myRobot != NULL)
    return myRobot->comInt(ArCommands::GRIPPER, 
			   ArGripperCommands::GRIPPER_HALT);
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
AREXPORT bool ArGripper::gripPressure(int mSecIntervals)
{
  if (myRobot == NULL)
    return false;
  
  if (myRobot->comInt(ArCommands::GRIPPER, ArGripperCommands::GRIP_PRESSURE) &&
      myRobot->comInt(ArCommands::GRIPPERVAL, mSecIntervals))
    return true;
  else
    return false;
}

/**
   @return whether the command was sent to the robot or not 
*/
AREXPORT bool ArGripper::liftCarry(int mSecIntervals)
{
  if (myRobot == NULL)
    return false;
  
  if (myRobot->comInt(ArCommands::GRIPPER, ArGripperCommands::LIFT_CARRY) &&
      myRobot->comInt(ArCommands::GRIPPERVAL, mSecIntervals))
    return true;
  else
    return false;
}

/**
   @return true if the gripper paddles are moving
*/
AREXPORT bool ArGripper::isGripMoving(void) const
{
  int d;

  if (myType == NOGRIPPER || myType == QUERYTYPE || myRobot == NULL)
  {
    return false;
  }
  else if (myType == GENIO || myType == GRIPPAC || myType == USERIO)
  {
    if (myType == GENIO || myType == USERIO)
      d = myRobot->getAnalogPortSelected() >> 8;
    else
      d = mySwitches;

    if (myType == USERIO && (d & ArUtil::BIT2)) // moving
      return true;
    else if (myType != USERIO && (d & ArUtil::BIT7)) // moving
      return true;
    else // not moving
      return false;
  }
  else
  {
    ArLog::log(ArLog::Terse, "ArGripper::isGripMoving: Gripper type unknown.");
    return false;
  }
}

/**
   @return true if the lift is moving
*/
AREXPORT bool ArGripper::isLiftMoving(void) const
{
  int d;

  if (myType == NOGRIPPER || myType == QUERYTYPE || myRobot == NULL)
  {
    return false;
  }
  else if (myType == GENIO || myType == GRIPPAC || myType == USERIO)
  {
    if (myType == GENIO || myType == USERIO)
      d = myRobot->getAnalogPortSelected() >> 8;
    else
      d = mySwitches;

    if (d & ArUtil::BIT6) // moving
      return true;
    else // not moving
      return false;
  }
  else
  {
    ArLog::log(ArLog::Terse, "ArGripper::isLiftMoving: Gripper type unknown.");
    return false;
  }
    
}

/**
   @return 0 if no gripper paddles are triggered, 1 if the left paddle
   is triggered, 2 if the right paddle is triggered, 3 if both are
   triggered
**/
AREXPORT int ArGripper::getPaddleState(void) const
{
  int d;
  int ret = 0;

  if (myType == NOGRIPPER || myType == QUERYTYPE || myRobot == NULL)
  {
    return 0;
  }
  else if (myType == GENIO || myType == GRIPPAC || myType == USERIO)
  {
    if (myType == GENIO)
      d = myRobot->getAnalogPortSelected() >> 8;
    else if (myType == USERIO)
      d = myRobot->getDigIn();
    else
      d = mySwitches;

    if (!(d & ArUtil::BIT4))
      ret += 1;
    if (!(d & ArUtil::BIT5))
      ret += 2;
    return ret;
  }
  else
  {
    ArLog::log(ArLog::Terse, "ArGripper::getPaddleState: Gripper type unknown.");
    return 0;
  }
}
/**
   @return 0 if gripper paddles between open and closed, 1 if gripper paddles 
   are open, 2 if gripper paddles are closed
*/
AREXPORT int ArGripper::getGripState(void) const
{
  int d;

  if (myType == NOGRIPPER || myType == QUERYTYPE || myRobot == NULL)
  {
    return 0;
  }
  else if (myType == GENIO || myType == GRIPPAC || myType == USERIO)
  {
    if (myType == GENIO)
      d = myRobot->getAnalogPortSelected() >> 8;
    else if (myType == USERIO)
      d = myRobot->getDigIn();
    else
      d = mySwitches;

    if (!(d & ArUtil::BIT4) && !(d & ArUtil::BIT5)) // both
      return 2;
    else if (!(d & ArUtil::BIT0)) // inner
      return 1;
    else // between
      return 0;
  }
  else
  {
    ArLog::log(ArLog::Terse, "ArGripper::getGripState: Gripper type unknown.");
    return 0;
  }

}

/**
   @return 0 if no breakbeams broken, 1 if inner breakbeam broken, 2 if 
   outter breakbeam broken, 3 if both breakbeams broken
*/
AREXPORT int ArGripper::getBreakBeamState(void) const
{
  int d;

  if (myType == NOGRIPPER || myType == QUERYTYPE || myRobot == NULL)
  {
    return 0;
  }
  else if (myType == GENIO || myType == GRIPPAC || myType == USERIO)
  {
    if (myType == GENIO)
      d = myRobot->getAnalogPortSelected() >> 8;
    else if (myType == USERIO)
      d = myRobot->getDigIn();
    else
      d = mySwitches;

    if ((d & ArUtil::BIT2) && (d & ArUtil::BIT3)) // both
      return 3;
    else if (d & ArUtil::BIT3) // inner
      return 1;
    else if (d & ArUtil::BIT2) // outter
      return 2;
    else // neither
      return 0;
  }
  else
  {
    ArLog::log(ArLog::Terse, 
	       "ArGripper::getBreakBeamState: Gripper type unknown.");
    return 0;
  }
}

/**
   @return false if lift is between up and down, true is either all the 
   way up or down
*/
AREXPORT bool ArGripper::isLiftMaxed(void) const
{
  int d = 0;

  if (myType == NOGRIPPER || myType == QUERYTYPE || myRobot == NULL)
  {
    return false;
  }
  else if (myType == GENIO || myType == GRIPPAC || myType == USERIO)
  {
    if (myType == GENIO)
      d = myRobot->getAnalogPortSelected() >> 8;
    else if (myType == USERIO)
      d = myRobot->getDigIn();
    else
      d = mySwitches;
    if (!(d & ArUtil::BIT1))
      return true;
    else
      return false;
  }
  else
  {
    ArLog::log(ArLog::Terse, "ArGripper::getLiftState: Gripper type unknown.");
    return false;
  }
}
  

AREXPORT void ArGripper::logState(void) const
{
  char paddleBuf[128];
  char liftBuf[128];
  char breakBeamBuf[128];
  char buf[1024];
  int state;

  if (myType == NOGRIPPER)
  {
    ArLog::log(ArLog::Terse, "There is no gripper.");
    return;
  } 
  if (myType == QUERYTYPE)
  {
    ArLog::log(ArLog::Terse, "Querying gripper type.");
    return;
  }
  
  if (isLiftMaxed())
    sprintf(liftBuf, "maxed");
  else
    sprintf(liftBuf, "between");

  if (isLiftMoving())
    strcat(liftBuf, "_moving");

  state = getGripState();
  if (state == 1)
    sprintf(paddleBuf, "open");
  else if (state == 2)
    sprintf(paddleBuf, "closed");
  else
    sprintf(paddleBuf, "between");

  if (isGripMoving())
    strcat(paddleBuf, "_moving");

  state = getBreakBeamState();
  if (state == 0)
    sprintf(breakBeamBuf, "none");
  else if (state == 1)
    sprintf(breakBeamBuf, "inner");
  else if (state == 2)
    sprintf(breakBeamBuf, "outter");
  else if (state == 3)
    sprintf(breakBeamBuf, "both");
  
  sprintf(buf, "Lift: %15s  Grip: %15s  BreakBeam: %10s", liftBuf, paddleBuf,
	  breakBeamBuf);
  if (myType == GRIPPAC)
    sprintf(buf, "%s TimeSince: %ld", buf, getMSecSinceLastPacket());
  ArLog::log(ArLog::Terse, buf);
  
}

AREXPORT bool ArGripper::packetHandler(ArRobotPacket *packet)
{
  int type;
  
  if (packet->getID() != 0xE0)
    return false;

  myLastDataTime.setToNow();
  type = packet->bufToUByte();  
  mySwitches = packet->bufToUByte();
  myGraspTime = packet->bufToUByte();

  if (myType == QUERYTYPE)
  {
    if (type == 2)
    {
      ArLog::log(ArLog::Normal, 
		 "Gripper:  querried, using General IO.");
      myType = GENIO;
    }
    else if (type == 1)
    {
      ArLog::log(ArLog::Normal, 
		 "Gripper:  querried, using User IO.");
      myType = USERIO;
    }
    else
    {
      ArLog::log(ArLog::Normal, 
		 "Gripper:  querried, the robot has no gripper.");
      myType = NOGRIPPER;
    }
    if (myRobot != NULL)
      myRobot->comInt(ArCommands::GRIPPERPACREQUEST, 0);
    return true;
  }
  if (myRobot != NULL && myType != GRIPPAC)
  {
    ArLog::log(ArLog::Verbose, 
	       "Gripper: got another gripper packet after stop requested.");
    myRobot->comInt(ArCommands::GRIPPERPACREQUEST, 0);
  }
  return true;
}

/**
   @return the gripper type
   @see Type
*/
AREXPORT int ArGripper::getType(void) const
{
  return myType;
}

/**
   @param type the type of gripper to set it to
*/
AREXPORT void ArGripper::setType(int type)
{
  myType = type;
  if (myRobot != NULL && (myType == GRIPPAC || myType == QUERYTYPE))
    myRobot->comInt(ArCommands::GRIPPERPACREQUEST, 2);
}

/**
   @return the number of milliseconds since the last packet
*/
AREXPORT long ArGripper::getMSecSinceLastPacket(void) const
{
  return myLastDataTime.mSecSince();
}

/**
   If you are using this as anything other than GRIPPAC and you want to
   find out the grasp time again, just do a setType with QUERYTYPE and 
   it will query the robot again and get the grasp time from the robot.
   @return the number of 20 MSec intervals the gripper will continue grasping 
   for after both paddles are triggered
*/
AREXPORT int ArGripper::getGraspTime(void) const
{
  return myGraspTime;
}
