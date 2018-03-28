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
#include "ArRobotPacketSender.h"

/**
   Use setDeviceConnection() to set the device connection before use.
   @param sync1 first byte of the header this sender will send, this 
   should be left as the default in nearly all cases, or it won't work with any
   production robot. ie don't mess with it
   @param sync2 second byte of the header this sender will send, this 
   should be left as the default in nearly all cases, or it won't work with any
   production robot. ie don't mess with it
*/
AREXPORT ArRobotPacketSender::ArRobotPacketSender(unsigned char sync1,
						  unsigned char sync2) :
  myPacket(sync1, sync2)
{
  myDeviceConn = NULL;
	myTracking = false;
	myTrackingLogName.clear();
  mySendingMutex.setLogName("ArRobotPacketSender");
  myPacketSentCallback = NULL;
  myCommandMonitorCB = NULL;
}

/**
   @param deviceConnection device connection to send packets to
   @param sync1 first byte of the header this sender will send, this 
   should be left as the default in nearly all cases, or it won't work with any
   production robot. ie don't mess with it
   @param sync2 second byte of the header this sender will send, this 
   should be left as the default in nearly all cases, or it won't work with any
   production robot. ie don't mess with it
*/
AREXPORT ArRobotPacketSender::ArRobotPacketSender(
	ArDeviceConnection *deviceConnection, 
    unsigned char sync1,
	unsigned char sync2) :
  myPacket(sync1, sync2)
{
  myDeviceConn = deviceConnection;
	myTracking = false;
	myTrackingLogName.clear();
  mySendingMutex.setLogName("ArRobotPacketSender");
  myPacketSentCallback = NULL;
}

/**
   @param deviceConnection device connection to send packets to
   @param sync1 first byte of the header this sender will send, this 
   should be left as the default in nearly all cases, or it won't work with any
   production robot. ie don't mess with it
   @param sync2 second byte of the header this sender will send, this 
   should be left as the default in nearly all cases, or it won't work with any
   production robot. ie don't mess with it
   @param tracking if true write packet-tracking log messages for each packet sent.
   @param trackingLogName name (packet type) to include in packet-tracking log messages 

*/
AREXPORT ArRobotPacketSender::ArRobotPacketSender(
	ArDeviceConnection *deviceConnection, 
    unsigned char sync1,
	unsigned char sync2,
	bool tracking,
	const char *trackingLogName) :
  myPacket(sync1, sync2),
	myTracking(tracking),
	myTrackingLogName(trackingLogName)
{
  myDeviceConn = deviceConnection;
  mySendingMutex.setLogName("ArRobotPacketSender");
  myPacketSentCallback = NULL;
}

AREXPORT ArRobotPacketSender::~ArRobotPacketSender()
{

}

AREXPORT void ArRobotPacketSender::setDeviceConnection(
	ArDeviceConnection *deviceConnection)
{
  myDeviceConn = deviceConnection;
}

AREXPORT ArDeviceConnection *ArRobotPacketSender::getDeviceConnection(void)
{
  return myDeviceConn;
}

bool ArRobotPacketSender::connValid(void)
{
  return (myDeviceConn != NULL && 
	  myDeviceConn->getStatus() == ArDeviceConnection::STATUS_OPEN);
}

/**
   @param command the command number to send
   @return whether the command could be sent or not
*/
AREXPORT bool ArRobotPacketSender::com(unsigned char command)
{
  if (!connValid())
    return false;

  bool ret;
  
  mySendingMutex.lock();

  myPacket.empty();
  myPacket.setID(command);

  myPacket.finalizePacket();

  if(myTracking)
    myPacket.log();

  // the old one seems wrong...  (next line)
  //  ret = myDeviceConn->write(myPacket.getBuf(), myPacket.getLength());
  ret = (myDeviceConn->write(myPacket.getBuf(), myPacket.getLength()) >= 0);

  if (myPacketSentCallback != NULL)
    myPacketSentCallback->invoke(&myPacket);

  mySendingMutex.unlock();
  
  return ret;
}

/**
   @param command the command number to send
   @param argument the integer argument to send with the command
   @return whether the command could be sent or not
*/
AREXPORT bool ArRobotPacketSender::comInt(unsigned char command, 
					  short int argument)
{

  if (!connValid())
    return false;

  bool ret = true;

  mySendingMutex.lock();

  myPacket.empty();
  myPacket.setID(command);
  if (argument >= 0) 
  {
    myPacket.uByteToBuf(INTARG);
  }
  else 
  {
    myPacket.uByteToBuf(NINTARG);
    argument = -argument;
  }
  myPacket.uByte2ToBuf(argument);

  myPacket.finalizePacket();

  if(myTracking)
    myPacket.log();

  ret = (myDeviceConn->write(myPacket.getBuf(), myPacket.getLength()) >= 0);

  if (myPacketSentCallback != NULL)
    myPacketSentCallback->invoke(&myPacket);
  if(myCommandMonitorCB) 
    myCommandMonitorCB->invoke(command, argument);

  mySendingMutex.unlock();

  return ret;
}

