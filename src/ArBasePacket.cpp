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
#include "ArBasePacket.h"
#include "ArLog.h"
#include "ariaUtil.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
@param bufferSize size of the data buffer. Note default is 0, so packet will have no data capacity unless set.
@param headerLength length of the header
@param buf buffer packet uses, if NULL, instance will allocate memory
@param footerLength length of the footer following the data
*/
AREXPORT ArBasePacket::ArBasePacket(ArTypes::UByte2 bufferSize, 
                                    ArTypes::UByte2 headerLength,
                                    char * buf,
                                    ArTypes::UByte2 footerLength) 
{
  if (buf == NULL && bufferSize > 0) 
  {
    myOwnMyBuf = true;
    myBuf = new char[bufferSize];
    // memset(myBuf, 0, bufferSize);
  } 
  else 
  {
    myOwnMyBuf = false;
    myBuf = buf;
  }
  myHeaderLength = headerLength;
  myFooterLength = footerLength;
  myReadLength = myHeaderLength;
  myMaxLength = bufferSize;
  myLength = myHeaderLength;
  myIsValid = true;
}


AREXPORT ArBasePacket::ArBasePacket(const ArBasePacket &other) :
  myHeaderLength(other.myHeaderLength),
  myFooterLength(other.myFooterLength),
  myMaxLength(other.myLength),
  myReadLength(other.myReadLength),
  myOwnMyBuf(true),
  myBuf((other.myLength > 0) ? new char[other.myLength] : NULL),
  myLength(other.myLength),
  myIsValid(other.myIsValid)
{
  if ((myBuf != NULL) && (other.myBuf != NULL)) {
    memcpy(myBuf, other.myBuf, myLength);
  }
}

AREXPORT ArBasePacket &ArBasePacket::operator=(const ArBasePacket &other)
{
  if (this != &other) {

    myHeaderLength = other.myHeaderLength;
    myFooterLength = other.myFooterLength;
    myReadLength   = other.myReadLength;

    if (myLength != other.myLength) {
      if (myOwnMyBuf && myBuf != NULL)
	delete [] myBuf;
      myOwnMyBuf = true;
      myBuf = NULL;
      if (other.myLength > 0) {
        myBuf = new char[other.myLength];
      }
      myLength = other.myLength;
      myMaxLength = other.myLength;
    }

    if ((myBuf != NULL) && (other.myBuf != NULL)) {
      memcpy(myBuf, other.myBuf, myLength);
    }
   
    myIsValid = other.myIsValid;
  }
  return *this;
}



AREXPORT ArBasePacket::~ArBasePacket()
{
  if (myOwnMyBuf && myBuf != NULL)
    delete[] myBuf;
}


AREXPORT void ArBasePacket::setBuf(char *buf, ArTypes::UByte2 bufferSize)
{
  if (myOwnMyBuf) 
  {
    delete[] myBuf;
    myOwnMyBuf = false;
  } 
  myBuf = buf;
  myMaxLength = bufferSize;
}

AREXPORT void ArBasePacket::setMaxLength(ArTypes::UByte2 bufferSize)
{
  if (myMaxLength >= bufferSize)
    return;
  if (myOwnMyBuf) 
  {
    delete[] myBuf;
    myOwnMyBuf = false;
  } 
  myBuf = new char[bufferSize];
  // memset(myBuf, 0, bufferSize);

  myMaxLength = bufferSize;
  myOwnMyBuf = true;
}

AREXPORT bool ArBasePacket::setLength(ArTypes::UByte2 length)
{
  if (myOwnMyBuf && length > myMaxLength)
    return false;

  myLength = length;
  return true;
}

AREXPORT void ArBasePacket::setReadLength(ArTypes::UByte2 readLength)
{
  myReadLength = readLength;
}

AREXPORT bool ArBasePacket::setHeaderLength(ArTypes::UByte2 length)
{
  if (myOwnMyBuf && length > myMaxLength)
    return false;

  myHeaderLength = length;
  return true;
}

/** 
Sets the length read back to the header length so the packet can be
reread using the other methods
*/

