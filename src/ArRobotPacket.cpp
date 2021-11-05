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
#include "Aria/ArExport.h"
#include "Aria/ariaOSDef.h"
#include "Aria/ArRobotPacket.h"
#include "stdio.h"

/**
   @param sync1 first byte of the header of this packet, this should be left as
   the default in nearly all cases, ie don't mess with it
   @param sync2 second byte of the header of this packet, this should be left
   as the default in nearly all cases, ie don't mess with it
 */
AREXPORT ArRobotPacket::ArRobotPacket(unsigned char sync1,
				      unsigned char sync2) :
    ArBasePacket(265, 4, NULL, 2)
{
  mySync1 = sync1;
  mySync2 = sync2;
}


AREXPORT ArRobotPacket::ArRobotPacket(const ArRobotPacket& other) : ArRobotPacket(other.mySync1, other.mySync2)
{
  *this = other; // see operator= below
  // XXX TODO make sure this is ok
}

AREXPORT ArRobotPacket &ArRobotPacket::operator=(const ArRobotPacket &other)
{
  // TODO much of this should perhaps be in ArBasePacket? 

  if (this != &other) {

    myHeaderLength = other.myHeaderLength;
    myFooterLength = other.myFooterLength;
    myReadLength   = other.myReadLength;
    myLength = other.myLength;
    mySync1 = other.mySync1;
    mySync2 = other.mySync2;
    myTimeReceived = other.myTimeReceived;

    if (myMaxLength != other.myMaxLength) {
      if (myOwnMyBuf && myBuf != NULL)
        delete [] myBuf;
      myOwnMyBuf = true;
      myBuf = NULL;
      if (other.myMaxLength > 0) {
        myBuf = new char[other.myMaxLength];
      }
      myMaxLength = other.myMaxLength;
    }

    if ((myBuf != NULL) && (other.myBuf != NULL)) {
      memcpy(myBuf, other.myBuf, myMaxLength);
    }
   
    myIsValid = other.myIsValid;
  }
  return *this;
}

AREXPORT uint8_t ArRobotPacket::getID()
{
  if (myLength >= 4)
    return (uint8_t) myBuf[3];
  else
    return 0;
}

AREXPORT void ArRobotPacket::setID(uint8_t id)
{
  myBuf[3] = (char) id;
}

AREXPORT void ArRobotPacket::finalizePacket()
{
  const uint16_t len = myLength;

  myLength = 0;
  uByteToBuf(mySync1);
  uByteToBuf(mySync2);
  uByteToBuf((uint8_t)(len - getHeaderLength() + 3));
  myLength = len;

  const int chkSum = calcCheckSum();
  byteToBuf((int8_t)((chkSum >> 8) & 0xff));
  byteToBuf((int8_t)(chkSum & 0xff));
  /* Put this in if you want to see the packets being outputted 
     printf("Output(%3d) ", getID());
     printHex();
  */
  // or put this in if you just want to see the type
  //printf("Output %d\n", getID());
}

AREXPORT int16_t ArRobotPacket::calcCheckSum()
{
  int16_t c = 0;
  int i = 3;
  int n = (int) myBuf[2] - 2;
  while (n > 1) {
    c += (int16_t) (((unsigned char)myBuf[i]<<8) | (unsigned char)myBuf[i+1]);
    c = (int16_t) (c & 0xffff);
    n -= 2;
    i += 2;
  }
  if (n > 0) 
    c = c ^ (int16_t)((unsigned char) myBuf[i]);
  return c;
}

AREXPORT bool ArRobotPacket::verifyCheckSum() 
{
  if (myLength - 2 < myHeaderLength)
    return false;

  const unsigned char c2 = (unsigned char) myBuf[myLength - 2];
  const unsigned char c1 = (unsigned char) myBuf[myLength - 1];
  const int16_t chksum = (int16_t) ( (c1 & 0xff) | (c2 << 8) );

  if (chksum == calcCheckSum()) {
    return true;
  } else {
    return false;
  }
  
}

AREXPORT ArTime ArRobotPacket::getTimeReceived()
{
  return myTimeReceived;
}

AREXPORT void ArRobotPacket::setTimeReceived(ArTime timeReceived)
{
  myTimeReceived = timeReceived;
}

AREXPORT void ArRobotPacket::log()
{
  ArLog::log(ArLog::Normal, "Robot Packet: length = %i, type = 0x%x %s: ", myLength, getID(), getName());
  for (int i = 0; i < myLength; ++i)
    ArLog::log(ArLog::Terse, "  [%03i] % 5d\t0x%x\t%c\t%s", i,
        (unsigned char) myBuf[i],
        (unsigned char) myBuf[i],
        (myBuf[i] >= ' ' && myBuf[i] <= '~') ? (unsigned char) myBuf[i] : ' ',
        i == 0 ? "[header0]" :
          i == 1 ? "[header1]" :
            i == 2 ? "[packet data length]" :
              i == 3 ? "[packet id]" :
                i == (myLength - 2) ? "[first checksum byte]" :
                  i == (myLength - 1) ? "[second checksum byte]" :
                    ""
    );
  ArLog::log(ArLog::Terse, "\n");
}

