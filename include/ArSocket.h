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
#ifndef ARSOCKET_H
#define ARSOCKET_H


#ifndef WIN32
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#endif


#include <string>
#include "ariaTypedefs.h"
#include "ArMutex.h"
#include "ariaUtil.h"

class ArFunctor;


/// socket communication wrapper
/**
   ArSocket is a layer which allows you to use the sockets networking
   interface in an operating system independent manner. All of the standard
   commonly used socket functions are implemented such as open(), close(),
   connect(), accept(), read(), write(), hostToNetOrder(), netToHostOrder().  ArSocket extends some of these functions to set useful options (see method documentation for details). It also provides additional useful functions like
   writeString(), readString, setCloseCallback(), and more.

   In Windows, the sockets subsystem needs to be initialized and shutdown
   by the program. So when a program starts it must call Aria::init() and
   call Aria::shutdown() when it exits. (Or, to only initialize the socket
   system, and not do any other global Aria initialization, use ArSocket::init()
   and ArSocket::shutdown().)

   Some calls set an error code on failure in addition to returning false. This value is available from getError().
   If getError() returns something other than NoErr, a text description of the error may be available from getErrorStr().  

   @sa @ref socketServerExample.cpp
   @sa @ref socketClientExample.cpp

    @ingroup UtilityClasses
*/
class ArSocket
{
public:

  enum Type {UDP, TCP, Unknown};
  enum Error {NoErr, NetFail, ConBadHost, ConNoRoute, ConRefused, NameLookup};

  /// Constructor. You must then use either connect() or open().
  AREXPORT ArSocket();

  /// Constructor which immediately connects to a server as a client
  /// @a host Hostname or IP address of remote server
  /// @a port Port number on server
  /// @a Which IP protocol to use, type ArSocket::TCP or ArSocket::UDP
  AREXPORT ArSocket(const char *host, int port, Type type);

  /// Constructor which immediately opens a port as a server
  /// @a port Port number to open. Use a value greater than 1024.
  /// @a doClose Automatically close the port when ArSocket is destroyed (recommend using true)
  /// @a type Which IP protocol to use, ArSocket::TCP or ArSocket::UDP
  AREXPORT ArSocket(int port, bool doClose, Type type);

  /// Destructor
  AREXPORT ~ArSocket();

  /// Initialize the OS sockets system, if neccesary
  AREXPORT static bool init();

  /// Shutdown the OS sockets system, if neccesary
  AREXPORT static void shutdown();

  /// Converts the given socket type to a displayable text string (for debugging).
  AREXPORT static const char *toString(Type t);

  /** @internal */
  AREXPORT static bool ourInitialized;

  /// Copy internal socket structures
  /// @internal
  AREXPORT bool copy(int fd, bool doclose);

  /// Copy socket 
  /// @internal
  AREXPORT void copy(ArSocket *s)
    {myFD=s->myFD; myDoClose=false; mySin=s->mySin;}

  /// Transfer ownership of a socket
  /** transfer() will transfer ownership to this socket. The input socket
      will no longer close the file descriptor when it is destructed.
  */
  AREXPORT void transfer(ArSocket *s)
    {myFD=s->myFD; myDoClose=true; s->myDoClose=false; mySin=s->mySin;
      myType=s->myType; strcpy(myRawIPString, s->myRawIPString); 
      setIPString(s->getIPString()); }

  /// Connect as a client to a server
  AREXPORT bool connect(const char *host, int port, Type type = TCP,
			const char *openOnIP = NULL);

  /** Open a server port

      Opens a server port, that is, a port that is bound to a local port (and optionally, address) 
      and listens for new incoming connections.  Use accept() to wait for a
      new incoming connection from a client.

      In additon, internally this method calls setLinger(0), setReuseAddress(),
      and setNonBlock() to turn on these options (having these on is 
      particularly useful for servers).  
      
      @param port Port number
      @param type ArSocket::TCP or ArSocket::UDP.
      @param openOnIP If given, only bind to the interface
        accociated with this address (Linux only) (by default, all interfaces are used)
  */
  AREXPORT bool open(int port, Type type, const char *openOnIP = NULL);

  /// Simply create a port.
  AREXPORT bool create(Type type);

  /** Find the first valid unused port after @a startPort, and bind the socket to it.
      @param startPort first port to try
	    @param openOnIP If given, only check ports open on the interface accociated with this address (Linux only)
  */
  AREXPORT bool findValidPort(int startPort, const char *openOnIP = NULL);

  /// Connect the socket to the given address
  AREXPORT bool connectTo(const char *host, int port);