AREXPORT void ArBasePacket::resetRead(void)
{
  myReadLength = myHeaderLength;
  resetValid();
}

ArTypes::UByte2 ArBasePacket::getDataLength(void) const { 
 
  // KMC 12/20/13 Do not allow negative values to be returned.  (They are basically 
  // converted to an erroneous positive value by the UByte2.)
  int len = myLength - myHeaderLength - myFooterLength; 
  if (len >= 0) {
    return len;
  }
  else {
/****
    ArLog::log(ArLog::Normal,
               "ArBasePacket::getDataLength() negative myLength = %i, myHeaderLength = %i, myFooterLength = %i",
               myLength,
               myHeaderLength,
               myFooterLength);
***/
    return 0;
  }
}

/**
Sets the packet length back to be the packets header length again
*/

AREXPORT void ArBasePacket::empty(void)
{
  myLength = myHeaderLength;
  resetValid();
}

AREXPORT bool ArBasePacket::isNextGood(int bytes)
{
  if (bytes <= 0)
    return false;

  // make sure it comes in before the header
  if (myReadLength + bytes <= myLength - myFooterLength)
    return true;

  myIsValid = false;

  return false;
}


AREXPORT bool ArBasePacket::hasWriteCapacity(int bytes)
{
  if (bytes < 0) {
    ArLog::log(ArLog::Normal, "ArBasePacket::hasWriteCapacity(%d) cannot write negative amount",
               bytes);
    return false;
  }

  // Make sure there's enough room in the packet 
  if ((myLength + bytes) <= myMaxLength) {
     return true;
  }

  myIsValid = false;

  return false;

} // end method hasWriteCapacity


/**
 * A packet is considered "invalid" if an attempt is made to write too much
 * data into the packet, or to read too much data from the packet.  Calls to
 * empty() and resetRead() will restore the valid state.
**/
AREXPORT bool ArBasePacket::isValid(void)
{
  return myIsValid;

} // end method isValid

/**
 * Resets the packet to the "valid" state.  This method should generally
 * only be called externally when the application has taken some recovery
 * action.  For example, if an attempt to write a long string to the packet
 * fails (and isValid() returns false), then a smaller string may be written
 * instead.
**/
AREXPORT void ArBasePacket::resetValid()
{
  myIsValid = true;
}

AREXPORT const char *ArBasePacket::getBuf(void) const
{
  return myBuf;
}

AREXPORT char *ArBasePacket::getBuf(void) 
{
  return myBuf;
}

AREXPORT void ArBasePacket::byteToBuf(ArTypes::Byte val)
{
  if (!hasWriteCapacity(1)) {
    return;
  }

  memcpy(myBuf+myLength, &val, 1);
  myLength += 1;
}

AREXPORT void ArBasePacket::byte2ToBuf(ArTypes::Byte2 val)
{
  if (!hasWriteCapacity(2)) {
    return;
  }

  unsigned char c;
  c = (val >> 8) & 0xff;
  memcpy(myBuf+myLength+1, &c, 1);
  c = val & 0xff;
  memcpy(myBuf+myLength, &c, 1);
  myLength += 2;
}

AREXPORT void ArBasePacket::byte4ToBuf(ArTypes::Byte4 val)
{
  if (!hasWriteCapacity(4)) {
    return;
  }

  unsigned char c;
  c = (val >> 24) & 0xff;
  memcpy(myBuf+myLength+3, &c, 1);
  c = (val >> 16) & 0xff;
  memcpy(myBuf+myLength+2, &c, 1);
  c = (val >> 8) & 0xff;
  memcpy(myBuf+myLength+1, &c, 1);
  c = val & 0xff;
  memcpy(myBuf+myLength, &c, 1);
  myLength += 4;
}

