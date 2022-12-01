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
#ifndef ARSOCKET_H
#define ARSOCKET_H


#ifndef _WIN32
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#endif

#include <limits.h>

#include <string>
#include "Aria/ariaTypedefs.h"
#include "Aria/ArMutex.h"
#include "Aria/ariaUtil.h"

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
  enum Error {NoErr, NetFail, ConBadHost, ConNoRoute, ConRefused, NameLookup, InvalidParam};

  /// Constructor. You must then use either connect() or open().
  AREXPORT ArSocket();

  /// Don't copy ArSocket objects. (Move is possible.)
  ArSocket(const ArSocket &) = delete;

  /// Move constructor. Transfers ownership of underlying system socket to this instance. @sa ArSocket::transferFrom()
  ArSocket(ArSocket&& other) noexcept {
    this->transferFrom(&other);
  }

  /// Move assignment. Transfers ownership of the underlying system socket to this instance. @sa ArSocket::transferFrom()
  ArSocket& operator=(ArSocket&& other) noexcept {
    this->transferFrom(&other);
    return *this;
  }

  /// Don't assign ArSocket objects unless as a move 
  ArSocket &operator=(const ArSocket &) = delete;


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

private:
  /** @internal */
  static bool ourInitialized;

  /// Copy internal socket structures
  /// @internal
  bool copy(int fd, bool doclose);

  /// Copy socket. The other socket retains control and will close the socket when deleted. This instance will not close the socket when deleted.
  /// @internal
  void copy(ArSocket *s)
    {myFD=s->myFD; myDoClose=false; mySin=s->mySin;}