  /// Connect the socket to the given address
  AREXPORT bool connectTo(struct sockaddr_in *sin);

  /// Accept a new connection
  AREXPORT bool accept(ArSocket *sock);

  /// Close the socket
  AREXPORT bool close();

  /// Write data to the socket
  AREXPORT int write(const void *buff, size_t len);

  /// Read data from the socket
  AREXPORT int read(void *buff, size_t len, unsigned int msWait = 0);

  /// Send a message (short string) on the socket
  AREXPORT int sendTo(const void *msg, int len);

  /// Send a message (short string) on the socket
  AREXPORT int sendTo(const void *msg, int len, struct sockaddr_in *sin);

  /// Receive a message (short string) from the socket
  AREXPORT int recvFrom(void *msg, int len, sockaddr_in *sin);

  /// Convert a hostname string to an address structure
  AREXPORT static bool hostAddr(const char *host, struct in_addr &addr);

  /// Convert an address structure to a hostname string
  AREXPORT static bool addrHost(struct in_addr &addr, char *host);

  /// Get the localhost address
  AREXPORT static std::string getHostName();

  /// Get socket information (socket "name"). Aspects of this "name" are accessible with sockAddrIn(), inAddr(), inPort() 
  AREXPORT bool getSockName();

  /// Accessor for the sockaddr
  struct sockaddr_in * sockAddrIn() {return(&mySin);}

  /// Accessor for the in_addr part of sockaddr
  struct in_addr * inAddr() {return(&mySin.sin_addr);}

  /// Accessor for the port of the sockaddr
  unsigned short int inPort() {return(mySin.sin_port);}

  /// Convert @a addr into string numerical address
  AREXPORT static void inToA(struct in_addr *addr, char *buff);

  /// Size of the sockaddr
  static size_t sockAddrLen() {return(sizeof(struct sockaddr_in));}

#ifdef WIN32
  /// Max host name length
  static size_t maxHostNameLen() {return(MAXGETHOSTSTRUCT);}
#else
  /// Max host name length
  static size_t maxHostNameLen() {return(MAXHOSTNAMELEN);}
#endif

  /// Convert an int from host byte order to network byte order
  AREXPORT static unsigned int hostToNetOrder(int i);

  /// Convert an int from network byte order to host byte order
  AREXPORT static unsigned int netToHostOrder(int i);

  /// Set the linger value
  AREXPORT bool setLinger(int time);

  /// Set broadcast value
  AREXPORT bool setBroadcast();

  /// Set the reuse address value
  AREXPORT bool setReuseAddress();

  /// Set socket to nonblocking.  Most importantly, calls to read() will return immediately, even if no data is available.
  AREXPORT bool setNonBlock();

  /// Change the doClose value
  AREXPORT void setDoClose(bool yesno) {myDoClose=yesno;}

  /// Set if we're faking writes or not
  AREXPORT void setFakeWrites(bool fakeWrites) {myFakeWrites=fakeWrites;}

  /// Get the file descriptor
  AREXPORT int getFD() const {return(myFD);}

  /// Get the protocol type
  AREXPORT Type getType() const {return(myType);}

  /// Get a string containing a description of the last error. Only valid if getError() does not return NoErr.
  AREXPORT const std::string & getErrorStr() const {return(myErrorStr);}

  /// Get a code representing the last error
  AREXPORT Error getError() const {return(myError);}

  /// Sets whether we're  error tracking or not
  AREXPORT void setErrorTracking(bool errorTracking)
    { myErrorTracking = errorTracking; }

  /// Gets whether we're doing error tracking or not
  AREXPORT bool getErrorTracking(void) { return myErrorTracking; }

  /// Gets if we've had a bad write (you have to use error tracking for this)
  AREXPORT bool getBadWrite(void) const { return myBadWrite; }

  /// Gets if we've had a bad read (you have to use error tracking for this)
  AREXPORT bool getBadRead(void) const { return myBadRead; }


#ifndef SWIG
  /** @brief Writes a string to the socket (adding end of line characters)
   *  @swigomit
   *  @sa writeStringPlain()
   */
  AREXPORT int writeString(const char *str, ...);
#endif

  /// Same as writeString(), but no varargs 
  AREXPORT int writeStringPlain(const char *str) { return writeString(str); }