AREXPORT void ArBasePacket::byte8ToBuf(ArTypes::Byte8 val)
{
  if (!hasWriteCapacity(8)) {
    return;
  }

  unsigned char c;
  c = (val >> 56) & 0xff;
  memcpy(myBuf+myLength+7, &c, 1);
  c = (val >> 48) & 0xff;
  memcpy(myBuf+myLength+6, &c, 1);
  c = (val >> 40) & 0xff;
  memcpy(myBuf+myLength+5, &c, 1);
  c = (val >> 32) & 0xff;
  memcpy(myBuf+myLength+4, &c, 1);

  c = (val >> 24) & 0xff;
  memcpy(myBuf+myLength+3, &c, 1);
  c = (val >> 16) & 0xff;
  memcpy(myBuf+myLength+2, &c, 1);
  c = (val >> 8) & 0xff;
  memcpy(myBuf+myLength+1, &c, 1);
  c = val & 0xff;
  memcpy(myBuf+myLength, &c, 1);
  myLength += 8;

}

AREXPORT void ArBasePacket::uByteToBuf(ArTypes::UByte val)
{
  if (!hasWriteCapacity(1)) {
    return;
  }
  memcpy(myBuf+myLength, &val, 1);
  myLength += 1;
}

AREXPORT void ArBasePacket::uByte2ToBuf(ArTypes::UByte2 val)
{
  if (!hasWriteCapacity(2)) {
    return;
  }
  // Note that MSB is placed one byte after the LSB in the end of the buffer:
  unsigned char c;
  c = (val >> 8) & 0xff;
  memcpy(myBuf+myLength+1, &c, 1);
  c = val & 0xff;
  memcpy(myBuf+myLength, &c, 1);
  myLength += 2;
}

AREXPORT void ArBasePacket::uByte4ToBuf(ArTypes::UByte4 val)
{
  if (!hasWriteCapacity(4)) {
    return;
  }
  
  /*
  MPL 2013_10_23 this doesn't match anything else with regards to how
  it's happening, and while it didn't matter when we're just going
  from x86 to x86 it may matter for others... if it causes problems
  just put back the old code

  memcpy(myBuf+myLength, &val, 4);
  myLength += 4;
  */

  unsigned char c;
  c = (val >> 24) & 0xff;
  memcpy(myBuf+myLength+3, &c, 1);
  c = (val >> 16) & 0xff;
  memcpy(myBuf+myLength+2, &c, 1);
  c = (val >> 8) & 0xff;
  memcpy(myBuf+myLength+1, &c, 1);
  c = val & 0xff;
  memcpy(myBuf+myLength, &c, 1);
  myLength += 4;
}

AREXPORT void ArBasePacket::uByte8ToBuf(ArTypes::UByte8 val)
{
  if (!hasWriteCapacity(8)) {
    return;
  }
  /*
    MPL 2013_10_23 this was how would have matched the old uByte4ToBuf
    but since that didn't match anything else I changed it

    memcpy(myBuf+myLength, &val, 8);
    myLength += 8;
  */

  unsigned char c;
  c = (val >> 56) & 0xff;
  memcpy(myBuf+myLength+7, &c, 1);
  c = (val >> 48) & 0xff;
  memcpy(myBuf+myLength+6, &c, 1);
  c = (val >> 40) & 0xff;
  memcpy(myBuf+myLength+5, &c, 1);
  c = (val >> 32) & 0xff;
  memcpy(myBuf+myLength+4, &c, 1);

  c = (val >> 24) & 0xff;
  memcpy(myBuf+myLength+3, &c, 1);
  c = (val >> 16) & 0xff;
  memcpy(myBuf+myLength+2, &c, 1);
  c = (val >> 8) & 0xff;
  memcpy(myBuf+myLength+1, &c, 1);
  c = val & 0xff;
  memcpy(myBuf+myLength, &c, 1);
  myLength += 8;
}

/**
@param str string to copy into buffer
*/
AREXPORT void ArBasePacket::strToBuf(const char *str)
{
  if (str == NULL) {
    str = "";
  }
  ArTypes::UByte2 tempLen = strlen(str) + 1;

  if (!hasWriteCapacity(tempLen)) {
    return;
  }

  memcpy(myBuf+myLength, str, tempLen);
  myLength += tempLen;
}

