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
#ifndef ARLMS2XXPACKET_H
#define ARLMS2XXPACKET_H

#include "ariaTypedefs.h"
#include "ArBasePacket.h"
#include "ariaUtil.h"

/// Represents the packets sent to the LMS2xx as well as those received from it
/**
   This class reimplements some of the buf operations since the robot is 
   little endian. 
   
   You can just look at the documentation for the ArBasePacket except
   for these functions here, setAddress, getAddress, verifyCheckSum,
   print, getID, and calcCheckSum.  
*/

class ArLMS2xxPacket: public ArBasePacket
{
public:
  /// Constructor
  AREXPORT ArLMS2xxPacket(unsigned char sendingAddress = 0);
  /// Destructor
  AREXPORT virtual ~ArLMS2xxPacket();

  /// Sets the address to send this packet to (only use for sending)
  AREXPORT void setSendingAddress(unsigned char address);

  /// Sets the address to send this packet to (only use for sending)
  AREXPORT unsigned char getSendingAddress(void);

  /// Gets the address this packet was sent from (only use for receiving)
  AREXPORT unsigned char getReceivedAddress(void);
  
  /// returns true if the crc matches what it should be
  AREXPORT bool verifyCRC(void);
  
  /// returns the ID of the packet (first byte of data)
  AREXPORT ArTypes::UByte getID(void);

  /// returns the crc, probably used only internally
  AREXPORT ArTypes::Byte2 calcCRC(void);
  
  // only call finalizePacket before a send
  AREXPORT virtual void finalizePacket(void);
  AREXPORT virtual void resetRead(void);
  
  /// Gets the time the packet was received at
  AREXPORT ArTime getTimeReceived(void);
  /// Sets the time the packet was received at
  AREXPORT void setTimeReceived(ArTime timeReceived);

  /// Duplicates the packet
  AREXPORT virtual void duplicatePacket(ArLMS2xxPacket *packet);
protected:
  ArTime myTimeReceived;
  unsigned char mySendingAddress;
};

typedef ArLMS2xxPacket ArSickPacket;

#endif // ARSICKPACKET_H
