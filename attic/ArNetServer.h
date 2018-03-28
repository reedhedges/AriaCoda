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
#ifndef ARNETSERVER_H
#define ARNETSERVER_H

#include "ariaTypedefs.h"
#include "ArSocket.h"
#include "ArFunctor.h"
#include "ariaUtil.h"

#include <list>


class ArRobot;

class ArArgumentBuilder;

/// A simple TCP server receive text commands
/**
   This class is for running a simple server which will have a
   list of commands to use and a fairly simple set of interactions.

   For a more full features system for publishing and receiving data as well
   as commands which engange application modes, see the ArNetworking library,
   provided with ARIA.  ArNetworking also includes many predefined services
   and features.

   Start the text server with the open() function, add commands with the
   addCommand() function and remove commands with remCommand(), and close
   the server with the close() function.

   A client can connect via TCP on the port provided to open() and send
   a line of text where the first word is the command and the following
   words are extra arguments or data (space separated).  The line should
   end with a newline ("\n") or carriage return character.   The first
   line sent should be a password and must match the password given to
   open() in order to continue.

   You can use the "telnet" program as a general client to any ArNetServer server.

   It has a built in mutex, if you only use sendToAllClients() through
   the normal commands or during the robot loop you don't need to
   worry about locking anything and the server is locked before any of
   the callbacks for the commands are called so you really only need
   to lock the server if you're dealing with from another thread....
   From another thread you can use sendToAllClientsNextCycle which
   takes care of all the locking itself in a threadsafe way (it puts
   the message in a list, then sends it in the next cycle of the
   loop).  The only real reason to use the
   lock/sendToAllClients/unlock method is if you're highly concerned
   about synchronizing the different types of output.

    @ingroup OptionalClasses
**/
class ArNetServer
{
public:
  /// Constructor
  AREXPORT ArNetServer(bool addAriaExitCB = true,
		       bool doNotAddShutdownServer = false, 
		       const char *name = "ArNetServer", 
		       ArNetServer *childServer = NULL);
  /// Destructor
  AREXPORT ~ArNetServer();
  
  /// Initializes the server
  AREXPORT bool open(ArRobot *robot, unsigned int port, 
		     const char *password, bool multipleClients = true,
		     const char *openOnIP = NULL);

  /// Closes the server
  AREXPORT void close(void);

  /// Adds a new command
  AREXPORT bool addCommand(const char *command, 
			   ArFunctor3<char **, int, ArSocket *> *functor, 
			   const char *help);

  /// Removes a command
  AREXPORT bool remCommand(const char *command);

  /// Gets the name of this instance
  const char *getName(void) { return myName.c_str(); }
#ifndef SWIG
  /** @brief Sends the given string to all the clients.  See also the
   *  notes on locking in the class description.
   *  @swigomit @sa
   *  sendToAllClientsPlain()
   */
  AREXPORT void sendToAllClients(const char *str, ...);
#endif

  /// Sends the given string to all the clients, no varargs, wrapper for java
  AREXPORT void sendToAllClientsPlain(const char *str);

#ifndef SWIG
  /** @brief Sends the given string to all the clients next cycle
   *  @swigomit
   *  @sa sendToAllClientsNextCyclePlain()
   */
  AREXPORT void sendToAllClientsNextCycle(const char *str, ...);
#endif

  /// Sends the given string to all the clients next cycle, no varargs
  AREXPORT void sendToAllClientsNextCyclePlain(const char *str);

  /// Sends the given string to all the clients next cycle, no varargs... helper for config changes
  AREXPORT bool sendToAllClientsNextCyclePlainBool(const char *str);

#ifndef SWIG
  /** @brief Sends the given string to the (hopefully) the client given (this method may go away)
   *  @swigomit
   *  @sa sendToClientPlain()
   */
  AREXPORT void sendToClient(ArSocket *socket, const char *ipString,
			     const char *str, ...);
#endif

  /// Sends the given plain string to the (hopefully) the client given (this method may go away)
  AREXPORT void sendToClientPlain(ArSocket *socket, const char *ipString,
				  const char *str);

