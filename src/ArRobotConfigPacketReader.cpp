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
#include "ArRobotConfigPacketReader.h"
#include "ArRobot.h"
#include "ArRobotPacket.h"
#include "ArCommands.h"

/**
   @param robot is the robot to connect this to

   @param onlyOneRequest if this is true then only one request for a
   packet will ever be honored (so that you can save the settings from
   one point in time)
   
   @param packetArrivedCB a functor to call when the packet comes in,
   note the robot is locked during this callback
 **/
AREXPORT ArRobotConfigPacketReader::ArRobotConfigPacketReader(
	ArRobot *robot, bool onlyOneRequest, ArFunctor *packetArrivedCB) : 
  myPacketHandlerCB(this, &ArRobotConfigPacketReader::packetHandler),
  myConnectedCB(this, &ArRobotConfigPacketReader::connected)
{
  myRobot = robot;
  myPacketHandlerCB.setName("ArRobotConfigPacketReader");
  myRobot->addPacketHandler(&myPacketHandlerCB);
  myRobot->addConnectCB(&myConnectedCB);
  myOnlyOneRequest = onlyOneRequest;
  myPacketRequested = false;
  myPacketArrived = false;
  myPacketArrivedCB = packetArrivedCB;
}

AREXPORT ArRobotConfigPacketReader::~ArRobotConfigPacketReader(void)
{
  myRobot->remPacketHandler(&myPacketHandlerCB);
  myRobot->remConnectCB(&myConnectedCB);
}

AREXPORT bool ArRobotConfigPacketReader::requestPacket(void)
{
  // make sure we haven't already gotten one
  if (myOnlyOneRequest && myPacketArrived)
    return false;

  if (myPacketRequested && myLastPacketRequest.mSecSince() < 200)
    return true;

  myPacketArrived = false;
  myPacketRequested = true;
  myLastPacketRequest.setToNow();
  myRobot->comInt(ArCommands::CONFIG, 1);
  return true;
}

AREXPORT void ArRobotConfigPacketReader::connected(void)
{
  if (myPacketRequested)
    myRobot->comInt(ArCommands::CONFIG, 1);
}

AREXPORT bool ArRobotConfigPacketReader::packetHandler(ArRobotPacket *packet)
{
  char buf[256];

  // if this isn't the right packet ignore it
  if (packet->getID() != 0x20)
    return false;
  // if we've already gotten our one request ignore it
  if (myPacketArrived)
    return false;
  // if we haven't requested a packet ignore it
  if (!myPacketRequested)
    return false;

  myPacketRequested = false;
  myPacketArrived = true;

  //make a copy of all the packet
  myPacket = (*packet);

  // read in all the data
  packet->bufToStr(buf, sizeof(buf));
  myType = buf;
  packet->bufToStr(buf, sizeof(buf));
  mySubType = buf;
  packet->bufToStr(buf, sizeof(buf));
  mySerialNumber = buf;
  packet->bufToUByte();
  myRotVelTop = packet->bufToUByte2();
  myTransVelTop = packet->bufToUByte2();
  myRotAccelTop = packet->bufToUByte2();
  myTransAccelTop = packet->bufToUByte2();
  myPwmMax = packet->bufToUByte2();
  packet->bufToStr(buf, sizeof(buf));
  myName = buf;
  mySipCycleTime = packet->bufToUByte();
  myHostBaud = packet->bufToUByte();
  myAux1Baud = packet->bufToUByte();
  myHasGripper = (bool)packet->bufToUByte2();
  myFrontSonar = (bool) packet->bufToUByte2();
  myRearSonar = (bool) packet->bufToUByte();
  myLowBattery = packet->bufToUByte2();
  myRevCount = packet->bufToUByte2();
  myWatchdog = packet->bufToUByte2();
  myNormalMPacs = (bool) packet->bufToUByte();
  myStallVal = packet->bufToUByte2();
  myStallCount = packet->bufToUByte2();
  myJoyVel = packet->bufToUByte2();
  myJoyRotVel = packet->bufToUByte2();
  myRotVelMax = packet->bufToUByte2();
  myTransVelMax = packet->bufToUByte2();
  myRotAccel = packet->bufToUByte2();
  myRotDecel = packet->bufToUByte2();
  myRotKP = packet->bufToUByte2();
  myRotKV = packet->bufToUByte2();
  myRotKI = packet->bufToUByte2();
  myTransAccel = packet->bufToUByte2();
  myTransDecel = packet->bufToUByte2();
  myTransKP = packet->bufToUByte2();
  myTransKV = packet->bufToUByte2();
  myTransKI = packet->bufToUByte2();
  myFrontBumps = packet->bufToUByte();
  myRearBumps = packet->bufToUByte();
  myHasCharger = packet->bufToUByte();
  mySonarCycle = packet->bufToUByte();
  if (packet->bufToUByte() == 2)
    myResetBaud = true;
  else
    myResetBaud = false;
  myGyroType = packet->bufToUByte();
  if (myGyroType == 1)
    myHasGyro = true;
  else
    myHasGyro = false;
  myDriftFactor = packet->bufToByte2();
  myAux2Baud = packet->bufToUByte();
  myAux3Baud = packet->bufToUByte();
  myTicksMM = packet->bufToUByte2();
  myShutdownVoltage = packet->bufToUByte2();

  packet->bufToStr(buf, sizeof(buf));
  myFirmwareVersion = buf;
  myGyroCW = packet->bufToUByte2();
  myGyroCCW = packet->bufToUByte2();

  myKinematicsDelay = packet->bufToUByte();
  myLatVelTop = packet->bufToUByte2();
  myLatAccelTop = packet->bufToUByte2();
  myLatVelMax = packet->bufToUByte2();
  myLatAccel = packet->bufToUByte2();
  myLatDecel = packet->bufToUByte2();
  myPowerbotChargeThreshold = packet->bufToUByte2();
  myPDBPort = packet->bufToUByte();
  myGyroRateLimit = packet->bufToUByte2();
  if (packet->getDataLength() - packet->getDataReadLength() > 0)
    myHighTemperatureShutdown = packet->bufToByte();
  else
    myHighTemperatureShutdown = -128;
  myPowerBits = packet->bufToUByte2();
  myBatteryType = packet->bufToUByte();
  myStateOfChargeLow = packet->bufToUByte2();
  myStateOfChargeShutdown = packet->bufToUByte2();

  packet->bufToStr(buf, sizeof(buf));
  myFirmwareBootloaderVersion = buf;

  myConfigFlags = packet->bufToUByte4();

  myGyroFWVersion = packet->bufToByte2();

  if (myPacketArrivedCB != NULL)
  {
    myPacketArrivedCB->invoke();
  }
  return true;
}
  
