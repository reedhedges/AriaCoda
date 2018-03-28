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
#include "ariaOSDef.h"
#include "ArExport.h"
#include "ArCommands.h"
#include "ArRobot.h"
#include "ArTCMCompassRobot.h"

AREXPORT ArTCMCompassRobot::ArTCMCompassRobot(ArRobot *robot) :
  myPacketHandlerCB(this, &ArTCMCompassRobot::packetHandler)
{
  myRobot = robot;
  myPacketHandlerCB.setName("ArTCMCompassRobot");
  if (myRobot != NULL)
    myRobot->addPacketHandler(&myPacketHandlerCB);
}

AREXPORT ArTCMCompassRobot::~ArTCMCompassRobot()
{
  if (myRobot != NULL)
    myRobot->remPacketHandler(&myPacketHandlerCB);
}

bool ArTCMCompassRobot::packetHandler(ArRobotPacket *packet)
{
  if (packet->getID() != 0xC0)
    return false;
  
  myHeading = ArMath::fixAngle(packet->bufToByte2() / 10.0);
  myPitch = ArMath::fixAngle(packet->bufToByte2() / 10.0);
  myRoll = ArMath::fixAngle(packet->bufToByte2() / 10.0);
  myXMag = packet->bufToByte2() / 100.0;  
  myYMag = packet->bufToByte2() / 100.0;
  myZMag = packet->bufToByte2() / 100.0;
  myTemperature = packet->bufToByte2() / 10.0;
  myError = packet->bufToByte2();
  myCalibrationH = packet->bufToByte();
  myCalibrationV = packet->bufToByte();
  myCalibrationM = packet->bufToByte2() / 100.0;

  myHaveHeading = 
    myHavePitch = 
    myHaveRoll = 
    myHaveXMag = 
    myHaveYMag = 
    myHaveZMag = 
    myHaveTemperature = 
    myHaveCalibrationH =
    myHaveCalibrationV =
    myHaveCalibrationM = true;

  incrementPacketCount();

  invokeHeadingDataCallbacks(myHeading);
  return true;
}

