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
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "ArFunctor.h"
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

/// We're always initialized in Linux
bool ArSocket::ourInitialized=true;


/**
   In Windows, the networking subsystem needs to be initialized and shutdown
   individyaly by each program. So when a program starts they will need to
   call the static function ArSocket::init() and call ArSocket::shutdown()
   when it exits. For programs that use Aria::init() and Aria::uninit()
   calling the ArSocket::init() and ArSocket::shutdown() is unnecessary. The
   Aria initialization functions take care of this. These functions do nothing
   in Linux.
*/
bool ArSocket::init()
{
  return(true);
}

/**
   In Windows, the networking subsystem needs to be initialized and shutdown
   individyaly by each program. So when a program starts they will need to
   call the static function ArSocket::init() and call ArSocket::shutdown()
   when it exits. For programs that use Aria::init() and Aria::uninit()
   calling the ArSocket::init() and ArSocket::shutdown() is unnecessary. The
   Aria initialization functions take care of this. These functions do nothing
   in Linux.
*/
void ArSocket::shutdown()
{
}

ArSocket::ArSocket() :
  myType(Unknown),
  myError(NoErr),
  myErrorStr(),
  myDoClose(true),
  myFD(-1),
  myNonBlocking(false),
  mySin()
{
  internalInit();
}

/**
   Constructs the socket and connects it to the given host.
   @param host hostname of the server to connect to
   @param port port number of the server to connect to
   @param type protocol type to use
*/
ArSocket::ArSocket(const char *host, int port, Type type) :
  myType(type),
  myError(NoErr),
  myErrorStr(),
  myDoClose(true),
  myFD(-1),
  myNonBlocking(false),
  mySin()
{
  internalInit();
  connect(host, port, type);
}

ArSocket::ArSocket(int port, bool doClose, Type type) :
  myType(type),
  myError(NoErr),
  myErrorStr(),
  myDoClose(doClose),
  myFD(-1),
  myNonBlocking(false),
  mySin()
{
  internalInit();
  open(port, type);
}

ArSocket::~ArSocket()
{
  close();
}

bool ArSocket::hostAddr(const char *host, struct in_addr &addr)
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
    bcopy(hp->h_addr, &addr, hp->h_length);
    return(true);
  }
}

bool ArSocket::addrHost(struct in_addr &addr, char *host)
{
  struct hostent *hp;

  hp=gethostbyaddr((char*)&addr.s_addr, sizeof(addr.s_addr), AF_INET);
  if (hp)
    strcpy(host, hp->h_name);
  else
    strcpy(host, inet_ntoa(addr));

  return(true);
}

std::string ArSocket::getHostName()
{
  char localhost[maxHostNameLen()];

  if (gethostname(localhost, sizeof(localhost)) == 1)
    return("");
  else
    return(localhost);
}

