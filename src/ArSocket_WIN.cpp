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
#include "ArSocket.h"
#include "ArLog.h"
#include <stdio.h>
#include <string.h>
#include "ArFunctor.h"

/*
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
*/

bool ArSocket::ourInitialized=false;


AREXPORT ArSocket::ArSocket() :
  myType(Unknown),
  myError(NoErr),
  myErrorStr(),
  myDoClose(true),
  myFD(INVALID_SOCKET),
  myHost(),
  myPort(-1),
  myNonBlocking(false),
  mySin()
{
  internalInit();
}

AREXPORT ArSocket::ArSocket(const char *host, int port, Type type) :
  myType(type),
  myError(NoErr),
  myErrorStr(),
  myDoClose(true),
  myFD(INVALID_SOCKET),
  myHost(),
  myPort(-1),
  myNonBlocking(false),
  mySin()
{
  internalInit();
  connect(host, port, type);
}

AREXPORT ArSocket::ArSocket(int port, bool doClose, Type type) :
  myType(type),
  myError(NoErr),
  myErrorStr(),
  myDoClose(doClose),
  myFD(INVALID_SOCKET),
  myHost(),
  myPort(-1),
  myNonBlocking(false),
  mySin()
{
  internalInit();
  open(port, type);
}

AREXPORT ArSocket::~ArSocket()
{
  close();
}

/** @return false failure. */
AREXPORT bool ArSocket::init()
{
  WORD wVersionRequested;
  WSADATA wsaData;

//  if (!ourInitialized)
  //{
  wVersionRequested=MAKEWORD( 2, 2 );
  
  if (WSAStartup(wVersionRequested, &wsaData) != 0)
  {
    ourInitialized=false;
    return(false);
  }
  
  ourInitialized=true;
  //}

  return(true);
}

AREXPORT void ArSocket::shutdown()
{
  if (ourInitialized)
  {
    ArLog::log(ArLog::Verbose, "ArSocket::shutdown calling WSACleanup");
    WSACleanup();
    ourInitialized=false;
  }
}

/** @return false on failure */
AREXPORT bool ArSocket::hostAddr(const char *host, struct in_addr &addr)
{
  struct hostent *hp;

  if (!(hp=gethostbyname(host)))
  {
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::hostAddr: gethostbyname failed");
    memset(&addr, 0, sizeof(in_addr));
    return(false);
  }
  else
  {
    memcpy(&addr, hp->h_addr, hp->h_length);
    return(true);
  }
}

/** @return false on failure */
AREXPORT bool ArSocket::addrHost(struct in_addr &addr, char *host)
{
  struct hostent *hp;

  hp=gethostbyaddr((char*)&addr.s_addr, sizeof(addr.s_addr), AF_INET);
  if (hp)
    strcpy(host, hp->h_name);
  else
    strcpy(host, inet_ntoa(addr));

  return(true);
}

