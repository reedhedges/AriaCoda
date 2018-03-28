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
#ifndef ARBASEPACKET_H
#define ARBASEPACKET_H


#include <string>
#include <vector>
#include <list>
#include "ariaTypedefs.h"

/// Base packet class
/** This class is a base class for specific packet types implemented by base
    classes.  In most cases, you would not instantiate this class directly, but instead 
    use a subclass. However, ArBasePacket contains many of the functions used to
    access the packet's data.

    A packet is a sequence of values stored in a buffer.  The contents 
    of a packet's data buffer is read from a device or other program or written to the
    device (for example, a serial port or TCP port
    using an ArDeviceConnection or using ArNetworking), optionally preceded
    by a header with some identifying data and a length, and optionally followed by a 
    footer with a checksum of the data. (If the
    header length of a particular packet type is 0, no header is written or expected on read, and likewise
    with footer.)

    Values are added to the buffer or removed from the buffer in sequence. 
    The "bufTo" methods are used to remove values from the buffer, and the
    "ToBuf" methods are used to add values to the buffer. There are different
    methods for different sized values.

    ArBasePacket keeps a current position index in the buffer, which is the position
    at which new values are added or values are removed. 

    A buffer may be statically allocated externally and supplied to the
    constructor
    (also give a buffer size to determine the maximum amount of data that can be
    placed in that bufer),
    or automatically and dynamically allocated by ArBasePacket as needed
    (the default behavior).

    When it is time to write out a packet, call finalizePacket() to set 
    up the footer if neccesary.
    To reuse a packet, use empty() to reset the buffer; new data will
    then be added to the beginning of the buffer again.
*/
class ArBasePacket
{
public:
    
  /// Constructor
  AREXPORT ArBasePacket(ArTypes::UByte2 bufferSize = 0,
    ArTypes::UByte2 headerLength = 0, 
    char * buf = NULL,
    ArTypes::UByte2 footerLength = 0);

  /// Copy constructor
  AREXPORT ArBasePacket(const ArBasePacket &other);

  /// Assignment operator
  AREXPORT ArBasePacket &operator=(const ArBasePacket &other);

  /// Destructor
  AREXPORT virtual ~ArBasePacket();

  /// resets the length for more data to be added
  AREXPORT virtual void empty(void);

  /// MakeFinals the packet in preparation for sending, must be done
  AREXPORT virtual void finalizePacket(void) {}

  /// ArLogs the hex and decimal values of each byte of the packet, and possibly extra metadata as well
  AREXPORT virtual void log(void);
  /// ArLogs the hex value of each byte in the packet 
  AREXPORT virtual void printHex(void);

  /// Returns whether the packet is valid, i.e. no error has occurred when reading/writing.
  AREXPORT virtual bool isValid(void);

  /// Resets the valid state of the packet.
  AREXPORT virtual void resetValid();

  // Utility functions to write different data types to a buffer. They will
  // increment the length.

  /// Puts ArTypes::Byte into packets buffer
  AREXPORT virtual void byteToBuf(ArTypes::Byte val);
  /// Puts ArTypes::Byte2 into packets buffer
  AREXPORT virtual void byte2ToBuf(ArTypes::Byte2 val);
  /// Puts ArTypes::Byte4 into packets buffer
  AREXPORT virtual void byte4ToBuf(ArTypes::Byte4 val);
  /// Puts ArTypes::Byte8 into packets buffer
  AREXPORT virtual void byte8ToBuf(ArTypes::Byte8 val);

  /// Puts ArTypes::UByte into packets buffer
  AREXPORT virtual void uByteToBuf(ArTypes::UByte val);
  /// Puts ArTypes::UByte2 into packet buffer
  AREXPORT virtual void uByte2ToBuf(ArTypes::UByte2 val);
  /// Puts ArTypes::UByte4 into packet buffer
  AREXPORT virtual void uByte4ToBuf(ArTypes::UByte4 val);
  /// Puts ArTypes::UByte8 into packet buffer
  AREXPORT virtual void uByte8ToBuf(ArTypes::UByte8 val);

  /// Puts a NULL-terminated string into packet buffer
  AREXPORT virtual void strToBuf(const char *str);

  /**
   * @brief Copies the given number of bytes from str into packet buffer
   * @deprecated use strToBufPadded(), strToBuf(), or dataToBuf() instead
  **/
  AREXPORT virtual void strNToBuf(const char *str, int length);
  /// Copies length bytes from str, if str ends before length, pads data with 0s
  AREXPORT virtual void strToBufPadded(const char *str, int length);
  /// Copies length bytes from data into packet buffer
  AREXPORT virtual void dataToBuf(const char *data, int length);
  /// Copies length bytes from data into packet buffer
  AREXPORT virtual void dataToBuf(const unsigned char *data, int length);