/** @return false and set error code and description string on failure  */
bool ArSocket::connect(const char *host, int port, Type type,
		       const char *openOnIP)
{
  char localhost[maxHostNameLen()];
  myError=NoErr;
  myErrorStr.clear();
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

  bzero(&mySin, sizeof(mySin));
  // MPL taking out this next code line from the if since it makes
  // everything we can't resolve try to connect to localhost
  // &&  !hostAddr("localhost", mySin.sin_addr))

  char useHost[1024];
  int usePort;
  separateHost(host, port, useHost, sizeof(useHost), &usePort);

  if (!hostAddr(useHost, mySin.sin_addr))
    return(false);
  setRawIPString();
  mySin.sin_family=AF_INET;
  mySin.sin_port=hostToNetOrder(usePort);

  if ((type == TCP) && ((myFD=socket(AF_INET, SOCK_STREAM, 0)) < 0))
  {
    myError=NetFail;
    myErrorStr="Failure to make TCP socket";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::connect: could not make tcp socket");
    return(false);
  }
  else if ((type == UDP) && ((myFD=socket(AF_INET, SOCK_DGRAM, 0)) < 0))
  {
    myError=NetFail;
    myErrorStr="Failure to make UDP socket";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::connect: could not make udp socket");
    return(false);
  }

  ArUtil::setFileCloseOnExec(myFD);

  if (openOnIP != NULL)
  {
    struct sockaddr_in outSin;
    if (!hostAddr(openOnIP, outSin.sin_addr))
    {
      myError = NameLookup;
      myErrorStr = "Name lookup failed";
      ArLog::log(ArLog::Normal, "Couldn't find ip of %s to open on", openOnIP);
      return(false); 
    }
    outSin.sin_family=AF_INET;
    outSin.sin_port=hostToNetOrder(0);
    if (bind(myFD, (struct sockaddr *)&outSin, sizeof(outSin)) < 0)
    {
      ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::connect: Failure to bind socket to port %d", 0);
      return(false);
    }
  }

  myType=type;

  if (::connect(myFD, (struct sockaddr *)&mySin,
		sizeof(struct sockaddr_in)) < 0)
  {
    myErrorStr="Failure to connect socket";
    switch (errno)
    {
    case ECONNREFUSED:
      myError=ConRefused;
      myErrorStr+="; Connection refused";
      break;
    case ENETUNREACH:
      myError=ConNoRoute;
      myErrorStr+="; No route to host";
      break;
    default:
      myError=NetFail;
      break;
    }
    ArLog::logErrorFromOS(ArLog::Verbose, "ArSocket::connect: could not connect");

    ::close(myFD);
    myFD = -1;
    return(false);
  }

  return(true);
}