  /// Reads a string from the socket
  AREXPORT char *readString(unsigned int msWait = 0);
  /// Whether to ignore carriage return characters in readString or not
  AREXPORT void setReadStringIgnoreReturn(bool ignore)
    { myStringIgnoreReturn = ignore; }
  /// Clears the partial string read
  AREXPORT void clearPartialReadString(void);
  /// Compares a string against what was partially read
  AREXPORT int comparePartialReadString(const char *partialString);
  /// Gets the time we last successfully read a string from the socket
  AREXPORT ArTime getLastStringReadTime(void) { return myLastStringReadTime; }
  /// Sets echoing on the readString calls this socket does
  AREXPORT void setEcho(bool echo) 
  { myStringAutoEcho = false; myStringEcho = echo; }
  /// Gets if we are echoing on the readString calls this socket does
  AREXPORT bool getEcho(void) { return myStringEcho; }
  /// Sets whether we log the writeStrings or not
  AREXPORT void setLogWriteStrings(bool logWriteStrings) 
    { myLogWriteStrings = logWriteStrings; }
  /// Gets whether we log the writeStrings or not
  AREXPORT bool getLogWriteStrings(void) { return myLogWriteStrings; }
  /// Sets whether we use the wrong (legacy) end chars or not
  AREXPORT void setStringUseWrongEndChars(bool useWrongEndChars) 
    { myStringWrongEndChars = useWrongEndChars; }
  /// Gets whether we log the writeStrings or not
  AREXPORT bool getStringUseWrongEndChars(void) 
    { return myStringWrongEndChars; }
  /// Gets the raw ip number as a string
  AREXPORT const char *getRawIPString(void) const { return myRawIPString; }
  /// Gets the ip number as a string (this can be modified though)
  AREXPORT const char *getIPString(void) const { return myIPString.c_str(); }
  /// Sets the ip string
  AREXPORT void setIPString(const char *ipString) 
    { if (ipString != NULL) myIPString = ipString; else myIPString = ""; }
  /// Sets the callback for when the socket is closed (nicely or harshly)
  AREXPORT void setCloseCallback(ArFunctor *functor) 
    { myCloseFunctor = functor; }
  /// Sets the callback for when the socket is closed (nicely or harshly)
  AREXPORT ArFunctor *getCloseCallback(void) { return myCloseFunctor; }
  /// Gets the number of writes we've done
  long getSends(void) { return mySends; }
  /// Gets the number of bytes we've written
  long getBytesSent(void) { return myBytesSent; }
  /// Gets the number of reads we've done
  long getRecvs(void) { return myRecvs; }
  /// Gets the number of bytes we've read
  long getBytesRecvd(void) { return myBytesRecvd; }
  /// Resets the tracking information on the socket
  void resetTracking(void) 
    { mySends = 0; myBytesSent = 0; myRecvs = 0; myBytesRecvd = 0; }
  
  /// Sets NODELAY option on TCP socket, which can reduce latency for small packet sizes.
  AREXPORT bool setNoDelay(bool flag);
  bool isOpen() { return myFD > 0; }
protected:
  /// Sets the ip string
  /// internal function that sets the ip string from the inAddr
  void setRawIPString(void);
  /// internal function that echos strings from read string
  void doStringEcho(void);
  // internal crossplatform init (mostly for string reading stuff)
  void internalInit(void);

  // separates out a host string (possibly host:port) into a host and
  // the port that should be used.
  void separateHost(const char *rawHost, int rawPort, char *useHost, 
		    size_t useHostSize, int *port);

  Type myType;
  Error myError;
  std::string myErrorStr;
  bool myDoClose;

#ifdef WIN32
  SOCKET myFD;
  // Using the SOCKET data type mostly because this is what the Win32 methods 
  // return.
  std::string myHost;
  int myPort;
#else // Windows
  int myFD;
#endif

  bool myNonBlocking;
  struct sockaddr_in mySin;

  bool myFakeWrites;
  bool myLogWriteStrings;
  ArMutex myReadStringMutex;
  ArMutex myWriteStringMutex;
  bool myStringAutoEcho;
  bool myStringEcho;
  bool myStringIgnoreReturn;
  bool myStringWrongEndChars;
  char myStringBuf[5000];
  size_t myStringPos;
  char myStringBufEmpty[1];
  size_t myStringPosLast;
  std::string myIPString;
  char myRawIPString[128];
  ArTime myLastStringReadTime;
  bool myStringGotEscapeChars;
  bool myStringGotComplete;
  bool myStringHaveEchoed;

  long mySends;
  long myBytesSent;
  long myRecvs;
  long myBytesRecvd;

  bool myBadWrite;
  bool myBadRead;
  bool myErrorTracking;

  // A functor to call when the socket closes
  ArFunctor *myCloseFunctor;
};


#endif // ARSOCKET_H
  
