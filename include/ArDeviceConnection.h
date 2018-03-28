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
#ifndef ARDEVICECONNECTION_H
#define ARDEVICECONNECTION_H

#include <string>
#include "ariaTypedefs.h"
#include "ariaUtil.h"
#include "ArBasePacket.h"

/// Base class for device connections
/**
   Base class for device connections, this is mostly for connections to the 
   robot or simulator but could also be used for a connection to a laser
   or other device

   Note that this is mostly a base class, so if you'll want to use one of the
   classes which inherit from this one... also note that in those classes 
   is where you'll find setPort which sets the place the device connection
   will try to connect to... the inherited classes also have an open which
   returns more detailed information about the open attempt, and which takes 
   the parameters for where to connect
*/
class ArDeviceConnection
{
 public:
  /// constructor
  AREXPORT ArDeviceConnection();
  /// destructor also forces a close on the connection
  AREXPORT virtual ~ArDeviceConnection();
  /// Reads data from connection
  /**
     Reads data from connection
     @param data pointer to a character array to read the data into
     @param size maximum number of bytes to read
     @param msWait read blocks for this many milliseconds (not at all for == 0)
     @return number of bytes read, or -1 for failure
     @see write, writePacket
  */
  AREXPORT virtual int read(const char *data, unsigned int size, 
			    unsigned int msWait = 0) = 0;
  /// Writes data to connection
  /**
     Writes data to connection from a packet
     @param packet pointer to a packet to write the data from
     @return number of bytes written, or -1 for failure
     @see read, write
  */
  AREXPORT virtual int writePacket(ArBasePacket *packet)
    { if (packet == NULL || packet->getLength() == 0) return 0;
    return write(packet->getBuf(), packet->getLength()); }
  /// Writes data to connection
  /**
     Writes data to connection
     @param data pointer to a character array to write the data from
     @param size number of bytes to write
     @return number of bytes read, or -1 for failure
     @see read, writePacket
  */
  AREXPORT virtual int write(const char *data, unsigned int size) = 0;
  /// Gets the status of the connection, which is one of the enum status
  /**
     Gets the status of the connection, which is one of the enum status.
     If you want to get a string to go along with the number, use 
     getStatusMessage
     @return the status of the connection
     @see getStatusMessage
  */
  AREXPORT virtual int getStatus(void) = 0;
  /// Gets the description string associated with the status
  /** 
      @param messageNumber the int from getStatus you want the string for
      @return the description associated with the status
      @see getStatus
  */
  AREXPORT const char *getStatusMessage(int messageNumber) const;

  /// Opens the connection again, using the values from setLocation or 
  // a previous open
  virtual bool openSimple(void) = 0;
    
  /// Closes the connection
  /**
     @return whether the close succeeded or not
  */
  virtual bool close(void) { return false; }
  /// Gets the string of the message associated with opening the device
  /**
     Each class inherited from this one has an open method which returns 0
     for success or an integer which can be passed into this function to 
     obtain a string describing the reason for failure
     @param messageNumber the number returned from the open
     @return the error description associated with the messageNumber
  */
  AREXPORT virtual const char * getOpenMessage(int messageNumber) = 0;
  enum Status { 
      STATUS_NEVER_OPENED = 1, ///< Never opened
      STATUS_OPEN,  ///< Currently open
      STATUS_OPEN_FAILED, ///< Tried to open, but failed
      STATUS_CLOSED_NORMALLY, ///< Closed by a close call
      STATUS_CLOSED_ERROR ///< Closed because of error
  };
  /// Gets the time data was read in
  /** 
      @param index looks like this is the index back in the number of bytes
      last read in
      @return the time the last read data was read in 
  */
  AREXPORT virtual ArTime getTimeRead(int index) = 0;
  /// sees if timestamping is really going on or not
  /** @return true if real timestamping is happening, false otherwise */
  AREXPORT virtual bool isTimeStamping(void) = 0;

  /// Gets the port name
  AREXPORT const char *getPortName(void) const;
  /// Gets the port type
  AREXPORT const char *getPortType(void) const;
  
  /// Sets the device type (what this is connecting to)
  AREXPORT void setDeviceName(const char *deviceName);
  /// Gets the device type (what this is connecting to)
  AREXPORT const char *getDeviceName(void) const;

  /// Notifies the device connection that the start of a packet is
  /// trying to be read
  AREXPORT void debugStartPacket(void);
  /// Notifies the device connection that some bytes were read (should
  /// call with 0 if it read but got no bytes)
  AREXPORT void debugBytesRead(int bytesRead);
  /// Notifies the device connection that the end of a packet was
  /// read, which will cause log messages if set to do so
  AREXPORT void debugEndPacket(bool goodPacket, int type = 0);
  /// Makes all device connections so that they'll dump data
  AREXPORT static bool debugShouldLog(bool shouldLog);
 protected:
  /// Sets the port name
  AREXPORT void setPortName(const char *portName);
  /// Sets the port type
  AREXPORT void setPortType(const char *portType);

  void buildStrMap(void);
  static bool ourStrMapInited;
  static ArStrMap ourStrMap;

  std::string myDCPortName;
  std::string myDCPortType;
  std::string myDCDeviceName;

  static bool ourDCDebugShouldLog;
  static ArTime ourDCDebugFirstTime;
  bool myDCDebugPacketStarted;
  ArTime myDCDebugStartTime;
  ArTime myDCDebugFirstByteTime;
  ArTime myDCDebugLastByteTime;
  int myDCDebugBytesRead;
  int myDCDebugTimesRead;
  long long myDCDebugNumGoodPackets;
  long long myDCDebugNumBadPackets;
};

#endif