AREXPORT void ArRobotConfigPacketReader::log(void) const
{
  std::string str;
  str = buildString();
  ArLog::log(ArLog::Terse, str.c_str());
}

AREXPORT void ArRobotConfigPacketReader::logMovement(void) const
{
  std::string str;
  str = buildStringMovement();
  ArLog::log(ArLog::Terse, str.c_str());
}

/**
   Like most memory stuff this won't work across DLL's in windows,
   it should work fine in linux or with static library files in windows.
**/
AREXPORT std::string ArRobotConfigPacketReader::buildString(void) const
{
  std::string ret;

  char line[32000];
  sprintf(line, "General information:\n");
  ret += line;
  sprintf(line, "Robot is type '%s' subtype '%s'\n",
	     getType(), getSubType());
  ret += line;
  sprintf(line, "serial number '%s' name '%s'\n", 
	     getSerialNumber(), getName());
  ret += line;
  sprintf(line, "firmware version '%s'\n",
	  getFirmwareVersion());
  ret += line;
  sprintf(line, "firmware bootloader version '%s'\n",
	  getFirmwareBootloaderVersion());
  ret += line;

   sprintf(line, "gyro firmware version %d\n",
	  getGyroFWVersion());

  ret += line;
  int flags = getConfigFlags();
  
  sprintf(line, "configFlags 0x%08x\n", flags);
  ret += line;

  sprintf(line, "\t(IsBootloader=%d, CanDownloadMTXFirmware=%d, CanDownloadMTXBootloader=%d, NotLegacyGyro=%d, MTXConfigSystem=%d)\n",
    (flags & ArUtil::BIT0), 
    (flags & ArUtil::BIT1), 
    (flags & ArUtil::BIT2), 
    (flags & ArUtil::BIT3), 
    (flags & ArUtil::BIT4)
  );
  ret += line;

  sprintf(line, "Intrinsic properties and unsettable maxes:\n");
  ret += line;
  sprintf(line, "TransVelTop %d TransAccelTop %d\n", 
	     getTransVelTop(), getTransAccelTop());
  ret += line;
  sprintf(line, "RotVelTop %d RotAccelTop %d\n", 
	     getRotVelTop(), getRotAccelTop());
  ret += line;
  if (myRobot->hasLatVel())
  {
    sprintf(line, "LatVelTop %d LatAccelTop %d\n", 
	    getLatVelTop(), getLatAccelTop());
    ret += line;
  }
  sprintf(line, "PWMMax %d ResetBaud %s\n", getPwmMax(),
	     ArUtil::convertBool(getResetBaud()));
  ret += line;
  sprintf(line, "Current values:\n");
  ret += line;
  sprintf(line, "TransVelMax %d TransAccel %d TransDecel %d\n", 
	     getTransVelMax(), getTransAccel(), getTransDecel());
  ret += line;  
  sprintf(line, "RotVelMax %d RotAccel %d RotDecel %d\n", 
	     getRotVelMax(), getRotAccel(), getRotDecel());
  ret += line;
  if (myRobot->hasLatVel())
  {
    sprintf(line, "LatVelMax %d LatAccel %d LatDecel %d\n", 
	    getLatVelMax(), getLatAccel(), getLatDecel());
    ret += line;  
  }
  sprintf(line, "Accessories:\n");
  ret += line;  
  sprintf(line, 
	  "Gripper %s FrontSonar %s RearSonar %s Charger %d GyroType %d\n", 
	  ArUtil::convertBool(getHasGripper()), 
	  ArUtil::convertBool(getFrontSonar()), 
	  ArUtil::convertBool(getRearSonar()), 
	  getHasCharger(),
	  getGyroType());
  ret += line;  
  sprintf(line, "FrontBumps %d RearBumps %d\n", 
	     getFrontBumps(), getRearBumps());
  ret += line;  
  sprintf(line, "Settings:\n");
  ret += line;  
  sprintf(line, "SipCycle %d SonarCycle %d HostBaud %d Aux1Baud %d\n", getSipCycleTime(), getSonarCycle(), getHostBaud(), getAux1Baud());
  ret += line;  
  sprintf(line, "StallVal %d StallCount %d RevCount %d Watchdog %d\n",
	     getStallVal(), getStallCount(), getRevCount(), getWatchdog());
  ret += line;  
  sprintf(line, "GyroRateLimit %d\n",
	  getGyroRateLimit());
  ret += line;  
  sprintf(line, "JoyVel %d JoyRVel %d NormalMotorPackets %s\n", getJoyVel(), getJoyRotVel(), ArUtil::convertBool(getNormalMPacs()));
  ret += line;  
  sprintf(line, "PID Settings:\n");
  ret += line;  
  sprintf(line, "Trans kp %d kv %d ki %d\n", getTransKP(), 
	     getTransKV(), getTransKI());
  ret += line;  
  sprintf(line, "Rot kp %d kv %d ki %d\n", getRotKP(), getRotKV(),
	     getRotKI());
  ret += line;  
  sprintf(line, "Other:\n");
  ret += line;  
  sprintf(line, "DriftFactor %d KinematicsDelay %d\n", getDriftFactor(),
	  getKinematicsDelay());
  ret += line;  
  sprintf(line, "Aux2Baud setting %d Aux3Baud setting %d\n", getAux2Baud(), getAux3Baud());
  ret += line;  
  sprintf(line, "PDBPort setting %d BatteryType %d\n", getPDBPort(), 
	  getBatteryType());
  ret += line;  
  sprintf(line, "TicksMM %d GyroCW %d GyroCCW %d\n", getTicksMM(),
	  getGyroCW(), getGyroCCW());
  ret += line;  
  sprintf(line, 
	  "LowBattery %d ShutdownVoltage %d PowerbotChargeThreshold %d\n", 
	  getLowBattery(), getShutdownVoltage(), getPowerbotChargeThreshold());
  ret += line;  
  sprintf(line, 
	  "LowStateOfCharge %d ShutdownStateOfCharge %d\n", 
	  getStateOfChargeLow(), getStateOfChargeShutdown());
  ret += line;  

  char buf[128];
  int i, j;
  unsigned int value = getPowerBits();
  int bit;
  buf[0] = '\0';
  for (j = 0, bit = 1; j < 16; ++j, bit *= 2)
  {
    if (j == 8)
      sprintf(buf, "%s ", buf);
    if (value & bit)
      sprintf(buf, "%s%d", buf, 1);
    else
      sprintf(buf, "%s%d", buf, 0);
  }

  sprintf(line, "HighTempShutdown %d PowerBits %s\n",
	  getHighTemperatureShutdown(), buf);
  ret += line;  

  return ret;
}


/**
   Like most memory stuff this won't work across DLL's in windows,
   it should work fine in linux or with static library files in windows.
**/
AREXPORT std::string ArRobotConfigPacketReader::buildStringMovement(void) const
{
  std::string ret;

  char line[32000];
  sprintf(line, "TransVelMax %d TransAccel %d TransDecel %d\n", 
	     getTransVelMax(), getTransAccel(), getTransDecel());
  ret += line;  
  sprintf(line, "RotVelMax %d RotAccel %d RotDecel %d\n", 
	     getRotVelMax(), getRotAccel(), getRotDecel());
  ret += line;
  if (myRobot->hasLatVel())
  {
    sprintf(line, "LatVelMax %d LatAccel %d LatDecel %d\n", 
	    getLatVelMax(), getLatAccel(), getLatDecel());
    ret += line;  
  }
  return ret;
}
