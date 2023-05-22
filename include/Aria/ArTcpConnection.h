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
#ifndef ARTCPCONNECTION_H
#define ARTCPCONNECTION_H

#include "Aria/ArDeviceConnection.h"
#include <string>

#include "Aria/ariaTypedefs.h"
#include "Aria/ArSocket.h"

/// For connecting to a device through a TCP network socket
/// @ingroup UtilityClasses
class ArTcpConnection final : public ArDeviceConnection
{
 public:
  /// Constructor
  AREXPORT ArTcpConnection();
  /// Destructor also closes connection
  AREXPORT virtual ~ArTcpConnection();

    // XXX TODO add or delete copy/move constructors/operators

  /// Opens a connection to the given host and port
  AREXPORT int open(const char * host = NULL, int port = 8101);

  AREXPORT void setPort(const char *host = NULL, int port = 8101);
  AREXPORT virtual bool openSimple() override;  
  AREXPORT virtual int getStatus() override;
  AREXPORT virtual bool close() override;
// XXX TODO why is data const char*?
  AREXPORT virtual int read(const char *data, unsigned int size, 
			    unsigned int msWait = 0) override;
  AREXPORT virtual int write(const char *data, unsigned int size) override;
  AREXPORT virtual const char * getOpenMessage(int messageNumber) override;
  AREXPORT virtual ArTime getTimeRead(int index) override;
  AREXPORT virtual bool isTimeStamping() override;

  /// Gets the name of the host connected to
  AREXPORT std::string getHost();
  /// Gets the number of the port connected to
  AREXPORT int getPort();

  /// Internal function used by open and openSimple
  AREXPORT int internalOpen();

  /// Sets the tcp connection to use this socket instead of its own
  AREXPORT void setSocket(ArSocket *socket);
  /// Gets the socket this tcp connection is using
  AREXPORT ArSocket *getSocket();
  /// Sets the status of the device, ONLY use this if you're playing
  /// with setSocket and know what you're doing
  AREXPORT void setStatus(int status);

  enum Open { 
      OPEN_NET_FAIL = 1, ///< Some critical part of the network isn't working
      OPEN_BAD_HOST, ///< Could not find the host
      OPEN_NO_ROUTE, ///< Know where the host is, but can't get to it
      OPEN_CON_REFUSED ///< Got to the host but it didn't allow a connection
  };



protected:
  void buildStrMap();
  
  ArStrMap myStrMap;
  bool myOwnSocket;
  ArSocket *mySocket;
  int myStatus;
  
  std::string myHostName;
  int myPortNum;
  ArTime myTimeRead;
};

#endif //ARTCPCONNECTION_H