/** @return false and set error code and description string on failure */
AREXPORT bool ArSocket::connect(const char *host, int port, Type type,
				const char *openOnIP)
{
  char localhost[MAXGETHOSTSTRUCT];
  myError = NoErr;
  myErrorStr.clear();

  init();

  if (!host)
  {
    if (gethostname(localhost, sizeof(localhost)) == 1)
    {
      myError=ConBadHost;
      myErrorStr="Failure to locate host '";
      myErrorStr+=localhost;
      myErrorStr+="'";
      ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::connect: gethostname failed");
      return(false);
    }
    host=localhost;
  }

  char useHost[1024];
  int usePort;
  separateHost(host, port, useHost, sizeof(useHost), &usePort);

  memset(&mySin, 0, sizeof(mySin));
  if ((mySin.sin_addr.s_addr = inet_addr(useHost)) == INADDR_NONE)
  {
    if (!hostAddr(host, mySin.sin_addr))
    {
      setRawIPString();
      myError = ConBadHost;
      myErrorStr = "Could not find the address of '";
      myErrorStr += host;
      myErrorStr += "'";
      return(false);
    }
  }

  mySin.sin_family=AF_INET;
  mySin.sin_port=hostToNetOrder(usePort);

  // WSA_FLAG_OVERLAPPED allows concurrent calls to select, read and send on the same socket,
  // which could happen occasionally. If OVERLAPPED is not enabled in this situation, calls can
  // hang mysteriously.
  // This flag is also required for all non-blocking sockets on Windows NT 4.0 (according to MS
  // Knowlege Base article Q179942)
  if ((type == TCP) && ((myFD=WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) < 0))
  {
    myError=NetFail;
    myErrorStr="Failure to make TCP socket";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::connect: could not make tcp socket");
    return(false);
  }
  else if ((type == UDP) && ((myFD=WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) < 0))
  {
    myError=NetFail;
    myErrorStr="Failure to make UDP socket";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::connect: could not make udp socket");
    return(0);
  }

  myType = type;
  myHost = ((host != NULL) ? host : "");
  myPort = port;

  if (::connect(myFD, (struct sockaddr *)&mySin,
		sizeof(struct sockaddr_in)) < 0)
  {
    char buff[10];
    int err=WSAGetLastError();
    sprintf(buff, "%d", err);
    myErrorStr="Failure to connect socket";
    myErrorStr+=buff;
    switch (err)
    {
    case WSAEADDRNOTAVAIL:
      myError=ConBadHost;
      break;
    case WSAECONNREFUSED:
      myError=ConRefused;
      break;
    case WSAENETUNREACH:
      myError=ConNoRoute;
      break;
    default:
      myError=NetFail;
      break;
    }
    ArLog::logErrorFromOS(ArLog::Verbose, 
			  "ArSocket::connect: Failure to connect");
    ::shutdown(myFD, SD_BOTH);
    closesocket(myFD);
    myFD = INVALID_SOCKET;
    return(0);
  }

  myLastStringReadTime.setToNow();
  return(1);
}