/**
   @param command the command number to send
   @param high the high byte to send with the command
   @param low the low byte to send with the command
   @return whether the command could be sent or not
*/
AREXPORT bool ArRobotPacketSender::com2Bytes(unsigned char command, char high,
					     char low)
{
  return comInt(command, ((high & 0xff)<<8) + (low & 0xff));
}

/**
 * Sends a length-prefixed string command.
   @param command the command number to send
   @param argument NULL-terminated string to send with the command
   @return whether the command could be sent or not
*/
AREXPORT bool ArRobotPacketSender::comStr(unsigned char command, 
					  const char *argument)
{
  size_t size;
  if (!connValid())
    return false;
  size = strlen(argument);
  if (size > 199) // 200 - 1 byte for length
    return false;

  bool ret = true;

  mySendingMutex.lock();

  myPacket.empty();
  
  myPacket.setID(command);
  myPacket.uByteToBuf(STRARG);
  myPacket.uByteToBuf((ArTypes::UByte)size);
  myPacket.strToBuf(argument);
  
  myPacket.finalizePacket();

  if(myTracking)
	  myPacket.log();

  ret = (myDeviceConn->write(myPacket.getBuf(), myPacket.getLength()) >= 0);

  if (myPacketSentCallback != NULL)
    myPacketSentCallback->invoke(&myPacket);

  mySendingMutex.unlock();

  return ret;
}

/**
 * Sends a packet containing the given command, and a length-prefixed string 
 * containing the specified number of bytes copied from the given source string.
   @param command the command number to send
   @param str the character array containing data to send with the command
   @param size number of bytes from the array to send; prefix the string with a byte containing this value as well. this size must be less than the maximum packet size of 200
   @return whether the command could be sent or not
*/
AREXPORT bool ArRobotPacketSender::comStrN(unsigned char command, 
					   const char *str, int size)
{
  if (!connValid())
    return false;

  if(size > 199) return false;   // 200 - 1 byte for length

  bool ret = true;

  mySendingMutex.lock();

  myPacket.empty();
  
  myPacket.setID(command);
  myPacket.uByteToBuf(STRARG);

  myPacket.uByteToBuf(size);
  myPacket.strNToBuf(str, size);
  
  myPacket.finalizePacket();

  if(myTracking)
	  myPacket.log();

  ret = (myDeviceConn->write(myPacket.getBuf(), myPacket.getLength()) >= 0);

  if (myPacketSentCallback != NULL)
    myPacketSentCallback->invoke(&myPacket);

  mySendingMutex.unlock();

  return false;
  
}


/**
 * Sends an ArRobotPacket
   @param packet ArRobotPacket
   @return whether the command could be sent or not
*/
AREXPORT bool ArRobotPacketSender::sendPacket(ArRobotPacket *packet)
{
  if (!connValid())
    return false;

  //if(size > 199) return false;   // 200 - 1 byte for length

  bool ret = true;

  mySendingMutex.lock();
  
  packet->finalizePacket();

	// if tracking is on - log packet - also make sure
	// buffer length is in range
	if ((myTracking) && (packet->getLength() < 10000)) {

		unsigned char *buf = (unsigned char *) packet->getBuf();
		
		char obuf[10000];
		obuf[0] = '\0';
		int j = 0;
		for (int i = 0; i < packet->getLength(); i++) {
			sprintf (&obuf[j], "_%02x", buf[i]);
			j= j+3;
		}
		ArLog::log (ArLog::Normal,
				            "Send Packet: %s packet = %s", 
										myTrackingLogName.c_str(), obuf);


	}

	//packet->log();
  ret = (myDeviceConn->write(packet->getBuf(), packet->getLength()) >= 0);

  if (myPacketSentCallback != NULL)
    myPacketSentCallback->invoke(packet);

  mySendingMutex.unlock();

  return ret;
  
}

AREXPORT bool ArRobotPacketSender::comDataN(unsigned char command, const char* data, int size)
{
  if(!connValid()) return false;
  if(size > 200) return false;

  bool ret = true;

  mySendingMutex.lock();

  myPacket.empty();
  myPacket.setID(command);
  myPacket.uByteToBuf(STRARG);
  myPacket.strNToBuf(data, size);
  myPacket.finalizePacket();

  if(myTracking)
    myPacket.log();

  ret = (myDeviceConn->write(myPacket.getBuf(), myPacket.getLength()) >= 0);

  if (myPacketSentCallback != NULL)
    myPacketSentCallback->invoke(&myPacket);

  mySendingMutex.unlock();

  return ret;
}

AREXPORT void ArRobotPacketSender::setPacketSentCallback(
	ArFunctor1<ArRobotPacket *> *functor)
{
  myPacketSentCallback = functor;
}
