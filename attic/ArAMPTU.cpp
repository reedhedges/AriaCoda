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
#include "ArAMPTU.h"
#include "ArCommands.h"
#include "ArLog.h"
#include "ArRobot.h"

AREXPORT ArAMPTUPacket::ArAMPTUPacket(ArTypes::UByte2 bufferSize) :
  ArBasePacket(bufferSize, 3)
{
  myUnitNumber = 0;
}

AREXPORT ArAMPTUPacket::~ArAMPTUPacket()
{

}

AREXPORT void ArAMPTUPacket::byteToBuf(ArTypes::Byte val)
{
  if (myLength + 1 > myMaxLength)
  {
    ArLog::log(ArLog::Terse, "ArAMPTUPacket::uByteToBuf: Trying to add beyond length of buffer.");
    return;
  }
  myBuf[myLength] = val;
  ++myLength;
}

AREXPORT void ArAMPTUPacket::byte2ToBuf(ArTypes::Byte2 val)
{
  if ((myLength + 2) > myMaxLength)
  {
    ArLog::log(ArLog::Terse, "ArAMPTUPacket::Byte2ToBuf: Trying to add beyond length of buffer.");
    return;
  }
  myBuf[myLength] = val/255;//(val & 0xff00) >> 8;
  ++myLength;
  myBuf[myLength] = val%255;//(val & 0x00ff);
  ++myLength;
}

AREXPORT void ArAMPTUPacket::finalizePacket(void)
{
  int length = myLength;
  myLength = 0;
  byteToBuf('P');
  byteToBuf('T');
  byteToBuf('0' + myUnitNumber);
  myLength = length;
}

/**
   Each AMPTU has a unit number, so that you can daisy chain multiple ones
   together.  This number is incorporated into the packet header, thus the
   packet has to know what the number is.
   @return the unit number this packet has
*/
AREXPORT unsigned char ArAMPTUPacket::getUnitNumber(void)
{
  return myUnitNumber;
}

/**
   Each AMPTU has a unit number, so that you can daisy chain multiple ones
   together.  This number is incorporated into the packet header, thus the
   packet has to know what the number is.
   @param unitNumber the unit number for this packet, this needs to be 0-7
   @return true if the number is acceptable, false otherwise
*/
AREXPORT bool ArAMPTUPacket::setUnitNumber(unsigned char unitNumber)
{
  if (unitNumber > 7)
    return false;

  myUnitNumber = unitNumber;    
  return true;
}

/**
   @param robot the robot to attach to
   @param unitNumber the unit number for this packet, this needs to be 0-7
*/
AREXPORT ArAMPTU::ArAMPTU(ArRobot *robot, int unitNumber) :
  ArPTZ(robot)
{
  myRobot = robot;
  myPanSlew = 0;
  myTiltSlew = 0;
  myPan = 0;
  myTilt = 0;
  myUnitNumber = unitNumber;
  ArPTZ::setLimits(150, -150, 90, -90);
}

AREXPORT ArAMPTU::~ArAMPTU()
{
}

AREXPORT bool ArAMPTU::init(void)
{
  if (!myPacket.setUnitNumber(myUnitNumber))
  {
    ArLog::log(ArLog::Terse, "ArAMPTU::init: the unit number is invalid.");
    return false;
  }
  myPacket.empty();
  myPacket.byteToBuf(ArAMPTUCommands::INIT);
  if (!sendPacket(&myPacket))
    return false;
  
  myPacket.empty();
  myPacket.byteToBuf(ArAMPTUCommands::RESP);
  myPacket.byteToBuf(0);
  if (!sendPacket(&myPacket))
    return false;
  
  if (!panTilt(0, 0))
    return false;

  return true;
}

AREXPORT bool ArAMPTU::pan_i(double deg)
{
  if (deg > getMaxPosPan_i())
    deg = getMaxPosPan_i();
  if (deg < getMaxNegPan_i())
    deg = getMaxNegPan_i();

  myPacket.empty();
  myPacket.byteToBuf(ArAMPTUCommands::ABSPAN);
  myPacket.byte2ToBuf(ArMath::roundInt(deg + 
				       (getMaxPosPan_i() - getMaxNegPan_i())/2));
  
  myPan = deg;
  return sendPacket(&myPacket);
}

AREXPORT bool ArAMPTU::panRel_i(double deg)
{
  if (deg + myPan > getMaxPosPan_i())
    deg = getMaxPosPan_i() - myPan;
  if (deg + myPan < getMaxNegPan_i())
    deg = getMaxNegPan_i() - myPan;

  myPan += deg;
  myPacket.empty();

  if (deg >= 0)
    myPacket.byteToBuf(ArAMPTUCommands::RELPANCW);
  else
    myPacket.byteToBuf(ArAMPTUCommands::RELPANCCW);
  
  myPacket.byte2ToBuf(ArMath::roundInt(fabs(deg)));
  
  return sendPacket(&myPacket);
}

AREXPORT bool ArAMPTU::tilt_i(double deg)
{
  if (deg > getMaxPosTilt_i())
    deg = getMaxPosTilt_i();
  if (deg < getMaxNegTilt_i())
    deg = getMaxNegTilt_i();

  myPacket.empty();
  myPacket.byteToBuf(ArAMPTUCommands::ABSTILT);
  myPacket.byteToBuf(ArMath::roundInt(deg + 
				      (getMaxPosTilt_i() - getMaxNegTilt_i())/2));
  
  myTilt = deg;
  return sendPacket(&myPacket);
}

