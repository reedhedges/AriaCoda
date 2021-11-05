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
#include "Aria/ArBasePacket.h"
#include "Aria/ArLog.h"
#include "Aria/ariaUtil.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
@param bufferSize size of the data buffer. Note default is 0, so packet will have no data capacity unless set.
@param headerLength length of the header
@param buf buffer packet uses, if NULL, instance will allocate memory
@param footerLength length of the footer following the data
*/
AREXPORT ArBasePacket::ArBasePacket(uint16_t bufferSize, 
                                    uint16_t headerLength,
                                    char * buf,
                                    uint16_t footerLength) 
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

AREXPORT ArBasePacket::ArBasePacket(ArBasePacket &&other) noexcept
{
  // move constructor
  
  myHeaderLength = other.myHeaderLength;
  myFooterLength = other.myFooterLength;
  myReadLength   = other.myReadLength;

  myBuf = other.myBuf;

  if(other.myOwnMyBuf )
    myOwnMyBuf = true;

  other.myOwnMyBuf = false;
  other.myBuf = NULL;

  myLength = other.myLength;
  myMaxLength = other.myLength;   
  myIsValid = other.myIsValid;
  other.myIsValid = false;
}

AREXPORT ArBasePacket& ArBasePacket::operator=(ArBasePacket&& other) noexcept
{
  // move assignment
  if(&other == this)
    return *this;

  myHeaderLength = other.myHeaderLength;
  myFooterLength = other.myFooterLength;
  myReadLength   = other.myReadLength;

  if (myOwnMyBuf && myBuf != NULL)
	  delete [] myBuf;

  myBuf = other.myBuf;

  if(other.myOwnMyBuf )
    myOwnMyBuf = true;

  other.myOwnMyBuf = false;
  other.myBuf = NULL;

  myLength = other.myLength;
  myMaxLength = other.myLength;   
  myIsValid = other.myIsValid;
  other.myIsValid = false;

  return *this;
}


AREXPORT ArBasePacket::~ArBasePacket()
{
  if (myOwnMyBuf && myBuf != NULL)
    delete[] myBuf;
}


AREXPORT void ArBasePacket::setBuf(char *buf, uint16_t bufferSize)
{
  if (myOwnMyBuf) 
  {
    delete[] myBuf;
    myOwnMyBuf = false;
  } 
  myBuf = buf;
  myMaxLength = bufferSize;
}

AREXPORT void ArBasePacket::setMaxLength(uint16_t bufferSize)
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

AREXPORT bool ArBasePacket::setLength(uint16_t length)
{
  if (myOwnMyBuf && length > myMaxLength)
    return false;

  myLength = length;
  return true;
}

AREXPORT void ArBasePacket::setReadLength(uint16_t readLength)
{
  myReadLength = readLength;
}

AREXPORT bool ArBasePacket::setHeaderLength(uint16_t length)
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

AREXPORT void ArBasePacket::resetRead()
{
  myReadLength = myHeaderLength;
  resetValid();
}