  // Utility functions to read differet data types from a bufer. Each read
  // will increment the myReadLength.
  /// Gets a ArTypes::Byte from the buffer
  AREXPORT virtual ArTypes::Byte bufToByte(void);
  /// Gets a ArTypes::Byte2 from the buffer
  AREXPORT virtual ArTypes::Byte2 bufToByte2(void);
  /// Gets a ArTypes::Byte4 from the buffer
  AREXPORT virtual ArTypes::Byte4 bufToByte4(void);
  /// Gets a ArTypes::Byte8 from the buffer
  AREXPORT virtual ArTypes::Byte8 bufToByte8(void);

  /// Gets a ArTypes::UByte from the buffer
  AREXPORT virtual ArTypes::UByte bufToUByte(void);
  /// Gets a ArTypes::UByte2 from the buffer
  AREXPORT virtual ArTypes::UByte2 bufToUByte2(void);
  /// Gets a ArTypes::UByte4 from the buffer
  AREXPORT virtual ArTypes::UByte4 bufToUByte4(void);
  /// Gets a ArTypes::UByte8 from the buffer
  AREXPORT virtual ArTypes::UByte8 bufToUByte8(void);

  /// Gets a null-terminated string from the buffer
  AREXPORT virtual void bufToStr(char *buf, int maxlen);
  /// Gets a null-terminated string from the buffer
  AREXPORT std::string bufToString();
  /// Gets a null-terminated string from the buffer
  AREXPORT void bufToString(std::string *s);
  /// Gets length bytes from buffer and puts them into data
  AREXPORT virtual void bufToData(char * data, int length);
  /// Gets length bytes from buffer and puts them into data
  AREXPORT virtual void bufToData(unsigned char * data, int length);

  /// Restart the reading process
  AREXPORT virtual void resetRead(void);

  // Accessors

  /// Gets the total length of the packet
  virtual ArTypes::UByte2 getLength(void) const { return myLength; }
  /// Gets the length of the data in the packet
  AREXPORT virtual ArTypes::UByte2 getDataLength(void) const;

  /// Gets how far into the packet that has been read
  virtual ArTypes::UByte2 getReadLength(void) const { return myReadLength; }
  /// Gets how far into the data of the packet that has been read
  virtual ArTypes::UByte2 getDataReadLength(void) const { return myReadLength - myHeaderLength; }
  /// Gets the length of the header
  virtual ArTypes::UByte2 getHeaderLength(void) const
  { return myHeaderLength; }
  /// Gets the length of the header
  virtual ArTypes::UByte2 getFooterLength(void) const
  { return myFooterLength; }

  /// Gets the maximum length packet
  virtual ArTypes::UByte2 getMaxLength(void) const { return myMaxLength; }

  /// Gets a const pointer to the buffer the packet uses 
  AREXPORT virtual const char * getBuf(void) const;

  /// Gets a pointer to the buffer the packet uses 
  AREXPORT virtual char * getBuf(void);

  /// Sets the buffer the packet is using
  AREXPORT virtual void setBuf(char *buf, ArTypes::UByte2 bufferSize);
  /// Sets the maximum buffer size (if new size is <= current does nothing)
  AREXPORT virtual void setMaxLength(ArTypes::UByte2 bufferSize);
  /// Sets the length of the packet
  AREXPORT virtual bool setLength(ArTypes::UByte2 length);
  /// Sets the read length
  AREXPORT virtual void setReadLength(ArTypes::UByte2 readLength);
  /// Sets the length of the header
  AREXPORT virtual bool setHeaderLength(ArTypes::UByte2 length);
  /// Makes this packet a duplicate of another packet
  AREXPORT virtual void duplicatePacket(ArBasePacket *packet);

  bool bufferContainsBytes(int bytes) { return isNextGood(bytes); }

protected:
  // internal function to make sure we have enough length left to read in the packet
  AREXPORT bool isNextGood(int bytes);

  /// Returns true if there is enough room in the packet to add the specified number of bytes
  AREXPORT bool hasWriteCapacity(int bytes);

  // internal data
  ArTypes::UByte2 myHeaderLength;
  ArTypes::UByte2 myFooterLength;
  ArTypes::UByte2 myMaxLength;

  ArTypes::UByte2 myReadLength;
  bool myOwnMyBuf;

  // Actual packet data
  char *myBuf;
  ArTypes::UByte2 myLength;

  // Whether no error has occurred in reading/writing the packet.
  bool myIsValid;

};


#endif // ARPACKET_H
