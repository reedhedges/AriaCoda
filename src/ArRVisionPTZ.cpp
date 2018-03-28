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
#include "ArRVisionPTZ.h"
#include "ArRobot.h"
#include "ArCommands.h"

AREXPORT ArRVisionPacket::ArRVisionPacket(ArTypes::UByte2 bufferSize) :
  ArBasePacket(bufferSize)
{

}

AREXPORT ArRVisionPacket::~ArRVisionPacket()
{

}

AREXPORT void ArRVisionPacket::uByteToBuf(ArTypes::UByte val)
{
  if (myLength + 1 > myMaxLength)
  {
    ArLog::log(ArLog::Terse, "ArRVisionPacket::uByteToBuf: Trying to add beyond length of buffer.");
    return;
  }
  myBuf[myLength] = val;
  ++myLength;
}

AREXPORT void ArRVisionPacket::byte2ToBuf(ArTypes::Byte2 val)
{
  if ((myLength + 4) > myMaxLength)
  {
    ArLog::log(ArLog::Terse, "ArRVisionPacket::Byte2ToBuf: Trying to add beyond length of buffer.");
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
   a character array.  This is used by the default rvision commands, unless
   you have a deep understanding of how the packets are working and what
   the packet structure looks like you should not play with this function, 
   it also isn't worth it unless you'll be sending commands frequently.
   @param val the Byte2 to put into the packet
   @param pose the position in the packets array to put the value
*/
AREXPORT void ArRVisionPacket::byte2ToBufAtPos(ArTypes::Byte2 val,
					    ArTypes::UByte2 pose)
{
  ArTypes::Byte2 prevLength = myLength;

  if ((pose + 4) > myMaxLength)
  {
    ArLog::log(ArLog::Terse, "ArRVisionPacket::Byte2ToBuf: Trying to add beyond length of buffer.");
    return;
  }
  myLength = pose;
  byte2ToBuf(val);
  myLength = prevLength;
}


AREXPORT ArRVisionPTZ::ArRVisionPTZ(ArRobot *robot) :
  ArPTZ(NULL),
  myPacket(255), 
  myZoomPacket(9),
  mySerialPort(ArUtil::COM4)
{
  //myRobot = robot;
  initializePackets();
  
  // these ticks were derived emperically.  Ticks / real_degrees
  myDegToTilt = 2880 / 180;
  myDegToPan =  960 / 60;

  myTiltOffsetInDegrees = TILT_OFFSET_IN_DEGREES; 
  myPanOffsetInDegrees = PAN_OFFSET_IN_DEGREES; 

  myConn = NULL;

  ArPTZ::setLimits(MAX_PAN, MIN_PAN, MAX_TILT, MIN_TILT, MAX_ZOOM, MIN_ZOOM);
    /*
  AREXPORT virtual double getMaxPosPan(void) const { return MAX_PAN; }
  AREXPORT virtual double getMaxNegPan(void) const { return MIN_PAN; }
  AREXPORT virtual double getMaxPosTilt(void) const { return MAX_TILT; }
  AREXPORT virtual double getMaxNegTilt(void) const { return MIN_TILT; }
  AREXPORT virtual int getMaxZoom(void) const { return MAX_ZOOM; }
  AREXPORT virtual int getMinZoom(void) const { return MIN_ZOOM; }
  */

}

AREXPORT ArRVisionPTZ::~ArRVisionPTZ()
{
}

void ArRVisionPTZ::initializePackets(void)
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


AREXPORT bool ArRVisionPTZ::init(void)
{
  // send command to power on camera on seekur
  if(myRobot)
  {
	  ArLog::log(ArLog::Normal, "ArRVisionPTZ: turning camera power on ...");
	  myRobot->com2Bytes(116, 12, 1);
  }
  
  myConn = getDeviceConnection();
  if(!myConn)
  {
     ArLog::log(ArLog::Normal, "ArRVisionPTZ: new connection to camera on %s...", mySerialPort);
     ArSerialConnection *ser = new ArSerialConnection();
     if(ser->open(mySerialPort) != 0)
     {
	ArLog::log(ArLog::Terse, "ArRVisionPTZ: error opening %s for camera PTZ control, initialization failed.", mySerialPort);
        myConn = NULL;
        return false;
     }
     myConn = ser;
  }
     
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
  {
    ArLog::log(ArLog::Terse, "ArRVisionPTZ: Error sending initialization packet to RVision camera!");
    return false;
  }
  if (!panTilt(0, 0))
    return false;
  if (!zoom(0))
    return false;
  return true;
}

AREXPORT bool ArRVisionPTZ::panTilt_i(double degreesPan, double degreesTilt)
{
  if (degreesPan > MAX_PAN)
    degreesPan = MAX_PAN;
  if (degreesPan < MIN_PAN)
    degreesPan = MIN_PAN;
  myPan = degreesPan;

  if (degreesTilt > MAX_TILT)
    degreesTilt = MAX_TILT;
  if (degreesTilt < MIN_TILT)
    degreesTilt = MIN_TILT;
  myTilt = degreesTilt;

  myPanTiltPacket.byte2ToBufAtPos(ArMath::roundInt((myPan+myPanOffsetInDegrees) * myDegToPan), 6);
  myPanTiltPacket.byte2ToBufAtPos(ArMath::roundInt((myTilt+myTiltOffsetInDegrees) * myDegToTilt), 10);
  return sendPacket(&myPanTiltPacket);
}

AREXPORT bool ArRVisionPTZ::panTiltRel_i(double degreesPan, double degreesTilt)
{
  return panTilt_i(myPan + degreesPan, myTilt + degreesTilt);
}

AREXPORT bool ArRVisionPTZ::pan_i(double degrees)
{
  return panTilt_i(degrees, myTilt);
}

AREXPORT bool ArRVisionPTZ::panRel_i(double degrees)
{
  return panTiltRel_i(degrees, 0);
}

AREXPORT bool ArRVisionPTZ::tilt_i(double degrees)
{
  return panTilt_i(myPan, degrees);
}

AREXPORT bool ArRVisionPTZ::tiltRel_i(double degrees)
{
  return panTiltRel_i(0, degrees);
}

AREXPORT bool ArRVisionPTZ::zoom(int zoomValue)
{
  //printf("ArRVision::zoom(%d)\n", zoomValue);
  if (zoomValue > MAX_ZOOM)
    zoomValue = MAX_ZOOM;
  if (zoomValue < MIN_ZOOM)
    zoomValue = MIN_ZOOM;
  myZoom = zoomValue;
    
  myZoomPacket.byte2ToBufAtPos(ArMath::roundInt(myZoom), 4);
  return sendPacket(&myZoomPacket);
}

AREXPORT bool ArRVisionPTZ::zoomRel(int zoomValue)
{
  return zoom(myZoom + zoomValue);
}

/*
AREXPORT bool ArRVisionPTZ::packetHandler(ArRobotPacket *packet)
{
  if (packet->getID() != 0xE0)
    return false;

  return true;
}
*/

#define ARRVISION_MAX_RESPONSE_BYTES 16
//AREXPORT bool ArRVisionPTZ::packetHandler(ArBasePacket *packet)
ArBasePacket * ArRVisionPTZ::readPacket(void)
{
  unsigned char data[ARRVISION_MAX_RESPONSE_BYTES];
  unsigned char byte;
  int num;
  memset(data, 0, ARRVISION_MAX_RESPONSE_BYTES);
  for (num=0; num <= ARRVISION_MAX_RESPONSE_BYTES+1; num++) {
    if (myConn->read((char *) &byte, 1,1) <= 0 ||
	num == ARRVISION_MAX_RESPONSE_BYTES+1) {
      return NULL;
    }
    else if (byte == 0x90) {
      data[0] = byte;
      //printf("ArRVisionPTZ::packetHandler:  got 0x%x, expecting packet\n", byte);
      break;
    }
    else {
      //printf("ArRVisionPTZ::packetHandler:  got 0x%x, skipping\n", byte);
    }
  }
  // we got the header
  for (num=1; num <= ARRVISION_MAX_RESPONSE_BYTES; num++) {
    if (myConn->read((char *) &byte, 1, 1) <= 0) {
      // there are no more bytes, so check the last byte for the footer
      if (data[num-1] != 0xFF) {
	//printf("ArRVisionPTZ::packetHandler: should have gotten 0xFF, got 0x%x\n", data[num-1]);
	return NULL;
      }
      else {
	break;
      }
    }
    else {
      // add the byte to the array
      data[num] = byte;
    }
  }
  // print the data for now
  //printf("ArRVisionPTZ::packetHandler: got packet!\n");
  //for (int i=0; i <= num; i++) {
  //printf("\t[%d]: 0x%x\n", i, data[i]);
  //}
  return NULL;
}

ArPTZConnector::GlobalPTZCreateFunc ArRVisionPTZ::ourCreateFunc(&ArRVisionPTZ::create);

ArPTZ* ArRVisionPTZ::create(size_t index, ArPTZParams params, ArArgumentParser *parser, ArRobot *robot)
{
  ArRVisionPTZ *ptz = new ArRVisionPTZ(robot);
  if(params.serialPort != "" && params.serialPort != "none")
	  ptz->setPort(params.serialPort.c_str());
  return ptz;
}

void ArRVisionPTZ::registerPTZType()
{
  ArPTZConnector::registerPTZType("rvision", &ourCreateFunc);
}
