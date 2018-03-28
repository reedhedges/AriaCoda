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
#include "ArDeviceConnection.h"

bool ArDeviceConnection::ourStrMapInited = false;
ArStrMap ArDeviceConnection::ourStrMap;
bool ArDeviceConnection::ourDCDebugShouldLog = false;
ArTime ArDeviceConnection::ourDCDebugFirstTime;

/**
   Subclasses of this connection type should call setDCPortType in
   their constructor, then setDCPortName in their openPort.
   
   Classes that use a device connection should call setDCDeviceName
   with the device the connection is attached too (usually in
   setDeviceConnection on the device)...

   Things that read the port should call debugStartPacket whenever
   they are starting reading a packet...  Then debugBytesRead with the
   byte count whenever they read or fail a read (0 if a read is
   failed)...  Then debugEndPacket with a boolean for if the packet
   was a good one or a bad one and an integer that's postive for the
   type of packet (if successful) or negative to express why the read
   failed (if not successful).  For the 'why the read failed' the
   values should be negative, and should all be unique (anywhere a
   read can return), preferably with a gap between the values, so that
   if more debugging is needed there are values in between the ones
   already there.  Generally this information isn't used or computed,
   unless the global member ArDeviceConnection::debugShouldLog is
   called to turn it on.
**/
AREXPORT ArDeviceConnection::ArDeviceConnection()
{
  if (!ourStrMapInited)
  {
    ourStrMapInited = true;
    buildStrMap();
  }

  myDCPortName = "Unknown port name";
  myDCPortType = "Unknown port type";
  myDCDeviceName = "Unknown device type";

  myDCDebugPacketStarted = false;
  myDCDebugBytesRead = 0;
  myDCDebugTimesRead = 0;
  myDCDebugNumGoodPackets = 0;
  myDCDebugNumBadPackets = 0;
}

AREXPORT ArDeviceConnection::~ArDeviceConnection()
{
  close();
}


void ArDeviceConnection::buildStrMap(void)
{
  ourStrMap[STATUS_NEVER_OPENED] = "never opened";
  ourStrMap[STATUS_OPEN] = "open";
  ourStrMap[STATUS_OPEN_FAILED] = "open failed";
  ourStrMap[STATUS_CLOSED_NORMALLY] = "closed";
  ourStrMap[STATUS_CLOSED_ERROR] = "closed on error";
}

AREXPORT const char * ArDeviceConnection::getStatusMessage(int messageNumber) const
{
  ArStrMap::const_iterator it;
  if ((it = ourStrMap.find(messageNumber)) != ourStrMap.end())
    return (*it).second.c_str();
  else
    return NULL;
}

AREXPORT void ArDeviceConnection::setPortName(const char *portName)
{
  if (portName != NULL)
    myDCPortName = portName;
  else
    myDCPortName = "Unknown port name";
}

AREXPORT const char *ArDeviceConnection::getPortName(void) const
{
  return myDCPortName.c_str();
}

AREXPORT void ArDeviceConnection::setPortType(const char *portType)
{
  if (portType != NULL)
    myDCPortType = portType;
  else
    myDCPortType = "Unknown port type";
}

AREXPORT const char *ArDeviceConnection::getPortType(void) const
{
  return myDCPortType.c_str();
}

AREXPORT void ArDeviceConnection::setDeviceName(const char *deviceName)
{
  if (deviceName != NULL)
    myDCDeviceName = deviceName;
  else
    myDCDeviceName = "Unknown device name";
}

AREXPORT const char *ArDeviceConnection::getDeviceName(void) const
{
  return myDCDeviceName.c_str();
}

AREXPORT void ArDeviceConnection::debugStartPacket(void)
{
  if (!ourDCDebugShouldLog)
    return;

  myDCDebugStartTime.setToNow();
  myDCDebugPacketStarted = true;
  myDCDebugBytesRead = 0;
  myDCDebugTimesRead = 0;
}

AREXPORT void ArDeviceConnection::debugBytesRead(int bytesRead)
{
  if (!ourDCDebugShouldLog || !myDCDebugPacketStarted)
    return;

  if (bytesRead > 0)
  {
    if (myDCDebugBytesRead == 0)
      myDCDebugFirstByteTime.setToNow();
    myDCDebugLastByteTime.setToNow();
    myDCDebugBytesRead += bytesRead;
  }

  myDCDebugTimesRead++;
}

AREXPORT void ArDeviceConnection::debugEndPacket(bool goodPacket, int type)
{
  if (!ourDCDebugShouldLog || !myDCDebugPacketStarted)
    return;

  if (myDCDebugBytesRead == 0)
  {
    myDCDebugFirstByteTime.setToNow();
    myDCDebugLastByteTime.setToNow();
  }
  
  if (goodPacket)
    myDCDebugNumGoodPackets++;
  else
    myDCDebugNumBadPackets++;

  long long firstSince = ourDCDebugFirstTime.mSecSinceLL(myDCDebugStartTime);

  ArLog::log(ArLog::Normal, 
	     "DevCon %s %s %s started %lld.%03lld firstByte %lld lastByte %lld bytesRead %d timesRead %d good %d numGood %lld numBad %lld type %d 0x%x",
	     myDCPortType.c_str(), myDCPortName.c_str(), myDCDeviceName.c_str(),
	     firstSince / 1000, firstSince % 1000,
	     myDCDebugStartTime.mSecSinceLL(myDCDebugFirstByteTime),
	     myDCDebugStartTime.mSecSinceLL(myDCDebugLastByteTime),
	     myDCDebugBytesRead, myDCDebugTimesRead, goodPacket,
	     myDCDebugNumGoodPackets, myDCDebugNumBadPackets, 
	     type, type);
  
  myDCDebugPacketStarted = false;
}

AREXPORT bool ArDeviceConnection::debugShouldLog(bool shouldLog)
{
  ourDCDebugShouldLog = shouldLog;
  return true;
}