public:

  /// Transfer ownership of a socket
  /** transfer() will transfer ownership from @a s to this socket. The other socket (@a s)
      will no longer close the file descriptor when it is destructed. It should generally not be used anymore as an interface to the OS socket, use this instance instead.
  */
  AREXPORT void transferFrom(ArSocket *s) noexcept
  {
    myFD=s->myFD;
    myDoClose=true;
    s->myDoClose=false;
    mySin=s->mySin;
    myType=s->myType;  
    setIPString(s->getIPString()); 
    myDebug = s->myDebug;
  }

  [[deprecated("use transferFrom() instead")]] void transfer(ArSocket *s)
  {
    transferFrom(s);
  }

  /// Connect as a client to a server
  AREXPORT bool connect(const char *host, int port, Type type = TCP,
			const char *openOnIP = NULL);

  /** Open a server port

      Opens a server port, that is, a port that is bound to a local port (and optionally, address) 
      and listens for new incoming connections.  Use accept() to wait for a
      new incoming connection from a client.

      In addition, internally this method calls setLinger(0), setReuseAddress(),
      and setNonBlock() to turn on these options (having these on is 
      particularly useful for servers).  
      
      @param port Port number
      @param type ArSocket::TCP or ArSocket::UDP.
      @param openOnIP If given, only bind to the interface
        associated with this address (Linux only) (by default, all interfaces are used)
  */
  AREXPORT bool open(int port, Type type, const char *openOnIP = NULL);

  /// Simply create a port.
  AREXPORT bool create(Type type);

  /** Find the first valid unused port after @a startPort, and bind the socket to it.
      @param startPort first port to try
	    @param openOnIP If given, only check ports open on the interface associated with this address (Linux only)
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
  AREXPORT int sendTo(const void *msg, size_t len);

  /// Send a message (short string) on the socket
  AREXPORT int sendTo(const void *msg, size_t len, struct sockaddr_in *sin);

  /// Receive a message (short string) from the socket
  AREXPORT int recvFrom(void *msg, size_t len, sockaddr_in *sin);

  /// Convert a hostname string to an address structure
  AREXPORT static bool hostAddr(const char *host, struct in_addr &addr);

  /// Convert an address structure to a hostname string
  AREXPORT static bool addrHost(struct in_addr &addr, char *s, size_t s_size);

  AREXPORT static std::string addrHost(struct in_addr &addr);

  [[deprecated("Use addrHost(struct in_addr&, char*, size_t) or addrHost(struct in_addr&) instead.")]] 
  AREXPORT static bool addrHost(struct in_addr &addr, char *host);


  /// Get the localhost address
  AREXPORT static std::string getHostName();

  /// Get socket information (socket "name"). Aspects of this "name" are accessible with sockAddrIn(), man sock(), inPort() 
  AREXPORT bool getSockName();

  /// Accessor for the sockaddr
  struct sockaddr_in * sockAddrIn() {return(&mySin);}

  /// Accessor for the sin_addr part of sockaddr
  struct in_addr * inAddr() {return(&mySin.sin_addr);}

  /// Accessor for the sin_port of the sockaddr
  unsigned short int inPort() {return(mySin.sin_port);}

  /// Accessor for port number (converted to host byte order)
  int getPortNumber() {
    const unsigned int p = netToHostOrder(inPort());
    assert(p <= INT_MAX);
    return (int)p;
  }

  [[deprecated("Use inToA(struct in_addr*, char*, size_t) with size of string buffer instead.")]] 
  AREXPORT static void inToA(struct in_addr *addr, char *buff);

  /// Convert @a addr into string numerical address
  AREXPORT static void inToA(struct in_addr *addr, char *buff, size_t buff_size);

  AREXPORT static std::string inToA(struct in_addr& addr);

  /// Size of the sockaddr
  static size_t sockAddrLen() {return(sizeof(struct sockaddr_in));}

#ifdef WIN32
  /// Max host name length
  const static size_t MaxHostNameLen = MAXGETHOSTSTRUCT;
#else
  /// Max host name length
  const static size_t MaxHostNameLen = MAXHOSTNAMELEN;
#endif

  /// Convert an int from host byte order to network byte order
  AREXPORT static unsigned int hostToNetOrder(unsigned int i);
  /// Convert a short from host byte order to network byte order
  AREXPORT static unsigned short hostToNetOrder(unsigned short i);
  /// Convert an int from host byte order to network byte order, converting to
  /// unsigned int first. Will fail with assertion if @a i is < 0 
  AREXPORT static unsigned int hostToNetOrder(int i);

  /// Convert an int from network byte order to host byte order
  AREXPORT static unsigned int netToHostOrder(unsigned int i);
  /// Convert a short from network byte order to host byte order
  AREXPORT static unsigned short netToHostOrder(unsigned short i);
  /// Convert an int from network byte order to host byte order, after
  /// converting to unsigned int. Will fail with assertion if @a i is < 0 
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
  AREXPORT int getFD() const {return((int)myFD);}

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
  AREXPORT bool getErrorTracking() { return myErrorTracking; }

  /// Gets if we've had a bad write (you have to use error tracking for this)
  AREXPORT bool getBadWrite() const { return myBadWrite; }

  /// Gets if we've had a bad read (you have to use error tracking for this)
  AREXPORT bool getBadRead() const { return myBadRead; }


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
  AREXPORT void clearPartialReadString();
  /// Compares a string against what was partially read
  AREXPORT int comparePartialReadString(const char *partialString);
  /// Gets the time we last successfully read a string from the socket
  AREXPORT ArTime getLastStringReadTime() { return myLastStringReadTime; }
  /// Sets echoing on the readString calls this socket does
  AREXPORT void setEcho(bool echo) 
  { myStringAutoEcho = false; myStringEcho = echo; }
  /// Gets if we are echoing on the readString calls this socket does
  AREXPORT bool getEcho() { return myStringEcho; }
  /// Sets whether we log the writeStrings or not
  AREXPORT void setLogWriteStrings(bool logWriteStrings) 
    { myLogWriteStrings = logWriteStrings; }
  /// Gets whether we log the writeStrings or not
  AREXPORT bool getLogWriteStrings() { return myLogWriteStrings; }
  /// Sets whether we use the wrong (legacy) end chars or not
  AREXPORT void setStringUseWrongEndChars(bool useWrongEndChars) 
    { myStringWrongEndChars = useWrongEndChars; }
  /// Gets whether we log the writeStrings or not
  AREXPORT bool getStringUseWrongEndChars() 
    { return myStringWrongEndChars; }
  // Gets the raw ip number as a string
  //AREXPORT const char *getRawIPString() const { return myRawIPString; }
  /// Gets the ip number as a string (this can be modified though)
  AREXPORT const char *getIPString() const { return myIPString.c_str(); }
  /// Sets the ip string
  AREXPORT void setIPString(const char *ipString) noexcept
    { if (ipString != NULL) myIPString = ipString; else myIPString = ""; }
  /// Sets the callback for when the socket is closed (nicely or harshly)
  AREXPORT void setCloseCallback(ArFunctor *functor) 
    { myCloseFunctor = functor; }
  /// Sets the callback for when the socket is closed (nicely or harshly)
  AREXPORT ArFunctor *getCloseCallback() { return myCloseFunctor; }
  /// Gets the number of writes we've done
  long getSends() { return mySends; }
  /// Gets the number of bytes we've written
  long getBytesSent() { return myBytesSent; }
  /// Gets the number of reads we've done
  long getRecvs() { return myRecvs; }
  /// Gets the number of bytes we've read
  long getBytesRecvd() { return myBytesRecvd; }
  /// Resets the tracking information on the socket
  void resetTracking() 
    { mySends = 0; myBytesSent = 0; myRecvs = 0; myBytesRecvd = 0; }
  
  /// Sets NODELAY option on TCP socket, which can reduce latency for small packet sizes.
  AREXPORT bool setNoDelay(bool flag);
  bool isOpen() { return myFD > 0; }

  void setDebug(bool d = true) { myDebug = d; }
protected:
  /// Sets the ip string
  /// internal function that sets the ip string from inAddr
  void setIPString(struct in_addr* addr);
  [[deprecated("use setIPString(in_addr*) instead")]] void setRawIPString() { setIPString(inAddr());  }
  /// internal function that echos strings from read string
  void doStringEcho();
  // internal crossplatform init (mostly for string reading stuff)
  void internalInit();

  // separates out a host string (possibly host:port) into a host and
  // the port that should be used.
  // @param hostString input host string to parse. May contain only a hostname,
  // or may contain hostname and port number separated by ':' character.
  // @param defaultPort port number to return if @a hostString does not contain
  // a port number.
  // @param hostname Host name (only) is copied into this string buffer.
  // @param hostnameSize Size of @a hostname buffer
  // @param port If @a hostString contained a port number, set this output
  // parameter to the port number.  If not, set it to @a defaultPort.
  // @param ok If provided, set this boolean value to false no any parsing
  // errors, true otherwise.  Log messages are also emitted on error.
  void separateHost(const char *hostString, int defaultPort, char *hostname, 
		    size_t hostnameSize, int *port, bool *ok = NULL);




  size_t myStringPos = 0;
  size_t myStringPosLast = 0;

  long mySends = 0;
  long myBytesSent = 0;
  long myRecvs = 0;
  long myBytesRecvd = 0;

  // A functor to call when the socket closes
  ArFunctor *myCloseFunctor = nullptr;

  ArTime myLastStringReadTime;

  std::string myErrorStr;

  std::string myIPString;
  //char myRawIPString[128];

  ArMutex myReadStringMutex;
  ArMutex myWriteStringMutex;



  Type myType = Unknown;
  Error myError = NoErr;

#ifdef WIN32
  SOCKET myFD = INVALID_SOCKET;
  // Using the SOCKET data type mostly because this is what the Win32 methods 
  // return.
  std::string myHost;
  int myPort = -1;
#else // Windows
  int myFD = -1;
#endif

  struct sockaddr_in mySin;

  bool myDoClose = false;
  bool myNonBlocking = false;
  bool myFakeWrites = false;
  bool myLogWriteStrings = false;
  bool myStringAutoEcho = false;
  bool myStringEcho = false;
  bool myStringIgnoreReturn = false;
  bool myStringWrongEndChars = false;
  bool myStringGotEscapeChars = false;
  bool myStringGotComplete = false;
  bool myStringHaveEchoed;
  bool myBadWrite = false;
  bool myBadRead = false;
  bool myErrorTracking = false;
  bool myDebug = false; // log when connecting, opened, closed, destroyed

  char myStringBufEmpty[1];
  char myStringBuf[5000];
};


#endif // ARSOCKET_H
  