/** @return false and set error code and description string on failure  */
bool ArSocket::open(int port, Type type, const char *openOnIP)
{
  int ret;
  char localhost[maxHostNameLen()];

  myError=NoErr;
  myErrorStr.clear();
  if ((type == TCP) && ((myFD=socket(AF_INET, SOCK_STREAM, 0)) < 0))
  {
    myErrorStr="Failure to make TCP socket";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::open: could not create tcp socket");
    return(false);
  }
  else if ((type == UDP) && ((myFD=socket(AF_INET, SOCK_DGRAM, 0)) < 0))
  {
    myErrorStr="Failure to make UDP socket";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::open: could not create udp socket");
    return(false);
  }

  ArUtil::setFileCloseOnExec(myFD);

  setLinger(0);
  setReuseAddress();

  myType=type;

  /* MPL removed this since with what I Took out down below months ago 
  if (gethostname(localhost, sizeof(localhost)) == 1)
  {
    myErrorStr="Failure to locate localhost";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::open: gethostname failed");
    return(false);
  }
  */
  bzero(&mySin, sizeof(mySin));
  /* MPL took this out since it was just overriding it with the
     INADDR_ANY anyways and it could cause slowdowns if a machine wasn't
     configured so lookups are quick
  if (!hostAddr(localhost, mySin.sin_addr) && 
      !hostAddr("localhost", mySin.sin_addr))
    return(false); */

  if (openOnIP != NULL)
  {
    
    if (!hostAddr(openOnIP, mySin.sin_addr))
    {
      ArLog::log(ArLog::Normal, "Couldn't find ip of %s to open on", openOnIP);
      myError = NameLookup;
      myErrorStr = "Name lookup failed";
      ::close(myFD);
      myFD = -1;
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

  setRawIPString();
  mySin.sin_family=AF_INET;
  mySin.sin_port=hostToNetOrder(port);

  if ((ret=bind(myFD, (struct sockaddr *)&mySin, sizeof(mySin))) < 0)
  {
    myError = NetFail;
    myErrorStr="Failure to bind socket to port ";
    sprintf(localhost, "%d", port);
    myErrorStr+=localhost;
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::open: could not bind");
    ::close(myFD);
    myFD = -1;
    return(false);
  }

  if ((type == TCP) && (listen(myFD, 5) < 0))
  {
    myError = NetFail;
    myErrorStr="Failure to listen on socket";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::open: could not listen");
    ::close(myFD);
    myFD = -1;
    return(false);
  }

  myLastStringReadTime.setToNow();
  return(true);
}

/** @return false and set error code and description string on failure  */
bool ArSocket::create(Type type)
{
  myError = NoErr;
  myErrorStr.clear();

  if ((type == TCP) && ((myFD=socket(AF_INET, SOCK_STREAM, 0)) < 0))
  {
    myError = NetFail;
    myErrorStr="Failure to make TCP socket";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::create: could not create tcp socket");
    return(false);
  }
  else if ((type == UDP) && ((myFD=socket(AF_INET, SOCK_DGRAM, 0)) < 0))
  {
    myError = NetFail;
    myErrorStr="Failure to make UDP socket";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::create: could not create udp socket");
    return(false);
  }

  ArUtil::setFileCloseOnExec(myFD);

  myType=type;

  if (getSockName())
    return(true);
  else
    return(false);
}

/** @return false on error */
bool ArSocket::findValidPort(int startPort, const char *openOnIP)
{
  //char localhost[maxHostNameLen()];

  /*
  if (gethostname(localhost, sizeof(localhost)) == 1)
  {
    myErrorStr="Failure to locate localhost";
    ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::findValidPort: gethostname failed");
    return(false);
  }
  */

  for (int i=0; i+startPort < 65000; ++i)
  {
    bzero(&mySin, sizeof(mySin));
    /*
    if (!hostAddr(localhost, mySin.sin_addr) && 
	!hostAddr("localhost", mySin.sin_addr))
      return(false);
    */
    setRawIPString();
    
    if (openOnIP != NULL)
    {
      
      if (!hostAddr(openOnIP, mySin.sin_addr))
      {
	ArLog::log(ArLog::Normal, "Couldn't find ip of %s to open udp on", openOnIP);
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
    mySin.sin_port=hostToNetOrder(startPort+i);

    if (bind(myFD, (struct sockaddr *)&mySin, sizeof(mySin)) == 0)
      break;
  }

  return(true);
}

/** @return false and set error code and description string on failure */
bool ArSocket::connectTo(const char *host, int port)
{
  char localhost[maxHostNameLen()];
  myError = NoErr;
  myErrorStr.clear();
  if (myFD < 0)
    return(false);

  if (!host)
  {
    if (gethostname(localhost, sizeof(localhost)) == 1)
    {
      myErrorStr="Failure to locate host '";
      myErrorStr+=localhost;
      myErrorStr+="'";
      ArLog::logErrorFromOS(ArLog::Normal, "ArSocket::connectTo: gethostname failed");
      return(false);
    }
    host=localhost;
  }

  char useHost[1024];
  int usePort;
  separateHost(host, port, useHost, sizeof(useHost), &usePort);

  bzero(&mySin, sizeof(mySin));
  if (!hostAddr(useHost, mySin.sin_addr))
    return(false);
  setRawIPString();
  mySin.sin_family=AF_INET;
  mySin.sin_port=hostToNetOrder(usePort);

  myLastStringReadTime.setToNow();
  return(connectTo(&mySin));
}

/** @return false and set error code and description string on failure */
bool ArSocket::connectTo(struct sockaddr_in *sin)
{
  myError = NoErr;
  myErrorStr.clear();
  if (::connect(myFD, (struct sockaddr *)sin,
		sizeof(struct sockaddr_in)) < 0)
  {
    myErrorStr="Failure to connect socket";
    myError = ConRefused;
    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSocket::connectTo: connect failed");
    return(0);
  }

  myLastStringReadTime.setToNow();
  return(1);
}


bool ArSocket::close()
{
  if (myFD == -1)
    return true;
  ArLog::log(ArLog::Verbose, "Closing socket");
  if (myCloseFunctor != NULL)
    myCloseFunctor->invoke();
  if (myDoClose && ::close(myFD))
  {
    myFD=-1;
    return(false);
  }
  else
  {
    myFD=-1;
    return(true);
  }
}

/** @return false and set error code and description string on failure. */
bool ArSocket::setLinger(int time)
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

  if (setsockopt(myFD, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin)) != 0)
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

/** @return false and set error code and description string on failure. */
bool ArSocket::setBroadcast()
{
  myError = NoErr;
  myErrorStr.clear();
  if (setsockopt(myFD, SOL_SOCKET, SO_BROADCAST, NULL, 0) != 0)
  {
    myErrorStr="Failure to setsockopt BROADCAST";
    myError = NetFail;
    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSocket::setBroadcast: setsockopt failed");
    return(false);
  }
  else
    return(true);
}

/** @return false and set error code and description string on failure. 
    @internal
    @note ArSocket always sets the reuse-address option in open(), so calling this function is normally unneccesary.
     (This apparently needs to be done after the socket is created before
     the socket is bound.)
*/
bool ArSocket::setReuseAddress()
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

/** @return false and set error code and description string on failure.  */
bool ArSocket::setNonBlock()
{
  myError = NoErr;
  myErrorStr.clear();
  if (fcntl(myFD, F_SETFL, O_NONBLOCK) != 0)
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

/**
   Copy socket structures. Copy from one Socket to another will still have
   the first socket close the file descripter when it is destructed.
 @return false and set error code and description string on failure.  
*/
bool ArSocket::copy(int fd, bool doclose)
{
  socklen_t len;

  myFD=fd;
  myDoClose=doclose;
  myType=Unknown;

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

/**
   @return true if there are no errors, false if there are
   errors... not that if you're in non-blocking mode and there is no
   socket to connect that is NOT an error, you'll want to check the
   getFD on the sock you pass in to see if it is actually a valid
   socket.
 **/
bool ArSocket::accept(ArSocket *sock)
{
  socklen_t len;
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
      (sock->myFD < 0 && errno != EWOULDBLOCK && myNonBlocking))
  {
    myErrorStr="Failed to accept on socket";
    myError = ConRefused;
    ArLog::logErrorFromOS(ArLog::Terse, 
			  "ArSocket::accept: accept failed");
    return(false);
  }

  return(true);
}

void ArSocket::inToA(struct in_addr *addr, char *buff)
{
  strcpy(buff, inet_ntoa(*addr));
}

bool ArSocket::getSockName()
{
  socklen_t size;
  myError = NoErr;
  myErrorStr.clear();
  if (myFD < 0)
  {
    myErrorStr="Trying to get socket name on an unopened socket";
    myError = NetFail;
    ArLog::log(ArLog::Verbose, "ArSocket: Internal Error: Could not get socket name on unopened socket.");
    return(false);
  }

  size=sizeof(mySin);
  if (getsockname(myFD, (struct sockaddr *)&mySin, &size) != 0)
  {
    myErrorStr="Error getting socket name";
    myError = NetFail;
    ArLog::logErrorFromOS(ArLog::Normal, 
			  "ArSocket::getSockName: Internal Error: getsockname() failed");
    return(false);
  }

  return(true);
}

unsigned int ArSocket::hostToNetOrder(int i)
{
  return(htons(i));
}

unsigned int ArSocket::netToHostOrder(int i)
{
  return(ntohs(i));
}

/** If this socket is a TCP socket, then set the TCP_NODELAY flag,
 *  to disable the use of the Nagle algorithm (which waits until enough
 *  data is ready to send to fill a TCP frame, rather then sending the
 *  packet immediately).
 *  @param flag true to turn on NoDelay, false to turn it off.
 *  @return true of the flag was successfully set, false if there was an 
 *    error or this socket is not a TCP socket.
 */
bool ArSocket::setNoDelay(bool flag)
{
  if(myType != TCP) return false;
  int f = flag?1:0;
  int r = setsockopt(myFD, IPPROTO_TCP, TCP_NODELAY, (char*)&f, sizeof(f));
  return (r != -1);
}