/**
 * This method performs no bounds checking on the given length and
 * the contents of the string.  For string operations, strNToBufPadded()
 * is preferred.  For raw data operations, dataToBuf() is preferred.
@param str character array to copy into the packet buffer
@param length how many characters to copy from str into the packet buffer
*/
AREXPORT void ArBasePacket::strNToBuf(const char *str, int length)
{
  // Do not perform bounds checking because it breaks existing code.

  //byte4ToBuf(length);
  memcpy(myBuf+myLength, str, length);
  myLength+=length;

}


/**
If string ends before length it pads the string with NUL ('\\0') characters.
@param str character array to copy into buffer
@param length how many bytes to copy from the str into packet
*/
AREXPORT void ArBasePacket::strToBufPadded(const char *str, int length)
{
  if (str == NULL) {
    str = "";
  }
  ArTypes::UByte2 tempLen = strlen(str);

  if (!hasWriteCapacity(length)) {
    return;
  }

  if (tempLen >= length) {
    memcpy(myBuf + myLength, str, length);
    myLength += length;
  }
  else // string is smaller than given length
  {
    memcpy(myBuf + myLength, str, tempLen);
    myLength += tempLen;
    memset(myBuf + myLength, 0, length - tempLen);
    myLength += length - tempLen;
  }
}


/**
@param data chacter array to copy into buffer
@param length how many bytes to copy from data into packet
*/
AREXPORT void ArBasePacket::dataToBuf(const char *data, int length)
{
  if (data == NULL) {
    ArLog::log(ArLog::Normal, "ArBasePacket::dataToBuf(NULL, %d) cannot add from null address",
               length);
    return;
  }

  if (!hasWriteCapacity(length)) {
    return;
  }

  memcpy(myBuf+myLength, data, length);
  myLength+=length;

}

/**
   This was added to get around having to cast data you put in, since the data shouldn't really matter if its signed or unsigned.
@param data chacter array to copy into buffer
@param length how many bytes to copy from data into packet
*/
AREXPORT void ArBasePacket::dataToBuf(const unsigned char *data, int length)
{
  if (data == NULL) {
    ArLog::log(ArLog::Normal, "ArBasePacket::dataToBuf(NULL, %d) cannot add from null address",
               length);
    return;
  }

  if (!hasWriteCapacity(length)) {
    return;
  }

  memcpy(myBuf+myLength, data, length);
  myLength+=length;

}


AREXPORT ArTypes::Byte ArBasePacket::bufToByte(void)
{
  ArTypes::Byte ret=0;

  if (isNextGood(1))
  {
    memcpy(&ret, myBuf+myReadLength, 1);
    myReadLength+=1;
  }

  return(ret);
}

AREXPORT ArTypes::Byte2 ArBasePacket::bufToByte2(void)
{
  ArTypes::Byte2 ret=0;
  unsigned char c1, c2;

  if (isNextGood(2))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    ret = (c1 & 0xff) | (c2 << 8);
    myReadLength+=2;
  }

  return ret;
}

AREXPORT ArTypes::Byte4 ArBasePacket::bufToByte4(void)
{
  ArTypes::Byte4 ret=0;
  unsigned char c1, c2, c3, c4;

  if (isNextGood(4))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    memcpy(&c3, myBuf+myReadLength+2, 1);
    memcpy(&c4, myBuf+myReadLength+3, 1);
    ret = (c1 & 0xff) | (c2 << 8) | (c3 << 16) | (c4 << 24);
    myReadLength+=4;
  }

  return ret;
}

AREXPORT ArTypes::Byte8 ArBasePacket::bufToByte8(void)
{
  ArTypes::Byte8 ret=0;
  unsigned char c1, c2, c3, c4, c5, c6, c7, c8;

  if (isNextGood(8))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    memcpy(&c3, myBuf+myReadLength+2, 1);
    memcpy(&c4, myBuf+myReadLength+3, 1);
    memcpy(&c5, myBuf+myReadLength+4, 1);
    memcpy(&c6, myBuf+myReadLength+5, 1);
    memcpy(&c7, myBuf+myReadLength+6, 1);
    memcpy(&c8, myBuf+myReadLength+7, 1);
    ret = ((ArTypes::Byte8)c1 & 0xff) | ((ArTypes::Byte8) c2 << 8) | ((ArTypes::Byte8) c3 << 16) | ((ArTypes::Byte8) c4 << 24) | ((ArTypes::Byte8) c5 << 32) | ((ArTypes::Byte8) c6 << 40) | ((ArTypes::Byte8) c7 << 48) | ((ArTypes::Byte8) c8 << 56);
    myReadLength+=8;
  }

  return ret;
}