AREXPORT bool ArAMPTU::tiltRel_i(double deg)
{
  if (deg + myTilt > getMaxPosTilt_i())
    deg = getMaxPosTilt_i() - myTilt;
  if (deg + myTilt < getMaxNegTilt_i())
    deg = getMaxNegTilt_i() - myTilt;

  myTilt += deg;
  myPacket.empty();

  if (deg >= 0)
    myPacket.byteToBuf(ArAMPTUCommands::RELTILTU);
  else
    myPacket.byteToBuf(ArAMPTUCommands::RELTILTD);
  
  myPacket.byteToBuf(ArMath::roundInt(fabs(deg)));
  
  return sendPacket(&myPacket);
}

AREXPORT bool ArAMPTU::panTilt_i(double panDeg, double tiltDeg)
{
  if (panDeg > getMaxPosPan_i())
    panDeg = getMaxPosPan_i();
  if (panDeg < getMaxNegPan_i())
    panDeg = getMaxNegPan_i();

  if (tiltDeg > getMaxPosTilt_i())
    tiltDeg = getMaxPosTilt_i();
  if (tiltDeg < getMaxNegTilt_i())
    tiltDeg = getMaxNegTilt_i();

  if (myPan - panDeg == 0 && myTilt - tiltDeg == 0)
    return true;
  if (myPan - panDeg == 0) 
    return tilt(tiltDeg);
  if (myTilt - tiltDeg == 0)
    return pan(panDeg);
  myPan = panDeg;
  myTilt = tiltDeg;



  myPacket.empty();
  myPacket.byteToBuf(ArAMPTUCommands::PANTILT);
  myPacket.byte2ToBuf(ArMath::roundInt(myPan + 
				       (getMaxPosPan_i() - getMaxNegPan_i())/2));
  myPacket.byteToBuf(ArMath::roundInt(myTilt + (getMaxPosTilt_i() - 
						getMaxNegTilt_i())/2));
  return sendPacket(&myPacket);
}

AREXPORT bool ArAMPTU::panTiltRel_i(double panDeg, double tiltDeg)
{
  if (panDeg + myPan > getMaxPosPan_i())
    panDeg = getMaxPosPan_i() - myPan;
  if (panDeg + myPan < getMaxNegPan_i())
    panDeg = getMaxNegPan_i() - myPan;

  if (tiltDeg + myTilt > getMaxPosTilt_i())
    tiltDeg = getMaxPosTilt_i() - myTilt;
  if (tiltDeg + myTilt < getMaxNegTilt_i())
    tiltDeg = getMaxNegTilt_i() - myTilt;

  myPan += panDeg;
  myTilt += tiltDeg;

  if (panDeg == 0 && tiltDeg == 0)
    return true;
  if (panDeg == 0) 
    return tiltRel(tiltDeg);
  if (tiltDeg == 0)
    return panRel(panDeg);

  myPacket.empty();
  if (panDeg >= 0 && tiltDeg >= 0)
    myPacket.byteToBuf(ArAMPTUCommands::PANTILTUCW);
  else if (panDeg >= 0 && tiltDeg < 0)
    myPacket.byteToBuf(ArAMPTUCommands::PANTILTDCW);
  else if (panDeg < 0 && tiltDeg >= 0)
    myPacket.byteToBuf(ArAMPTUCommands::PANTILTUCCW);
  else
    myPacket.byteToBuf(ArAMPTUCommands::PANTILTDCCW);

  myPacket.byte2ToBuf(ArMath::roundInt(fabs(panDeg)));
  myPacket.byte2ToBuf(ArMath::roundInt(fabs(tiltDeg)));

  return sendPacket(&myPacket);
}

AREXPORT bool ArAMPTU::panSlew(double deg)
{
  if (deg > MAX_PAN_SLEW)
    deg = MAX_PAN_SLEW;
  if (deg < MIN_SLEW)
    deg = MIN_SLEW;
  
  myPanSlew = deg;
  myPacket.empty();
  myPacket.byteToBuf(ArAMPTUCommands::PANSLEW);
  myPacket.byteToBuf((int)(256 - (3840 / (float)deg)));
  return sendPacket(&myPacket);
}

AREXPORT bool ArAMPTU::tiltSlew(double deg)
{
  if (deg > MAX_TILT_SLEW)
    deg = MAX_TILT_SLEW;
  if (deg < MIN_SLEW)
    deg = MIN_SLEW;
  
  myTiltSlew = deg;
  myPacket.empty();
  myPacket.byteToBuf(ArAMPTUCommands::TILTSLEW);
  myPacket.byteToBuf((int)(256 - (3840 / (float)deg)));
  return sendPacket(&myPacket);
}

AREXPORT bool ArAMPTU::pause(void)
{
  myPacket.empty();
  myPacket.byteToBuf(ArAMPTUCommands::PAUSE);
  return sendPacket(&myPacket);
}

AREXPORT bool ArAMPTU::resume(void)
{
  myPacket.empty();
  myPacket.byteToBuf(ArAMPTUCommands::CONT);
  return sendPacket(&myPacket);
}

AREXPORT bool ArAMPTU::purge(void)
{
  myPacket.empty();
  myPacket.byteToBuf(ArAMPTUCommands::PURGE);
  return sendPacket(&myPacket);
}

AREXPORT bool ArAMPTU::requestStatus(void)
{
  myPacket.empty();
  myPacket.byteToBuf(ArAMPTUCommands::STATUS);
  return sendPacket(&myPacket);
}

