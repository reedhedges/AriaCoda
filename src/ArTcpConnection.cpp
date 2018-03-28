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
#include "ArTcpConnection.h"
#include "ArLog.h"
#include "ariaUtil.h"

AREXPORT ArTcpConnection::ArTcpConnection()
{
  myStatus = STATUS_NEVER_OPENED;
  buildStrMap();
  myOwnSocket = true;
  mySocket = new ArSocket();
  setPortType("tcp");
}

AREXPORT ArTcpConnection::~ArTcpConnection()
{
  if (myOwnSocket)
    delete mySocket;
}

/**
   This will make the connection use this socket, its useful for doing
   funkier things with sockets but still being able to use a device
   connection.
   @param socket the socket to use
**/
AREXPORT void ArTcpConnection::setSocket(ArSocket *socket)
{
  if (myOwnSocket)
  {
    delete mySocket;
    myOwnSocket = false;
  }
  mySocket = socket;
}

AREXPORT ArSocket *ArTcpConnection::getSocket(void)
{
  return mySocket;
}

AREXPORT void ArTcpConnection::setStatus(int status)
{
  myStatus = status;
}

AREXPORT void ArTcpConnection::setPort(const char *host, int port)
{
  myPortNum = port;

  if (host == NULL)
    myHostName = "localhost";
  else
    myHostName = host;

  char portBuf[1024];
  sprintf(portBuf, "%d", port); 
  
  std::string portName;
  portName = myHostName;
  portName += ":";
  portName += portBuf;
  setPortName(portName.c_str());
}

AREXPORT bool ArTcpConnection::openSimple(void)
{
  if (internalOpen() == 0)
    return true;
  else
    return false;
}

/**
   @param host the hostname or IP address to connect to, if NULL (default) then localhost
   @param port the port to connect to. (the default port, 8101, is the TCP port
number used by the wireless bridge device and by the simulator for robot
connection)
   @return 0 for success, otherwise one of the open enums
   @see getOpenMessage
*/
AREXPORT int ArTcpConnection::open(const char *host, int port)
{
  setPort(host, port);
  return internalOpen();
}

AREXPORT int ArTcpConnection::internalOpen(void)
{
  mySocket->init();

  ArLog::log(ArLog::Verbose, "ArTcpConnection::internalOpen: Connecting to %s %d", myHostName.c_str(), myPortNum);

  if (mySocket->connect(const_cast<char *>(myHostName.c_str()), myPortNum,
		       ArSocket::TCP)) 
  {
    myStatus = STATUS_OPEN;
    mySocket->setNonBlock();
    mySocket->setNoDelay(true);
    return 0;
  }
  
  myStatus = STATUS_OPEN_FAILED;
  switch(mySocket->getError())
  {
  case ArSocket::NetFail:
    return OPEN_NET_FAIL;
  case ArSocket::ConBadHost:
    return OPEN_BAD_HOST;
  case ArSocket::ConNoRoute:
    return OPEN_NO_ROUTE;
  case ArSocket::ConRefused:
    return OPEN_CON_REFUSED;
  case ArSocket::NoErr:
    ArLog::log(ArLog::Terse, "ArTcpConnection::open: No error!\n");
  default:
    return -1;
  }

}

void ArTcpConnection::buildStrMap(void)
{
  myStrMap[OPEN_NET_FAIL] = "Network failed.";
  myStrMap[OPEN_BAD_HOST] = "Could not find host.";
  myStrMap[OPEN_NO_ROUTE] = "No route to host.";
  myStrMap[OPEN_CON_REFUSED] = "Connection refused.";
}

AREXPORT const char *ArTcpConnection::getOpenMessage(int messageNumber)
{
  return myStrMap[messageNumber].c_str();
}

AREXPORT bool ArTcpConnection::close(void)
{
  myStatus = STATUS_CLOSED_NORMALLY;
  return mySocket->close();
}

AREXPORT int ArTcpConnection::read(const char *data, unsigned int size, 
				   unsigned int msWait)
{
  ArTime timeDone;
  unsigned int bytesRead = 0;
  int n;

  if (getStatus() != STATUS_OPEN) 
  {
    ArLog::log(ArLog::Terse, 
	       "ArTcpConnection::read: Attempt to use port that is not open.");
    return -1;
  }


  int timeToWait;  
  timeDone.setToNow();
  if (!timeDone.addMSec(msWait)) {
    ArLog::log(ArLog::Normal,
               "ArTcpConnection::read() error adding msecs (%i)",
               msWait);
  }

  do 
  {
    timeToWait = timeDone.mSecTo();
    if (timeToWait < 0)
      timeToWait = 0;
    // if the sockets empty don't read it, but pause some
    if (mySocket->getFD() < 0)
    {
      ArLog::log(ArLog::Terse, 
		 "ArTcpConnection::read: Attempt to read port that already closed. (%d)", timeToWait);
      if (timeToWait > 0)
	ArUtil::sleep(timeToWait);
      return -1;
    }

    n = mySocket->read(const_cast<char *>(data) + bytesRead, size - bytesRead,
		       timeToWait);
    /*if (n == -1) 
    {
      ArLog::log("ArTcpConnection::read: read failed.");
      return -1;
      } */
    //printf("%ld %d %d\n", timeDone.mSecTo(), n, size);
    if (n != -1)
      bytesRead += n;
    if (bytesRead >= size)
      return bytesRead;
  } while (timeDone.mSecTo() >= 0);

  return bytesRead;
}

AREXPORT int ArTcpConnection::write(const char *data, unsigned int size)
{
  int ret;

  if (getStatus() != STATUS_OPEN) 
  {
    ArLog::log(ArLog::Terse, 
	       "ArTcpConnection::write: Attempt to use port that is not open.");
    return -1;
  }
  if ((ret = mySocket->write(data, size)) != -1)
    return ret;

  ArLog::log(ArLog::Terse, "ArTcpConnection::write: Write failed, closing connection.");
  close();
  return -1;
}


/** 
    @return the name of the host connected to
    @see getPort
*/
AREXPORT std::string ArTcpConnection::getHost(void)
{
  return myHostName;
}

/**
   @return the number of the port connected to
   @see getHost
*/
AREXPORT int ArTcpConnection::getPort(void)
{
  return myPortNum;
}

AREXPORT int ArTcpConnection::getStatus(void)
{
  return myStatus;
}

AREXPORT bool ArTcpConnection::isTimeStamping(void)
{
  return false;
}

AREXPORT ArTime ArTcpConnection::getTimeRead(int index)
{
  ArTime now;
  now.setToNow();
  return now;
}