AREXPORT ArTypes::UByte ArBasePacket::bufToUByte(void)
{
  ArTypes::UByte ret=0;

  if (isNextGood(1))
  {
    memcpy(&ret, myBuf+myReadLength, 1);
    myReadLength+=1;
  }

  return(ret);
}

AREXPORT ArTypes::UByte2 ArBasePacket::bufToUByte2(void)
{
  ArTypes::UByte2 ret=0;
  unsigned char c1, c2;

  if (isNextGood(2))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    ret = (c1 & 0xff) | (c2 << 8);
    myReadLength+=2;
  }

  return ret;
}

AREXPORT ArTypes::UByte4 ArBasePacket::bufToUByte4(void)
{
  /// MPL 2013_10_23 this was Byte4 not UByte4
  //ArTypes::Byte4 ret=0;
  ArTypes::UByte4 ret=0;
  unsigned char c1, c2, c3, c4;

  if (isNextGood(4))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    memcpy(&c3, myBuf+myReadLength+2, 1);
    memcpy(&c4, myBuf+myReadLength+3, 1);
    ret = (c1 & 0xff) | (c2 << 8) | (c3 << 16) | (c4 << 24);
    myReadLength+=4;
  }

  return ret;
}

AREXPORT ArTypes::UByte8 ArBasePacket::bufToUByte8(void)
{
  ArTypes::UByte8 ret=0;
  unsigned char c1, c2, c3, c4, c5, c6, c7, c8;

  if (isNextGood(8))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    memcpy(&c3, myBuf+myReadLength+2, 1);
    memcpy(&c4, myBuf+myReadLength+3, 1);
    memcpy(&c5, myBuf+myReadLength+4, 1);
    memcpy(&c6, myBuf+myReadLength+5, 1);
    memcpy(&c7, myBuf+myReadLength+6, 1);
    memcpy(&c8, myBuf+myReadLength+7, 1);
    ret = ((ArTypes::UByte8)c1 & 0xff) | ((ArTypes::UByte8)c2 << 8) | ((ArTypes::UByte8)c3 << 16) | ((ArTypes::UByte8)c4 << 24) | ((ArTypes::UByte8)c5 << 32) | ((ArTypes::UByte8)c6 << 40) | ((ArTypes::UByte8)c7 << 48) | ((ArTypes::UByte8) c8 << 56);
    myReadLength+=8;
  }

  return ret;
}

/** 
Copy a string from the packet buffer into the given buffer, stopping when
the end of the packet buffer is reached, the given length is reached,
or a NUL character ('\\0') is reached.  If the given length is not large
enough, then the remainder of the string is flushed from the packet.
A NUL character ('\\0') is appended to @a buf if there is sufficient room
after copying the sting from the packet, otherwise no NUL is added (i.e.
if @a len bytes are copied).
@param buf Destination buffer
@param len Maximum number of characters to copy into the destination buffer
*/
AREXPORT void ArBasePacket::bufToStr(char *buf, int len)
{
   if (buf == NULL) {
    ArLog::log(ArLog::Normal, "ArBasePacket::bufToStr(NULL, %d) cannot write to null address",
               len);
    return;
  }

  int i;

  buf[0] = '\0';
  // see if we can read
  if (isNextGood(1))
  {
    // while we can read copy over those bytes
    for (i = 0; 
         isNextGood(1) && i < (len - 1) && myBuf[myReadLength] != '\0';
         ++myReadLength, ++i) {
      buf[i] = myBuf[myReadLength];
    }
    // if we stopped because of a null then copy that one too
    if (myBuf[myReadLength] == '\0')
    {
      buf[i] = myBuf[myReadLength];
      myReadLength++;
    }
    else if (i >= (len - 1)) { 

      // Otherwise, if we stopped reading because the output buffer was full,
      // then attempt to flush the rest of the string from the packet

      // This is a bit redundant with the code below, but wanted to log the  
      // string for debugging
      myBuf[len - 1] = '\0';

      ArLog::log(ArLog::Normal, "ArBasePacket::bufToStr(buf, %d) output buf is not large enough for packet string %s",
                 len, myBuf);

      while (isNextGood(1) && (myBuf[myReadLength] != '\0')) {
        myReadLength++;
      }
      if (myBuf[myReadLength] == '\0') {
        myReadLength++;
      }
    } // end else if output buffer filled before null-terminator
  } // end if something to read

  // Make absolutely sure that the string is null-terminated...
  buf[len - 1] = '\0';

}

