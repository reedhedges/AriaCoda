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
#ifndef ARFILECONNECTION_H
#define ARFILECONNECTION_H

#include "ArDeviceConnection.h"
#include <string>

#include "ariaTypedefs.h"
#include "ArSocket.h"

/// Reads/writes data to a plain file or character device. Used occasionally
/// by certain tests/debugging.  For real connections, use ArSerialConnection or
/// ArTcpDeviceConnection instead.
class ArFileDeviceConnection: public ArDeviceConnection
{
 public:
  /// Constructor
  AREXPORT ArFileDeviceConnection();
  /// Destructor also closes connection
  AREXPORT virtual ~ArFileDeviceConnection();

  /// Opens a connection to the given host and port
  AREXPORT int open(const char *infilename = NULL, const char *outfilename = NULL, int outflags = 0);
  bool openSimple() { return this->open() == 0; }
  AREXPORT virtual bool close(void);
  AREXPORT virtual int read(const char *data, unsigned int size, 
			    unsigned int msWait = 0);
  AREXPORT virtual int write(const char *data, unsigned int size);
  virtual int getStatus() { return myStatus; }
  AREXPORT virtual const char *getOpenMessage(int err);
  AREXPORT virtual ArTime getTimeRead(int index);
  AREXPORT virtual bool isTimeStamping(void);

  /// If >0 then only read at most this many bytes during read(), regardless of supplied size argument
  void setForceReadBufferSize(unsigned int s) { myForceReadBufferSize = s; }

  /// If >0 then add additional, artificial delay in read() by sleeping this many miliseconds per byte read in read().
  void setReadByteDelay(float d) { myReadByteDelay = d; }

protected:
  std::string myInFileName;
  std::string myOutFileName;
  int myInFD;
  int myOutFD;
  int myStatus;
  unsigned int myForceReadBufferSize;
  float myReadByteDelay;
};

#endif
