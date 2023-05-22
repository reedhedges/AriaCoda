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


*/
#ifndef ARLMS2XXPACKET_H
#define ARLMS2XXPACKET_H

#ifndef ARIA_WRAPPER

#include "Aria/ariaTypedefs.h"
#include "Aria/ArBasePacket.h"
#include "Aria/ariaUtil.h"

/// Represents the packets sent to the LMS2xx as well as those received from it
/**
   This class re-implements some of the buf operations since the robot is 
   little endian. 
   
   You can just look at the documentation for the ArBasePacket except
   for these functions here, setAddress, getAddress, verifyCheckSum,
   print, getID, and calcCheckSum.  

   @internal
*/

class ArLMS2xxPacket final : public ArBasePacket
{
public:
  /// Constructor
  AREXPORT ArLMS2xxPacket(unsigned char sendingAddress = 0);
  /* /// Destructor
  AREXPORT virtual ~ArLMS2xxPacket();
 */
  /// Sets the address to send this packet to (only use for sending)
  AREXPORT void setSendingAddress(unsigned char address);

  /// Sets the address to send this packet to (only use for sending)
  AREXPORT unsigned char getSendingAddress();

  /// Gets the address this packet was sent from (only use for receiving)
  AREXPORT unsigned char getReceivedAddress();
  
  /// returns true if the crc matches what it should be
  AREXPORT bool verifyCRC();
  
  /// returns the ID of the packet (first byte of data)
  AREXPORT uint8_t getID();

  /// returns the crc, probably used only internally
  AREXPORT int16_t calcCRC();
  
  // only call finalizePacket before a send
  AREXPORT virtual void finalizePacket() override;
  AREXPORT virtual void resetRead() override;
  
  /// Gets the time the packet was received at
  AREXPORT ArTime getTimeReceived();
  /// Sets the time the packet was received at
  AREXPORT void setTimeReceived(ArTime timeReceived);

  /// Duplicates the packet
  AREXPORT void duplicatePacket(ArLMS2xxPacket *packet);

  virtual void duplicatePacket(ArBasePacket *packet) override {
    duplicatePacket(dynamic_cast<ArLMS2xxPacket*>(packet));
  }

protected:
  ArTime myTimeReceived;
  unsigned char mySendingAddress;
};

typedef ArLMS2xxPacket ArSickPacket;

#endif // not ARIA_WRAPPER
#endif // ARSICKPACKET_H