/** @return false and set error code and description string on failure */
AREXPORT bool ArSocket::open(int port, Type type, const char *openOnIP)
{
  int ret;
  char localhost[MAXGETHOSTSTRUCT];
  myError = NoErr;
  myErrorStr.clear();

  if ((type == TCP) && ((myFD=socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET))
  {
    //ret=WSAGetLastError();
    myError = NetFail;
    myErrorStr="Failure to make TCP socket";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::open: could not create tcp socket");
    return(false);
  }
  else if ((type == UDP) && ((myFD=socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET))
  {
    myError = NetFail;
    myErrorStr="Failure to make UDP socket";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::open: could not create udp socket");
    return(false);
  }

  setLinger(0);
  setReuseAddress();

  /* MPL this is useless withw hat I Took out below
  if (gethostname(localhost, sizeof(localhost)) == 1)
  {
    myErrorStr="Failure to locate localhost";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::open: gethostname failed");
    return(false);
  }
  */

  memset(&mySin, 0, sizeof(mySin));
  /* MPL took this out since it was just overriding it with the
     INADDR_ANY anyways and it could cause slowdowns if a machine wasn't
     configured so lookups are quick
  if (!hostAddr(localhost, mySin.sin_addr))
    return(false);
  */
  setRawIPString();
  if (openOnIP != NULL)
  {
    
    if (!hostAddr(openOnIP, mySin.sin_addr))
    {
      myError = NameLookup;
      myErrorStr = "Name lookup failed";
      ArLog::log(ArLog::Normal, "Couldn't find ip of %s to open on", openOnIP);
      ::shutdown(myFD, SD_BOTH);
      myFD = INVALID_SOCKET;
      return(false); 
    }
    else
    {
      //printf("Opening on %s\n", openOnIP);
    }
  }
  else
  {
    mySin.sin_addr.s_addr=htonl(INADDR_ANY);
  }
  mySin.sin_family=AF_INET;
  mySin.sin_port=hostToNetOrder(port);

  myType=type;

  if ((ret=bind(myFD, (struct sockaddr *)&mySin, sizeof(mySin))) < 0)
  {
    myErrorStr="Failure to bind socket to port ";
    sprintf(localhost, "%d", port);
    myErrorStr+=localhost;
    myError = NetFail;
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::open: could not bind");
    ::shutdown(myFD, SD_BOTH);
    myFD = INVALID_SOCKET;
    return(false);
  }

  if ((type == TCP) && (listen(myFD, 5) < 0))
  {
    myErrorStr="Failure to listen on socket";
    myError = NetFail;
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::open: could not listen");
    ::shutdown(myFD, SD_BOTH);
    myFD = INVALID_SOCKET;
    return(false);
  }

  return(true);
}

/** @return false and set error code and description string on failure */
AREXPORT bool ArSocket::create(Type type)
{
  myError = NoErr;
  myErrorStr.clear();
  //if ((type == TCP) && ((myFD=socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET))
  if ((type == TCP) && ((myFD=WSASocket(AF_INET, SOCK_STREAM, 0,  NULL, 0, 0)) == INVALID_SOCKET))
  {
    myErrorStr="Failure to make TCP socket";
    myError = NetFail;
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::create: could not create tcp socket");
    return(false);
  }
  //else if ((type == UDP) && ((myFD=socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET))
  else if ((type == UDP) && ((myFD=WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, 0)) == INVALID_SOCKET))
  {
    myErrorStr="Failure to make UDP socket";
    myError = NetFail;
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::create: could not create udp socket");
    return(false);
  }

  // KMC Reinserted
  myType = type;

                          

/*
  int zero = 0;
  if (setsockopt(myFD, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero)) != 0)
  {
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::create: setsockopt failed on SNDBUF");
    return(false);
  }

  if (setsockopt(myFD, SOL_SOCKET, SO_RCVBUF, (char *)&zero, sizeof(zero)) != 0)
  {
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::create: setsockopt failed on RCVVBUF");
    return(false);
  }

  myType=type;
*/
  /*if (getSockName())
    return(true);
  else
    return(false);*/
  return(true);
}

/** @return false on failure */
AREXPORT bool ArSocket::findValidPort(int startPort, const char *openOnIP)
{

  /*
  char localhost[MAXGETHOSTSTRUCT];
  if (gethostname(localhost, sizeof(localhost)) == 1)
  {
    myErrorStr="Failure to locate localhost";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::findValidPort: gethostname failed");
    return(false);
  }
  */
  for (int i=0; i+startPort < 65000; ++i)
  {
    memset(&mySin, 0, sizeof(mySin));
    /*
    if (!hostAddr(localhost, mySin.sin_addr))
      return(false);
    */
    setRawIPString();
    if (openOnIP != NULL)
    {
      if (!hostAddr(openOnIP, mySin.sin_addr))
      {
        ArLog::log(ArLog::Normal, "Couldn't find ip of %s to open on", openOnIP);
        return(false); 
      }
      else
      {
	//printf("Opening on %s\n", openOnIP);
      }
    }
    else
    {
      mySin.sin_addr.s_addr=htonl(INADDR_ANY);
    }

    mySin.sin_family=AF_INET;
    //mySin.sin_addr.s_addr=htonl(INADDR_ANY);
    mySin.sin_port=hostToNetOrder(startPort+i);

    if (bind(myFD, (struct sockaddr *)&mySin, sizeof(mySin)) == 0)
      break;
  }

  return(true);
}

/** @return false and set error code and description string on failure */
AREXPORT bool ArSocket::connectTo(const char *host, int port)
{

  char localhost[MAXGETHOSTSTRUCT];
  myError = NoErr;
  myErrorStr.clear();

  if (myFD == INVALID_SOCKET)
    return(false);

  if (!host)
  {
    if (gethostname(localhost, sizeof(localhost)) == 1)
    {
      myErrorStr="Failure to locate host '";
      myErrorStr+=localhost;
      myErrorStr+="'";
      myError = ConBadHost;
      ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::connectTo: gethostname failed");
      return(false);
    }
    host=localhost;
  }


  char useHost[1024];
  int usePort;
  separateHost(host, port, useHost, sizeof(useHost), &usePort);

  //myHost = ((host != NULL) ? host : "");
  //myPort = port;
  myHost = useHost;
  myPort = usePort;

  memset(&mySin, 0, sizeof(mySin));
  if (!hostAddr(useHost, mySin.sin_addr))
  {
    myError = ConBadHost;
    return(false);
  }
  setRawIPString();
  mySin.sin_family=AF_INET;
  mySin.sin_port=hostToNetOrder(usePort);

  myLastStringReadTime.setToNow();
  return(connectTo(&mySin));
}

/** @return false and set error code and description string on failure */
AREXPORT bool ArSocket::connectTo(struct sockaddr_in *sin)
{
  myError = NoErr;
  myErrorStr.clear();
  if (::connect(myFD, (struct sockaddr *)sin,
		sizeof(struct sockaddr_in)) < 0)
  {
    myErrorStr="Failure to connect socket";
    myError = NetFail;
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::connectTo: gethostname failed");
    return(0);
  }

  myLastStringReadTime.setToNow();
  return(1);
}
  


/** @return false and set error code and description string on failure */
AREXPORT bool ArSocket::close()
{
  myError = NoErr;
  myErrorStr.clear();

  if (myFD != INVALID_SOCKET) {
    if (!myHost.empty() && (myPort != -1)) {
      ArLog::log(ArLog::Verbose, "Closing %s socket %s:%i (0x%p)",
                 toString(myType),
                 myHost.c_str(),
                 myPort,
                 myFD);
    }
    else {
      ArLog::log(ArLog::Verbose, "Closing %s socket (0x%p)",
                 toString(myType),
                 myFD);
    }
  }

  if (myCloseFunctor != NULL)
    myCloseFunctor->invoke();

  if (myDoClose && (myFD != INVALID_SOCKET))
  {
    int shutdownRet = ::shutdown(myFD, SD_BOTH);
    if (shutdownRet != 0) {
      int error = WSAGetLastError();
      ArLog::log(ArLog::Normal, "Shutdown %s socket (0x%p) returns %i (error = %i)",
                 toString(myType),
                 myFD,
                 shutdownRet, 
                 error);
    }

    // ?? Should we do this only if shutdown return successful?
    // ?? In theory, I think that we're supposed to wait for the shutdown to complete.
    // ?? Could this be causing the intermittent hang?

    int closeRet = closesocket(myFD);
    if (closeRet  != 0) {
      int error = WSAGetLastError();
      ArLog::log(ArLog::Normal, "Close %s socket (0x%p) returns %i (error = %i)",
                 toString(myType),
                 myHost.c_str(),
                 closeRet,
                 error);
    }

    myFD = INVALID_SOCKET;

    if (!myHost.empty() && (myPort != -1)) {
      ArLog::log(ArLog::Verbose, "%s socket %s:%i closed",
                 toString(myType),
                 myHost.c_str(),
                 myPort);
    }
    else {
      ArLog::log(ArLog::Verbose, "%s socket closed",
                 toString(myType));
    }

    return(true);
  }

  ArLog::log(ArLog::Verbose, "Close %s socket requires no action",
              toString(myType));


  return(false);
}

/** @return false and set error code and description string on failure */
AREXPORT bool ArSocket::setLinger(int time)
{
  struct linger lin;
  myError = NoErr;
  myErrorStr.clear();

  if (time)
  {
    lin.l_onoff=1;
    lin.l_linger=time;
  }
  else
  {
    lin.l_onoff=0;
    lin.l_linger=time;
  }

  if (setsockopt(myFD, SOL_SOCKET, SO_LINGER, (char*)&lin, sizeof(lin)) != 0)
  {
    myErrorStr="Failure to setsockopt LINGER";
    myError = NetFail;
    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSocket::setLinger: setsockopt failed");
    return(false);
  }
  else
    return(true);
}

/** @return false and set error code and description string on failure */
AREXPORT bool ArSocket::setBroadcast()
{
  myError = NoErr;
  myErrorStr.clear();
  if (setsockopt(myFD, SOL_SOCKET, SO_BROADCAST, NULL, 0) != 0)
  {
    myError = NetFail;
    myErrorStr="Failure to setsockopt BROADCAST";
    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSocket::setBroadcast: setsockopt failed");
    return(false);
  }
  else
    return(true);
}

/** @return false and set error code and description string on failure 
    @internal
    @note ArSocket always sets the reuse-address option in open(), so calling this function is normally unneccesary.
     (This apparently needs to be done after the socket is created before
     the socket is bound.)
*/
AREXPORT bool ArSocket::setReuseAddress()
{
  int opt=1;
  myError = NoErr;
  myErrorStr.clear();
  if (setsockopt(myFD, SOL_SOCKET, SO_REUSEADDR,
		 (char*)&opt, sizeof(opt)) != 0)
  {
    myErrorStr="Failure to setsockopt REUSEADDR";
    myError = NetFail;
    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSocket::setReuseAddress: setsockopt failed");
    return(false);
  }
  else
    return(true);
}

/** @return false and set error code and description string on failure  */
AREXPORT bool ArSocket::setNonBlock()
{
  u_long arg=1;
  myError = NoErr;
  myErrorStr.clear();
  if (ioctlsocket(myFD, FIONBIO, &arg) != 0)
  {
    myErrorStr="Failure to fcntl O_NONBLOCK";
    myError = NetFail;
    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSocket::setNonBlock: fcntl failed");
    return(false);
  }
  else
  {
    myNonBlocking = true;
    return(true);
  }
}

/** @return false and set error code and description string on failure  */
AREXPORT bool ArSocket::copy(int fd, bool doclose)
{
  int len;

  myFD=fd;
  myDoClose=doclose;
  myError = NoErr;
  myErrorStr.clear();

  len=sizeof(struct sockaddr_in);
  if (getsockname(myFD, (struct sockaddr*)&mySin, &len))
  {
    myErrorStr="Failed to getsockname on fd ";
    myError = NetFail;
    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSocket::copy: getsockname failed");
    return(false);
  }
  else
    return(true);
}

/** @return false and set error code and description string on failure  */
AREXPORT bool ArSocket::accept(ArSocket *sock)
{
  int len;
  //unsigned char *bytes;
  myError = NoErr;
  myErrorStr.clear(); 
  len=sizeof(struct sockaddr_in);
  sock->myFD=::accept(myFD, (struct sockaddr*)&(sock->mySin), &len);
  sock->myType=myType;
  sock->setRawIPString();
  /*
  bytes = (unsigned char *)sock->inAddr();
  sprintf(sock->myIPString, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], 
	  bytes[3]);
  */
  if ((sock->myFD < 0 && !myNonBlocking) || 
      (sock->myFD < 0 && WSAGetLastError() != WSAEWOULDBLOCK && myNonBlocking))
  {
    myErrorStr="Failed to accept on socket";
    myError = ConRefused;
    ArLog::logErrorFromOS(ArLog::Terse, 
			  "ArSocket::accept: accept failed");
    return(false);
  }

  return(true);
}

AREXPORT void ArSocket::inToA(struct in_addr *addr, char *buff)
{
  strcpy(buff, inet_ntoa(*addr));
}

/** @return false and set error code and description string on failure  */
AREXPORT bool ArSocket::getSockName()
{
  int size;

  myError = NoErr;
  myErrorStr.clear();
  if (myFD == INVALID_SOCKET)
  {
    myErrorStr="Trying to get socket name on an unopened socket";
    myError = NetFail;
    printf(myErrorStr.c_str());
    return(false);
  }

  size=sizeof(mySin);
  if (getsockname(myFD, (struct sockaddr *)&mySin, &size) != 0)
  {
    myErrorStr="Error getting socket name";
    switch (WSAGetLastError())
    {
    case WSAEINVAL:
      myErrorStr+=": inval";
      break;
    case WSANOTINITIALISED:
      myErrorStr+=": not init";
      break;
    }
    myError = NetFail;
    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSocket::getSockName: getSockName failed");
    return(false);
  }

  return(true);
}

AREXPORT unsigned int ArSocket::hostToNetOrder(int i)
{
  return(htons(i));
}

AREXPORT unsigned int ArSocket::netToHostOrder(int i)
{
  return(ntohs(i));
}

AREXPORT std::string ArSocket::getHostName()
{
  char localhost[MAXGETHOSTSTRUCT];

  if (gethostname(localhost, sizeof(localhost)) == 1)
    return("");
  else
    return(localhost);
}

/** If this socket is a TCP socket, then set the TCP_NODELAY flag to 1,
 *  to disable the use of the Nagle algorithm (which waits until enough
 *  data is ready to send to fill a TCP frame, rather then sending the
 *  packet immediately).
 *  @return true of the flag was successfully set, false if there was an 
 *    error or this socket is not a TCP socket.
 */
AREXPORT bool ArSocket::setNoDelay(bool flag)
{
  if(myType != TCP) return false;
  int f = flag?1:0;
  int r = setsockopt(myFD, IPPROTO_TCP, TCP_NODELAY, (char*)&f, sizeof(f));
  return (r != -1);
}