uint16_t ArBasePacket::getDataLength() const { 
 
  // KMC 12/20/13 Do not allow negative values to be returned.  (They are basically 
  // converted to an erroneous positive value by the UByte2.)
  int len = myLength - myHeaderLength - myFooterLength; 
  if (len >= 0) {
    return (uint16_t) len;
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

AREXPORT void ArBasePacket::empty()
{
  myLength = myHeaderLength;
  resetValid();
}

AREXPORT bool ArBasePacket::isNextGood(size_t bytes)
{
/*   if (bytes <= 0)
    return false;
 */
  // make sure it comes in before the header
  if ((size_t)myReadLength + bytes <= (size_t)myLength - (size_t)myFooterLength)
    return true;

  myIsValid = false;

  return false;
}


AREXPORT bool ArBasePacket::hasWriteCapacity(size_t bytes)
{
/*   if (bytes < 0) {
    ArLog::log(ArLog::Normal, "ArBasePacket::hasWriteCapacity(%d) cannot write negative amount",
               bytes);
    return false;
  } */

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
AREXPORT bool ArBasePacket::isValid()
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

AREXPORT const char *ArBasePacket::getBuf() const
{
  return myBuf;
}

AREXPORT char *ArBasePacket::getBuf() 
{
  return myBuf;
}

AREXPORT void ArBasePacket::byteToBuf(int8_t val)
{
  if (!hasWriteCapacity(1)) {
    return;
  }

  memcpy(myBuf+myLength, &val, 1);
  myLength += 1;
}

AREXPORT void ArBasePacket::byte2ToBuf(int16_t val)
{
  if (!hasWriteCapacity(2)) {
    return;
  }

  unsigned char c;
  c = (unsigned char) ((val >> 8) & 0xff);
  memcpy(myBuf+myLength+1, &c, 1);
  c = (unsigned char) (val & 0xff);
  memcpy(myBuf+myLength, &c, 1);
  myLength += 2;
}

AREXPORT void ArBasePacket::byte4ToBuf(int32_t val)
{
  if (!hasWriteCapacity(4)) {
    return;
  }

  unsigned char c;
  c = (unsigned char) ((val >> 24) & 0xff);
  memcpy(myBuf+myLength+3, &c, 1);
  c =  (unsigned char) ((val >> 16) & 0xff);
  memcpy(myBuf+myLength+2, &c, 1);
  c = ((unsigned char) (val >> 8) & 0xff);
  memcpy(myBuf+myLength+1, &c, 1);
  c = (unsigned char) (val & 0xff);
  memcpy(myBuf+myLength, &c, 1);
  myLength += 4;
}

AREXPORT void ArBasePacket::byte8ToBuf(int64_t val)
{
  if (!hasWriteCapacity(8)) {
    return;
  }

  unsigned char c;
  c = (unsigned char)( (val >> 56) & 0xff);
  memcpy(myBuf+myLength+7, &c, 1);
  c = (unsigned char) ((val >> 48) & 0xff);
  memcpy(myBuf+myLength+6, &c, 1);
  c = (unsigned char) ((val >> 40) & 0xff);
  memcpy(myBuf+myLength+5, &c, 1);
  c = (unsigned char) ((val >> 32) & 0xff);
  memcpy(myBuf+myLength+4, &c, 1);

  c = (unsigned char) ((val >> 24) & 0xff);
  memcpy(myBuf+myLength+3, &c, 1);
  c = (unsigned char) ((val >> 16) & 0xff);
  memcpy(myBuf+myLength+2, &c, 1);
  c = (unsigned char) ((val >> 8) & 0xff);
  memcpy(myBuf+myLength+1, &c, 1);
  c = (unsigned char) (val & 0xff);
  memcpy(myBuf+myLength, &c, 1);
  myLength += 8;

}

AREXPORT void ArBasePacket::uByteToBuf(uint8_t val)
{
  if (!hasWriteCapacity(1)) {
    return;
  }
  memcpy(myBuf+myLength, &val, 1);
  myLength += 1;
}

AREXPORT void ArBasePacket::uByte2ToBuf(uint16_t val)
{
  if (!hasWriteCapacity(2)) {
    return;
  }
  // Note that MSB is placed one byte after the LSB in the end of the buffer:
  unsigned char c;
  c = (unsigned char)((val >> 8) & 0xff);
  memcpy(myBuf+myLength+1, &c, 1);
  c = (unsigned char)(val & 0xff);
  memcpy(myBuf+myLength, &c, 1);
  myLength += 2;
}

AREXPORT void ArBasePacket::uByte4ToBuf(uint32_t val)
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
  c = (unsigned char)((val >> 24) & 0xff);
  memcpy(myBuf+myLength+3, &c, 1);
  c = (unsigned char)((val >> 16) & 0xff);
  memcpy(myBuf+myLength+2, &c, 1);
  c = (unsigned char)((val >> 8) & 0xff);
  memcpy(myBuf+myLength+1, &c, 1);
  c = (unsigned char)(val & 0xff);
  memcpy(myBuf+myLength, &c, 1);
  myLength += 4;
}

AREXPORT void ArBasePacket::uByte8ToBuf(uint64_t val)
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
  c = (unsigned char)((val >> 56) & 0xff);
  memcpy(myBuf+myLength+7, &c, 1);
  c = (unsigned char)((val >> 48) & 0xff);
  memcpy(myBuf+myLength+6, &c, 1);
  c = (unsigned char)((val >> 40) & 0xff);
  memcpy(myBuf+myLength+5, &c, 1);
  c = (unsigned char)((val >> 32) & 0xff);
  memcpy(myBuf+myLength+4, &c, 1);

  c = (unsigned char)((val >> 24) & 0xff);
  memcpy(myBuf+myLength+3, &c, 1);
  c = (unsigned char)((val >> 16) & 0xff);
  memcpy(myBuf+myLength+2, &c, 1);
  c = (unsigned char)((val >> 8) & 0xff);
  memcpy(myBuf+myLength+1, &c, 1);
  c = (unsigned char)(val & 0xff);
  memcpy(myBuf+myLength, &c, 1);
  myLength += 8;
}

/**
  Copy null-terminated string into buffer.  If there is not capacity for the string, do not add it.
*/
AREXPORT void ArBasePacket::strToBuf(const char *str)
{
  if (str == NULL) {
    str = "";
  }

  const size_t tempLen = strlen(str) + 1;

  if (!hasWriteCapacity(tempLen)) {
    return; // todo return or throw error
  }

  memcpy(myBuf+myLength, str, tempLen);
  myLength += (uint16_t) tempLen;
}

/**
 * Copy the given number of bytes from char* into buffer. 
 * This method performs no bounds/termination checking on the string length or on the capacity of the packet buffer. 
 * The string in the packet buffer will not be not null terminated unless null byte was included in given length, 
 * nor is given length stored in the buffer.  For string operations, strNToBufPadded() or strToBuf()
 * is preferred.  For raw data operations, dataToBuf() is preferred.
@param str character array to copy into the packet buffer
@param length how many characters to copy from str into the packet buffer
*/
AREXPORT void ArBasePacket::strNToBuf(const char *str, size_t length)
{
  // Do not perform bounds checking because it breaks existing code.

  //byte4ToBuf(length); // do this in a different method for length-prefixed strings?
  memcpy(myBuf+myLength, str, length);
  myLength+=(uint16_t)length;

}


/**
* Copy string into packet buffer, padding to the given length if neccesary with null characters (i.e. copy the string into a fixed length field in the packet).
If string ends before length it pads the string with NUL ('\\0') characters.
@param str character array to copy into buffer
@param length how many bytes to copy from the str into packet
*/
AREXPORT void ArBasePacket::strToBufPadded(const char *str, size_t length)
{
  if (str == NULL) {
    str = "";
  }
  size_t tempLen = strlen(str);

  if (!hasWriteCapacity(length)) {
    return;
  }

  assert(length > 0);
  if (tempLen >= (size_t)length) {
    memcpy(myBuf + myLength, str, length);
    myLength += (uint16_t)length;
  }
  else // string is smaller than given length
  {
    memcpy(myBuf + myLength, str, tempLen);
    myLength += (uint16_t) tempLen;
    memset(myBuf + myLength, 0, length - tempLen);
    myLength += (uint16_t)(length - tempLen);
  }
}


/**
@param data character array to copy into buffer
@param length how many bytes to copy from data into packet
*/
AREXPORT void ArBasePacket::dataToBuf(const char *data, size_t length)
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
  myLength+=(uint16_t)length;

}

