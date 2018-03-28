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
#include "ArSonyPTZ.h"
#include "ArRobot.h"
#include "ArCommands.h"

AREXPORT ArSonyPacket::ArSonyPacket(ArTypes::UByte2 bufferSize) :
  ArBasePacket(bufferSize)
{
  
}

AREXPORT ArSonyPacket::~ArSonyPacket()
{

}

AREXPORT void ArSonyPacket::uByteToBuf(ArTypes::UByte val)
{
  if (myLength + 1 > myMaxLength)
  {
    ArLog::log(ArLog::Terse, "ArSonyPacket::uByteToBuf: Trying to add beyond length of buffer.");
    return;
  }
  myBuf[myLength] = val;
  ++myLength;
}

AREXPORT void ArSonyPacket::byte2ToBuf(ArTypes::Byte2 val)
{
  if ((myLength + 4) > myMaxLength)
  {
    ArLog::log(ArLog::Terse, "ArSonyPacket::Byte2ToBuf: Trying to add beyond length of buffer.");
    return;
  }
  myBuf[myLength] = (val & 0xf000) >> 12;
  ++myLength;
  myBuf[myLength] = (val & 0x0f00) >> 8;
  ++myLength;
  myBuf[myLength] = (val & 0x00f0) >> 4;
  ++myLength;
  myBuf[myLength] = (val & 0x000f) >> 0;
  ++myLength;
}

/**
   This function is my concession to not rebuilding a packet from scratch
   for every command, basicaly this is to not lose all speed over just using
   a character array.  This is used by the default sony commands, unless
   you have a deep understanding of how the packets are working and what
   the packet structure looks like you should not play with this function, 
   it also isn't worth it unless you'll be sending commands frequently.
   @param val the Byte2 to put into the packet
   @param pose the position in the packets array to put the value
*/
AREXPORT void ArSonyPacket::byte2ToBufAtPos(ArTypes::Byte2 val,
					    ArTypes::UByte2 pose)
{
  ArTypes::Byte2 prevLength = myLength;

  if ((pose + 4) > myMaxLength)
  {
    ArLog::log(ArLog::Terse, "ArSonyPacket::Byte2ToBuf: Trying to add beyond length of buffer.");
    return;
  }
  myLength = pose;
  byte2ToBuf(val);
  myLength = prevLength;
}


AREXPORT ArSonyPTZ::ArSonyPTZ(ArRobot *robot) :
  ArPTZ(robot),
  myPacket(255), 
  myZoomPacket(9)
{
  myRobot = robot;
  initializePackets();

  setLimits(90, -90, 30, -30, 1024, 0);
    /*
  AREXPORT virtual double getMaxPosPan(void) const { return 90; }
  AREXPORT virtual double getMaxNegPan(void) const { return -90; }
  AREXPORT virtual double getMaxPosTilt(void) const { return 30; }
  AREXPORT virtual double getMaxNegTilt(void) const { return -30; }
  AREXPORT virtual int getMaxZoom(void) const { return 1024; }
  AREXPORT virtual int getMinZoom(void) const { return 0; }
  */
  
  myDegToTilt = 0x12c / ((double) getMaxTilt() /*MAX_TILT*/ );
  myDegToPan = 0x370 / ((double) getMaxPan() /*MAX_PAN*/ );
}

AREXPORT ArSonyPTZ::~ArSonyPTZ()
{
}

void ArSonyPTZ::initializePackets(void)
{
  myZoomPacket.empty();
  myZoomPacket.uByteToBuf(0x81);
  myZoomPacket.uByteToBuf(0x01);
  myZoomPacket.uByteToBuf(0x04);
  myZoomPacket.uByteToBuf(0x47);
  myZoomPacket.uByteToBuf(0x00);
  myZoomPacket.uByteToBuf(0x00);
  myZoomPacket.uByteToBuf(0x00);
  myZoomPacket.uByteToBuf(0x00);
  myZoomPacket.uByteToBuf(0xff);

  myPanTiltPacket.empty();
  myPanTiltPacket.uByteToBuf(0x81);
  myPanTiltPacket.uByteToBuf(0x01);
  myPanTiltPacket.uByteToBuf(0x06);
  myPanTiltPacket.uByteToBuf(0x02);
  myPanTiltPacket.uByteToBuf(0x18);
  myPanTiltPacket.uByteToBuf(0x14);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0x00);
  myPanTiltPacket.uByteToBuf(0xff);
}