  /// Sees if the server is running and open
  AREXPORT bool isOpen(void);

  /// Sets whether we are logging all data sent or not
  AREXPORT void setLoggingDataSent(bool loggingData);
  
  /// Gets whether we are logging all data sent or not
  AREXPORT bool getLoggingDataSent(void);

  /// Sets whether we are logging all data received or not
  AREXPORT void setLoggingDataReceived(bool loggingData);
  
  /// Gets whether we are logging all data received or not
  AREXPORT bool getLoggingDataReceived(void);

  /// Sets whether we're using the wrong (legacy) end chars or not
  AREXPORT void setUseWrongEndChars(bool useWrongEndChars);

  /// Gets whether we're using the wrong (legacy) end chars or not
  AREXPORT bool getUseWrongEndChars(void);

  /// the internal sync task we use for our loop
  AREXPORT void runOnce(void);

  /// the internal function that gives the greeting message
  AREXPORT void internalGreeting(ArSocket *socket);
  
  /// The internal function that does the help
  AREXPORT void internalHelp(ArSocket *socket);
  /// The internal function for the help cb
  AREXPORT void internalHelp(char **argv, int argc, ArSocket *socket);
  /// The internal function for echo
  AREXPORT void internalEcho(char **argv, int argc, ArSocket *socket);
  /// The internal function for closing this connection
  AREXPORT void internalQuit(char **argv, int argc, ArSocket *socket);
  /// The internal function for shutting down
  AREXPORT void internalShutdownServer(char **argv, int argc, 
				       ArSocket *socket);
  /// The internal function for parsing a command on a socket
  AREXPORT void parseCommandOnSocket(ArArgumentBuilder *args, 
				     ArSocket *socket, bool allowLog = true);
  /// The internal function that adds a client to our list
  AREXPORT void internalAddSocketToList(ArSocket *socket);
  /// The internal function that adds a client to our delete list
  AREXPORT void internalAddSocketToDeleteList(ArSocket *socket);
  /// This squelchs all the normal commands and help
  AREXPORT void squelchNormal(void);
  /// Sets an extra string that the server holds for passing around
  AREXPORT void setExtraString(const char *str) { myExtraString = str; }
  /// Gets an extra string that the server holds for passing around
  AREXPORT const char *getExtraString(void) { return myExtraString.c_str(); }
  /// Lock the server
  AREXPORT int lock() {return(myMutex.lock());}
  /// Try to lock the server without blocking
  AREXPORT int tryLock() {return(myMutex.tryLock());}
  /// Unlock the server
  AREXPORT int unlock() {return(myMutex.unlock());}
protected:
  std::string myName;
  ArNetServer *myChildServer;
  ArMutex myMutex;
  ArSocket myAcceptingSocket;
  std::map<std::string, ArFunctor3<char **, int, ArSocket *> *, ArStrCaseCmpOp> myFunctorMap;
  std::map<std::string, std::string, ArStrCaseCmpOp> myHelpMap;
  bool myLoggingDataSent;
  bool myLoggingDataReceived;
  bool myUseWrongEndChars;
  bool myOpened;
  bool myWantToClose;
  bool mySquelchNormal;
  ArSocket myServerSocket;
  ArRobot *myRobot;
  std::string myPassword;
  bool myMultipleClients;
  unsigned int myPort;
  std::string myExtraString;
  std::list<ArSocket *> myConns;
  std::list<ArSocket *> myConnectingConns;
  std::list<ArSocket *> myDeleteList;
  
  ArMutex myNextCycleSendsMutex;
  std::list<std::string> myNextCycleSends;
  
  ArFunctorC<ArNetServer> myTaskCB;
  ArFunctor3C<ArNetServer, char **, int, ArSocket *> myHelpCB;
  ArFunctor3C<ArNetServer, char **, int, ArSocket *> myEchoCB;
  ArFunctor3C<ArNetServer, char **, int, ArSocket *> myQuitCB;
  ArFunctor3C<ArNetServer, char **, int, ArSocket *> myShutdownServerCB;
  ArFunctorC<ArNetServer> myAriaExitCB;
};

#endif // ARNETSERVER_H
