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
#include "ArRobotBatteryPacketReader.h"
#include "ArRobot.h"
#include "ArRobotPacket.h"
#include "ArCommands.h"

AREXPORT ArRobotBatteryPacketReader::ArRobotBatteryPacketReader(
	ArRobot *robot) : 
  myPacketHandlerCB(this, &ArRobotBatteryPacketReader::packetHandler),
  myConnectCB(this, &ArRobotBatteryPacketReader::connectCallback)
{
  myRobot = robot;
  myPacketArrived = false;
  myNumBatteries = 0;
  myNumBytesPerBattery = 0;
  myRequestedBatteryPackets = false;
  myPacketHandlerCB.setName("ArRobotBatteryPacketReader");
  myConnectCB.setName("ArRobotBatteryPacketReader");
  myRobot->addPacketHandler(&myPacketHandlerCB);
  myRobot->addConnectCB(&myConnectCB);
}

AREXPORT ArRobotBatteryPacketReader::~ArRobotBatteryPacketReader(void)
{
  myRobot->remPacketHandler(&myPacketHandlerCB);
  myRobot->remConnectCB(&myConnectCB);
}

AREXPORT void ArRobotBatteryPacketReader::connectCallback(void)
{
  requestSinglePacket();
}

AREXPORT void ArRobotBatteryPacketReader::requestSinglePacket(void)
{
  myRobot->comInt(ArCommands::BATTERYINFO, 1);
  myRequestedBatteryPackets = false;
}

AREXPORT void ArRobotBatteryPacketReader::requestContinuousPackets(void)
{
  myRobot->comInt(ArCommands::BATTERYINFO, 2);
  myRequestedBatteryPackets = true;
}

AREXPORT void ArRobotBatteryPacketReader::stopPackets(void)
{
  myRobot->comInt(ArCommands::BATTERYINFO, 0);
  myRequestedBatteryPackets = false;
}

AREXPORT bool ArRobotBatteryPacketReader::haveRequestedPackets(void)
{
  return myRequestedBatteryPackets;
}

AREXPORT bool ArRobotBatteryPacketReader::packetHandler(ArRobotPacket *packet)
{
  //char buf[256];

  if (packet->getID() != 0xfa)
    return false;
  
  myFlags1.clear();
  myFlags2.clear();
  myFlags3.clear();
  myRelSOC.clear();
  myAbsSOC.clear();

  myNumBatteries = packet->bufToUByte();
  myNumBytesPerBattery = packet->bufToUByte();

  int battery;
  int byte;
  int val;

  for (battery = 1; battery <= myNumBatteries; battery++)
  {
    for (byte = 1; byte <= myNumBytesPerBattery; byte++)
    {
      val = packet->bufToUByte();
      if (byte == 1)
	myFlags1[battery] = val;
      else if (byte == 2)
	myFlags2[battery] = val;
      else if (byte == 3)
	myFlags3[battery] = val;
      else if (byte == 4)
	myRelSOC[battery] = val;
      else if (byte == 5)
	myAbsSOC[battery] = val;
    }
  }
  myPacketArrived = true;
  return true;
}
