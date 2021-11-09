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
#ifndef ARBASEPACKET_H
#define ARBASEPACKET_H


#include <string>
#include <vector>
#include <list>
#include <assert.h>
#include "Aria/ariaTypedefs.h"

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
    placed in that buffer),
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
  AREXPORT ArBasePacket(uint16_t bufferSize = 0,
    uint16_t headerLength = 0, 
    char * buf = NULL,
    uint16_t footerLength = 0);

  /// Copy constructor
  AREXPORT ArBasePacket(const ArBasePacket &other);

  /// Assignment operator
  AREXPORT ArBasePacket &operator=(const ArBasePacket &other);

  /// Destructor
  AREXPORT virtual ~ArBasePacket();

  ArBasePacket(ArBasePacket &&old) noexcept;

  ArBasePacket &operator=(ArBasePacket &&other) noexcept;

  /// resets the length for more data to be added
  AREXPORT virtual void empty();

  /// MakeFinals the packet in preparation for sending, must be done
  AREXPORT virtual void finalizePacket() {}

  /// ArLogs the hex and decimal values of each byte of the packet, and possibly extra metadata as well
  AREXPORT virtual void log();
  /// ArLogs the hex value of each byte in the packet 
  AREXPORT virtual void printHex();

  /// Returns whether the packet is valid, i.e. no error has occurred when reading/writing.
  AREXPORT virtual bool isValid();

  /// Resets the valid state of the packet.
  AREXPORT virtual void resetValid();

  // Utility functions to write different data types to a buffer. They will
  // increment the length.

  /// Puts int8_t into packets buffer as a single signed byte
  AREXPORT virtual void byteToBuf(int8_t val);
  void append(int8_t val) { byteToBuf(val); }

  /// Puts int16_t into packets buffer as two bytes with sign
  AREXPORT virtual void byte2ToBuf(int16_t val);
  void append(int16_t val) { byte2ToBuf(val);  }

  /// Puts int32_t into packets buffer as four bytes with sign
  AREXPORT virtual void byte4ToBuf(int32_t val);
  void append(int32_t val) { byte4ToBuf(val);  }

  /// Puts int64_t into packets buffer and 8 bytes
  AREXPORT virtual void byte8ToBuf(int64_t val);
  void append(int64_t val) { byte8ToBuf(val);  }

  /// Puts uint8_t into packets buffer as single unsigned byte
  AREXPORT virtual void uByteToBuf(uint8_t val);
  void append(uint8_t val) { uByteToBuf(val);  }

  /// Puts uint16_t into packet buffer as two bytes
  AREXPORT virtual void uByte2ToBuf(uint16_t val);
  void append(uint16_t val) { uByte2ToBuf(val);  }

  /// Puts uint32_t into packet buffer as 4 bytes
  AREXPORT virtual void uByte4ToBuf(uint32_t val);
  void append(uint32_t val) { uByte4ToBuf(val);  }

  /// Puts uint64_t into packet buffer as 8 bytes
  AREXPORT virtual void uByte8ToBuf(uint64_t val);
  void append(uint64_t val) { uByte8ToBuf(val);  }

  /// Puts a NULL-terminated string into packet buffer
  AREXPORT virtual void strToBuf(const char *str);
  void append(const char *str) { strToBuf(str); }
  void strToBuf(const std::string &str) { strToBuf(str.c_str()); }
  void append(const std::string &str) { strToBuf(str.c_str());  }

  /**
   * @brief Copies the given number of bytes from str into packet buffer
   * @deprecated use strToBufPadded(), strToBuf(), or dataToBuf() instead
  **/
  AREXPORT virtual void strNToBuf(const char *str, size_t length);
  void strNToBuf(const std::string &str, size_t len) { strNToBuf(str.data(), len); }
  /// Copies length bytes from str, if str ends before length, pads data with 0s
  AREXPORT virtual void strToBufPadded(const char *str, size_t length);
  /// Copies length bytes from data into packet buffer
  AREXPORT virtual void dataToBuf(const char *data, size_t length);
  /// Copies length bytes from data into packet buffer
  AREXPORT virtual void dataToBuf(const unsigned char *data, size_t length);
  void dataToBuf(const std::string &data, size_t len) { dataToBuf(data.data(), len);  }

  // Utility functions to read different data types from a buffer. Each read
  // will increment the myReadLength.

  /// Remove one byte from the packet buffer and return as int8_t
  AREXPORT virtual int8_t bufToByte();
  /// Remove two bytes from the packet buffer and return as int16_t
  AREXPORT virtual int16_t bufToByte2();
  /// Remove four bytes from the packet buffer and return as int32_t
  AREXPORT virtual int32_t bufToByte4();
  /// Remove eight bytes from the packet buffer and return as int64_t
  AREXPORT virtual int64_t bufToByte8();

  /// Remove one byte from the packet buffer and return as uint8_t
  AREXPORT virtual uint8_t bufToUByte();
  /// Remove two bytes from the packet buffer and return as uint16_t
  AREXPORT virtual uint16_t bufToUByte2();
  /// Remove four bytes from the packet buffer and return as uint32_t
  AREXPORT virtual uint32_t bufToUByte4();
  /// Remove eight bytes from the packet buffer and return as uint64_t
  AREXPORT virtual uint64_t bufToUByte8();

  /// Use extract<T>() to get a value of type T from the packet buffer, where T is one of int8_t, int16_t, etc.
  /// Same as the bufTo...() methods.
  /// E.g. <code>auto val = packet.extract<uint32_t>();</code>
  //@{
  template <typename T> T extract() { assert(false); }
  template <int8_t> int8_t extract() { return bufToByte();  }
  template <int16_t> int16_t extract() { return bufToByte2();  }
  template <int32_t> int32_t extract() { return bufToByte4();  }
  template <int64_t> int64_t extract() { return bufToByte8();  }
  template <uint8_t> uint8_t extract() { return bufToUByte();  }
  template <uint16_t> uint16_t extract() { return bufToUByte2();  }
  template <uint32_t> uint32_t extract() { return bufToUByte4();  }
  template <uint64_t> uint64_t extract() { return bufToUByte8();  }
  //@}

  /// Remove a null-terminated string from the buffer
  AREXPORT virtual void bufToStr(char *buf, size_t maxlen);
  /// Remove a null-terminated string from the buffer
  AREXPORT std::string bufToString();
  /// Remove a null-terminated string from the buffer
  AREXPORT void bufToString(std::string *s);
  /// Remove number of bytes specified by @a length from buffer and put them into @a data
  AREXPORT virtual void bufToData(char *data, size_t length);
  /// Remove number of bytes specified by @a length bytes from buffer and put them into @a data
  AREXPORT virtual void bufToData(unsigned char *data, size_t length);

  /// Restart the reading process
  AREXPORT virtual void resetRead();

  // Accessors

  /// Gets the total length of the packet
  virtual uint16_t getLength() const { return myLength; }
  /// Gets the length of the data in the packet
  AREXPORT virtual uint16_t getDataLength() const;

  /// Gets how far into the packet that has been read
  virtual uint16_t getReadLength() const { return myReadLength; }
  /// Gets how far into the data of the packet that has been read
  virtual uint16_t getDataReadLength() const { return myReadLength - myHeaderLength; }
  /// Gets the length of the header
  virtual uint16_t getHeaderLength() const
  {
    return myHeaderLength; }
  /// Gets the length of the header
  virtual uint16_t getFooterLength() const
  { return myFooterLength; }

  /// Gets the maximum length packet
  virtual uint16_t getMaxLength() const { return myMaxLength; }

  /// Gets a const pointer to the buffer the packet uses 
  AREXPORT virtual const char * getBuf() const;

  /// Gets a pointer to the buffer the packet uses 
  AREXPORT virtual char * getBuf();

  /// Sets the buffer the packet is using
  AREXPORT virtual void setBuf(char *buf, uint16_t bufferSize);
  /// Sets the maximum buffer size (if new size is <= current does nothing)
  AREXPORT virtual void setMaxLength(uint16_t bufferSize);
  /// Sets the length of the packet
  AREXPORT virtual bool setLength(uint16_t length);
  /// Sets the read length
  AREXPORT virtual void setReadLength(uint16_t readLength);
  /// Sets the length of the header
  AREXPORT virtual bool setHeaderLength(uint16_t length);
  /// Makes this packet a duplicate of another packet
  AREXPORT virtual void duplicatePacket(ArBasePacket *packet);

  bool bufferContainsBytes(size_t bytes) { return isNextGood(bytes); }

protected:
  // internal function to make sure we have enough length left to read in the packet
  AREXPORT bool isNextGood(size_t bytes);

  /// Returns true if there is enough room in the packet to add the specified number of bytes
  AREXPORT bool hasWriteCapacity(size_t bytes);
  //bool hasWriteCapacity(size_t bytes) { return hasWriteCapacity((int)bytes); }

  // internal data
  uint16_t myHeaderLength;
  uint16_t myFooterLength;
  uint16_t myMaxLength;

  uint16_t myReadLength;
  bool myOwnMyBuf;

  // Actual packet data
  char *myBuf;
  uint16_t myLength;

  // Whether no error has occurred in reading/writing the packet.
  bool myIsValid;

};


#endif // ARPACKET_H
