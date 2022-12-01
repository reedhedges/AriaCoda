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
#include "Aria/ArLMS2xxPacket.h"
#include "stdio.h"

AREXPORT ArLMS2xxPacket::ArLMS2xxPacket(unsigned char sendingAddress) :
  ArBasePacket(2048, 4)
{
  mySendingAddress = sendingAddress;
}

/* AREXPORT ArLMS2xxPacket::~ArLMS2xxPacket()
{
} */

/** 
   This sets the address for use in sending packets, the address
   is saved, then when a packet is finalizePacketd for sending, the address
   is put into the appropriate spot in the packet.
   @param address the address of the laser to be addressed
*/
AREXPORT void ArLMS2xxPacket::setSendingAddress(unsigned char address)
{
  mySendingAddress = address;
}

/** 
   This gets the address for use in sending packets, the address is
   what has been saved, then when a packet is finalizePacketd for sending,
   the address is put into the appropriate spot in the packet. 
   @return the address of the laser to be addressed 
*/
AREXPORT unsigned char ArLMS2xxPacket::getSendingAddress()
{
  return mySendingAddress;
}

/**
   This gets the address that this packet was received from.  Note that 
   this is only valid if this packet was received from a laser, if you want
   to know where a packet was addressed to use getSendingAdress instead.
   @return the address a packet was received from
*/
AREXPORT unsigned char ArLMS2xxPacket::getReceivedAddress()
{
  // toss it into the second byte of the packet
  myReadLength = 1;
  const unsigned char address = bufToUByte();
  myLength = myReadLength;
  return address;
}

AREXPORT uint8_t ArLMS2xxPacket::getID()
{
 if (myLength >= 5)
    return (uint8_t) myBuf[4];
  else
    return 0;
}

AREXPORT void ArLMS2xxPacket::resetRead()
{
  myReadLength = myHeaderLength + 1;
}

AREXPORT void ArLMS2xxPacket::finalizePacket()
{
  const uint16_t len = myLength;

  // put in the start of the packet
  myLength = 0;
  // toss in the header 
  uByteToBuf(0x02);
  // now the laser we want to talk to
  uByteToBuf(mySendingAddress);
  // dump in the length
  uByte2ToBuf(len - myHeaderLength);
  myLength = len;

  // that lovely CRC
  const int16_t chkSum = calcCRC();
  byteToBuf((char)(chkSum & 0xff) );
  byteToBuf((char)((chkSum >> 8) & 0xff));

  //printf("Sending ");
  //log();
}

/**
   Copies the given packets buffer into the buffer of this packet, also
   sets this length and readlength to what the given packet has
   @param packet the packet to duplicate
*/
AREXPORT void ArLMS2xxPacket::duplicatePacket(ArLMS2xxPacket *packet)
{
  myLength = packet->getLength();
  myReadLength = packet->getReadLength();
  myTimeReceived = packet->getTimeReceived();
  mySendingAddress = packet->getSendingAddress();
  memcpy(myBuf, packet->getBuf(), myLength);
  
}

AREXPORT int16_t ArLMS2xxPacket::calcCRC()
{
  unsigned short uCrc16;
  unsigned char abData[2];
  unsigned int uLen = myLength;
  unsigned char * commData = (unsigned char *)myBuf;

  uCrc16 = 0;
  abData[0] = 0;
  while (uLen--)
  {
    abData[1] = abData[0];
    abData[0] = *commData++;
    if (uCrc16 & 0x8000)
    {
      uCrc16 = (unsigned short)((uCrc16 & 0x7fff) << 1);
      uCrc16 ^= 0x8005;
    }
    else
    {
      //uCrc16 <<= 1;
      uCrc16 = (unsigned short)(uCrc16 << 1);
    }
    uCrc16 ^= (unsigned short)(abData[0] | (abData[1] << 8));
  }
  return (int16_t)uCrc16;
}

AREXPORT bool ArLMS2xxPacket::verifyCRC() 
{
  const uint16_t readLen = myReadLength;
  const uint16_t len = myLength;

  myReadLength = myLength - 2;
  
  if (myReadLength < myHeaderLength)
    return false;

  const unsigned char c1 = (unsigned char) bufToByte();
  const unsigned char c2 = (unsigned char) bufToByte();
  myReadLength = readLen;
  const int16_t chksum = (int16_t) ( (c1 & 0xff) | (c2 << 8) );

  myLength = myLength - 2;
  if (chksum == calcCRC()) {
    myLength = len;
    return true;
  } else {
    myLength = len;
    return false;
  }
  
}

AREXPORT ArTime ArLMS2xxPacket::getTimeReceived()
{
  return myTimeReceived;
}

AREXPORT void ArLMS2xxPacket::setTimeReceived(ArTime timeReceived)
{
  myTimeReceived = timeReceived;
}
