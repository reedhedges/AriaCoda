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
#ifndef ARROBOTPACKET_H
#define ARROBOTPACKET_H

#ifndef ARIA_WRAPPER

#include "Aria/ariaTypedefs.h"
#include "Aria/ArBasePacket.h"
#include "Aria/ariaUtil.h"
#include "Aria/ariaInternal.h"

/// Represents the packets sent to the robot as well as those received from it
/**
   This class reimplements some of the buf operations since the robot is 
   opposite endian from intel.  Also has the getID for convenience.  
   
   You can just look at the documentation for the ArBasePacket except for
   the 4 new functions here, verifyCheckSum, getID, print, and calcCheckSum.
 
   @internal
 */
class ArRobotPacket final : public ArBasePacket
{
public:
  /// Constructor
  AREXPORT ArRobotPacket(unsigned char sync1 = 0xfa, 
			 unsigned char sync2 = 0xfb);
  /// Destructor
  AREXPORT virtual ~ArRobotPacket() = default;

  AREXPORT ArRobotPacket(const ArRobotPacket &other);

  AREXPORT ArRobotPacket &operator=(const ArRobotPacket &other);


  // XXX TODO should also include move constructor and  move assignment operator?

  /// returns true if the checksum matches what it should be
  AREXPORT bool verifyCheckSum();

  /// returns the ID of the packet 
  AREXPORT uint8_t getID();

  /// Get string containing packet type name, if known
  const char *getName() { return Aria::getPacketTypeName(getID()); }

  /// Sets the ID of the packet 
  AREXPORT void setID(uint8_t id);

  /// returns the checksum, probably used only internally
  AREXPORT int16_t calcCheckSum();
  
  // only call finalizePacket before a send
  AREXPORT virtual void finalizePacket() override;
  
  /// Gets the time the packet was received at
  AREXPORT ArTime getTimeReceived();
  /// Sets the time the packet was received at
  AREXPORT void setTimeReceived(ArTime timeReceived);

  AREXPORT virtual void log() override;

protected:
  unsigned char mySync1;
  unsigned char mySync2;
  ArTime myTimeReceived;
};

#endif // not ARIA_WRAPPER
#endif // ARROBOTPACKET_H