/**
   This was added to get around having to cast data you put in, since the data shouldn't really matter if its signed or unsigned.
@param data character array to copy into buffer
@param length how many bytes to copy from data into packet
*/
AREXPORT void ArBasePacket::dataToBuf(const unsigned char *data, size_t length)
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
  myLength+=(uint16_t)length;

}


AREXPORT int8_t ArBasePacket::bufToByte()
{
  int8_t ret=0;

  if (isNextGood(1))
  {
    memcpy(&ret, myBuf+myReadLength, 1);
    myReadLength+=1;
  }

  return(ret);
}

AREXPORT int16_t ArBasePacket::bufToByte2()
{
  int16_t ret=0;
  unsigned char c1, c2;

  if (isNextGood(2))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    ret = (int16_t) ((c1 & 0xff) | (c2 << 8));
    myReadLength+=2;
  }

  return ret;
}

AREXPORT int32_t ArBasePacket::bufToByte4()
{
  int32_t ret=0;
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

AREXPORT int64_t ArBasePacket::bufToByte8()
{
  int64_t ret=0;
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
    ret = ((int64_t)c1 & 0xff) | ((int64_t) c2 << 8) | ((int64_t) c3 << 16) | ((int64_t) c4 << 24) | ((int64_t) c5 << 32) | ((int64_t) c6 << 40) | ((int64_t) c7 << 48) | ((int64_t) c8 << 56);
    myReadLength+=8;
  }

  return ret;
}