/// Note the string obtained from the packet can have at most 512 characters.
AREXPORT std::string ArBasePacket::bufToString()
{
  // todo own implementation to avoid needing temporary buf rather than
  // calling bufTotr() with temporary buffer
  char buf[512];
  bufToStr(buf, 512);
  return buf;
}

AREXPORT void ArBasePacket::bufToString(std::string *s)
{
  // todo own implementation to avoid needing temporary buf rather than
  // calling bufTotr() with temporary buffer
  char buf[512];
  bufToStr(buf, 512);
  s->assign(buf);
}

/**
copies length bytes from the buffer into data, length is passed in, not read
from packet
@param data character array to copy the data into
@param length number of bytes to copy into data
*/
AREXPORT void ArBasePacket::bufToData(char *data, int length)
{
  if (data == NULL) {
    ArLog::log(ArLog::Normal, "ArBasePacket::bufToData(NULL, %d) cannot write to null address",
               length);
    return;
  }
  if (isNextGood(length))
  {
    memcpy(data, myBuf+myReadLength, length);
    myReadLength += length;
  }
}


/**
   This was added to get around having to cast data you put in, since the data shouldn't really matter if its signed or unsigned.

copies length bytes from the buffer into data, length is passed in, not read
from packet
@param data character array to copy the data into
@param length number of bytes to copy into data
*/
AREXPORT void ArBasePacket::bufToData(unsigned char *data, int length)
{
  if (data == NULL) {
    ArLog::log(ArLog::Normal, "ArBasePacket::bufToData(NULL, %d) cannot write to null address",
               length);
    return;
  }
  if (isNextGood(length))
  {
    memcpy(data, myBuf+myReadLength, length);
    myReadLength += length;
  }
}


/**
Copies the given packets buffer into the buffer of this packet, also
sets this length and readlength to what the given packet has
@param packet the packet to duplicate
*/
AREXPORT void ArBasePacket::duplicatePacket(ArBasePacket *packet)
{
  myLength = packet->getLength();
  myReadLength = packet->getReadLength();

  // KMC Added this because otherwise... If myMaxLength < packet->getMaxLength(),
  // then this will overwrite memory.
  //
  if (myMaxLength < myLength) {
    setMaxLength(myLength);
  }

  memcpy(myBuf, packet->getBuf(), myLength);
}

AREXPORT void ArBasePacket::log(void)
{
  int i;
  ArLog::log(ArLog::Terse, "Packet: (length = %i)", myLength);
  for (i = 0; i < myLength; i++)
    ArLog::log(ArLog::Terse, "  [%03i] % 5d\t0x%x", i,(unsigned char) myBuf[i],
        (unsigned char) myBuf[i]);
  ArLog::log(ArLog::Terse, "\n");
}

AREXPORT void ArBasePacket::printHex(void)
{
  int i;
  ArLog::log(ArLog::Terse, "Packet: (length = %i)", myLength);
  for (i = 0; i < myLength; i++)
    ArLog::log(ArLog::Terse, "  [%i] 0x%x ", i,(unsigned char) myBuf[i]);
  ArLog::log(ArLog::Terse, "\n");
}