AREXPORT bool ArSonyPTZ::init(void)
{
  myPacket.empty();
  myPacket.uByteToBuf(0x88);
  myPacket.uByteToBuf(0x01);
  myPacket.uByteToBuf(0x00);
  myPacket.uByteToBuf(0x01);
  myPacket.uByteToBuf(0xff);
  myPacket.uByteToBuf(0x88);
  myPacket.uByteToBuf(0x30);
  myPacket.uByteToBuf(0x01);
  myPacket.uByteToBuf(0xff);

  if (!sendPacket(&myPacket))
    return false;
  if (!panTilt(0, 0))
    return false;
  if (!zoom(0))
    return false;
  return true;
}

AREXPORT bool ArSonyPTZ::backLightingOn(void)
{
  myPacket.empty();
  myPacket.uByteToBuf(0x81);
  myPacket.uByteToBuf(0x01);
  myPacket.uByteToBuf(0x04);
  myPacket.uByteToBuf(0x33);
  myPacket.uByteToBuf(0x02);
  myPacket.uByteToBuf(0xff);

  return sendPacket(&myPacket);
}

AREXPORT bool ArSonyPTZ::backLightingOff(void)
{
  myPacket.empty();
  myPacket.uByteToBuf(0x81);
  myPacket.uByteToBuf(0x01);
  myPacket.uByteToBuf(0x04);
  myPacket.uByteToBuf(0x33);
  myPacket.uByteToBuf(0x03);
  myPacket.uByteToBuf(0xff);

  return sendPacket(&myPacket);
}

AREXPORT bool ArSonyPTZ::panTilt_i(double degreesPan, double degreesTilt)
{
  if (degreesPan > getMaxPan())
    degreesPan = getMaxPan();
  if (degreesPan < getMinPan())
    degreesPan = getMinPan();
  myPan = degreesPan;

  if (degreesTilt > getMaxTilt())
    degreesTilt = getMaxTilt();
  if (degreesTilt < getMinTilt())
    degreesTilt = getMinTilt();
  myTilt = degreesTilt;

  myPanTiltPacket.byte2ToBufAtPos(ArMath::roundInt(myPan * myDegToPan), 6);
  myPanTiltPacket.byte2ToBufAtPos(ArMath::roundInt(myTilt * myDegToTilt), 10);
  return sendPacket(&myPanTiltPacket);
}

AREXPORT bool ArSonyPTZ::panTiltRel_i(double degreesPan, double degreesTilt)
{
  return panTilt(myPan + degreesPan, myTilt + degreesTilt);
}

AREXPORT bool ArSonyPTZ::pan_i(double degrees)
{
  return panTilt(degrees, myTilt);
}

AREXPORT bool ArSonyPTZ::panRel_i(double degrees)
{
  return panTiltRel(degrees, 0);
}

AREXPORT bool ArSonyPTZ::tilt_i(double degrees)
{
  return panTilt(myPan, degrees);
}

AREXPORT bool ArSonyPTZ::tiltRel_i(double degrees)
{
  return panTiltRel(0, degrees);
}

AREXPORT bool ArSonyPTZ::zoom(int zoomValue)
{
  if (zoomValue > getMaxZoom())
    zoomValue = getMaxZoom();
  if (zoomValue < getMinZoom())
    zoomValue = getMinZoom();
  myZoom = zoomValue;
    
  myZoomPacket.byte2ToBufAtPos(ArMath::roundInt(myZoom), 4);
  return sendPacket(&myZoomPacket);
}

AREXPORT bool ArSonyPTZ::zoomRel(int zoomValue)
{
  return zoom(myZoom + zoomValue);
}


/*
AREXPORT bool ArSonyPTZ::packetHandler(ArRobotPacket *packet)
{
  if (packet->getID() != 0xE0)
    return false;

  return true;
}
*/

ArPTZConnector::GlobalPTZCreateFunc ArSonyPTZ::ourCreateFunc(&ArSonyPTZ::create);

ArPTZ* ArSonyPTZ::create(size_t index, ArPTZParams params, ArArgumentParser *parser, ArRobot *robot)
{
  return new ArSonyPTZ(robot);
}

void ArSonyPTZ::registerPTZType()
{
  ArPTZConnector::registerPTZType("sony", &ourCreateFunc);
}