AREXPORT uint8_t ArBasePacket::bufToUByte()
{
  uint8_t ret=0;

  if (isNextGood(1))
  {
    memcpy(&ret, myBuf+myReadLength, 1);
    myReadLength+=1;
  }

  return(ret);
}

AREXPORT uint16_t ArBasePacket::bufToUByte2()
{
  uint16_t ret=0;
  unsigned char c1, c2;

  if (isNextGood(2))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    ret = (uint16_t) ((c1 & 0xff) | (c2 << 8));
    myReadLength+=2;
  }

  return ret;
}

AREXPORT uint32_t ArBasePacket::bufToUByte4()
{
  /// MPL 2013_10_23 this was Byte4 not UByte4
  //int32_t ret=0;
  uint32_t ret=0;
  unsigned char c1, c2, c3, c4;

  if (isNextGood(4))
  {
    memcpy(&c1, myBuf+myReadLength, 1);
    memcpy(&c2, myBuf+myReadLength+1, 1);
    memcpy(&c3, myBuf+myReadLength+2, 1);
    memcpy(&c4, myBuf+myReadLength+3, 1);
    ret = (uint32_t) ((c1 & 0xff) | (c2 << 8) | (c3 << 16) | (c4 << 24));
    myReadLength+=4;
  }

  return ret;
}

AREXPORT uint64_t ArBasePacket::bufToUByte8()
{
  uint64_t ret=0;
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
    ret = ((uint64_t)c1 & 0xff) | ((uint64_t)c2 << 8) | ((uint64_t)c3 << 16) | ((uint64_t)c4 << 24) | ((uint64_t)c5 << 32) | ((uint64_t)c6 << 40) | ((uint64_t)c7 << 48) | ((uint64_t) c8 << 56);
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
AREXPORT void ArBasePacket::bufToStr(char *buf, size_t len)
{
   if (buf == NULL) {
    ArLog::log(ArLog::Normal, "ArBasePacket::bufToStr(NULL, %d) cannot write to null address",
               len);
    return;
  }

  buf[0] = '\0';
  // see if we can read
  if (isNextGood(1))
  {
    size_t i = 0;
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
  // calling bufTotSr() with temporary buffer
  char buf[512];
  bufToStr(buf, 512);
  return buf;
}

AREXPORT void ArBasePacket::bufToString(std::string *s)
{
  // todo own implementation to avoid needing temporary buf rather than
  // calling bufToStr() with temporary buffer
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
AREXPORT void ArBasePacket::bufToData(char *data, size_t length)
{
  if (data == NULL) {
    ArLog::log(ArLog::Normal, "ArBasePacket::bufToData(NULL, %lu) cannot write to null address",
               length);
    return;
  }
  if (isNextGood(length))
  {
    memcpy(data, myBuf+myReadLength, length);
    myReadLength += (uint16_t) length;
  }
}


/**
   This was added to get around having to cast data you put in, since the data shouldn't really matter if its signed or unsigned.

copies length bytes from the buffer into data, length is passed in, not read
from packet
@param data character array to copy the data into
@param length number of bytes to copy into data
*/
AREXPORT void ArBasePacket::bufToData(unsigned char *data, size_t length)
{
  if (data == NULL) {
    ArLog::log(ArLog::Normal, "ArBasePacket::bufToData(NULL, %d) cannot write to null address",
               length);
    return;
  }
  if (isNextGood(length))
  {
    memcpy(data, myBuf+myReadLength, length);
    myReadLength += (uint16_t) length;
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

AREXPORT void ArBasePacket::log()
{
  int i;
  ArLog::log(ArLog::Terse, "Packet: (length = %i)", myLength);
  for (i = 0; i < myLength; i++)
    ArLog::log(ArLog::Terse, "  [%03i] % 5d\t0x%x", i,(unsigned char) myBuf[i],
        (unsigned char) myBuf[i]);
  ArLog::log(ArLog::Terse, "\n");
}

AREXPORT void ArBasePacket::printHex()
{
  int i;
  ArLog::log(ArLog::Terse, "Packet: (length = %i)", myLength);
  for (i = 0; i < myLength; i++)
    ArLog::log(ArLog::Terse, "  [%i] 0x%x ", i,(unsigned char) myBuf[i]);
  ArLog::log(ArLog::Terse, "\n");
}

