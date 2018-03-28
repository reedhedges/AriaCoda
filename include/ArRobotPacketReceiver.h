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
#ifndef ARROBOTPACKETRECEIVER_H
#define ARROBOTPACKETRECEIVER_H

#include "ariaTypedefs.h"
#include "ArRobotPacket.h"


class ArDeviceConnection;

/// Given a device connection it receives packets from the robot through it
class ArRobotPacketReceiver
{
public:
  /// Constructor without an already assigned device connection
  AREXPORT ArRobotPacketReceiver(bool allocatePackets = false,
				 unsigned char sync1 = 0xfa, 
				 unsigned char sync2 = 0xfb);
  /// Constructor with assignment of a device connection
  AREXPORT ArRobotPacketReceiver(ArDeviceConnection *deviceConnection, 
				 bool allocatePackets = false,
				 unsigned char sync1 = 0xfa, 
				 unsigned char sync2 = 0xfb);
  /// Constructor with assignment of a device connection and tracking
  AREXPORT ArRobotPacketReceiver(ArDeviceConnection *deviceConnection, 
				 bool allocatePackets,
				 unsigned char sync1, 
				 unsigned char sync2,
					bool tracking,
					const char *trackingLogName);
  /// Destructor
  AREXPORT virtual ~ArRobotPacketReceiver();
  
  /// Receives a packet from the robot if there is one available
  AREXPORT ArRobotPacket *receivePacket(unsigned int msWait = 0);

  /// Sets the device this instance receives packets from
  AREXPORT void setDeviceConnection(ArDeviceConnection *deviceConnection);
  /// Gets the device this instance receives packets from
  AREXPORT ArDeviceConnection *getDeviceConnection(void);
  
  /// Gets whether or not the receiver is allocating packets
  AREXPORT bool isAllocatingPackets(void) { return myAllocatePackets; }
  /// Sets whether or not the receiver is allocating packets
  AREXPORT void setAllocatingPackets(bool allocatePackets) 
    { myAllocatePackets = allocatePackets; }

#ifdef DEBUG_SPARCS_TESTING
  AREXPORT void setSync1(unsigned char s1) { mySync1 = s1; }
  AREXPORT void setSync2(unsigned char s2) { mySync2 = s2; }
#endif

	void setTracking(bool tracking)
  {
    myTracking = tracking;
  }

	void setTrackingLogName(const char *trackingLogName)
  {
    myTrackingLogName = trackingLogName;
  }

  /// Sets the callback that gets called with the finalized version of
  /// every packet set... this is ONLY for very internal very
  /// specialized use
  AREXPORT void setPacketReceivedCallback(ArFunctor1<ArRobotPacket *> *functor);
protected:
  ArDeviceConnection *myDeviceConn;
	bool myTracking;
	std::string myTrackingLogName;

  bool myAllocatePackets;
  ArRobotPacket myPacket;
  enum { STATE_SYNC1, STATE_SYNC2, STATE_ACQUIRE_DATA };
  unsigned char mySync1;
  unsigned char mySync2;

  ArFunctor1<ArRobotPacket *> *myPacketReceivedCallback;
};

#endif // ARROBOTPACKETRECEIVER_H
