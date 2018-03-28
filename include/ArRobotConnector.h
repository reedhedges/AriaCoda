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
#ifndef ARROBOTCONNECTOR_H
#define ARROBOTCONNECTOR_H

#include "ariaTypedefs.h"
#include "ArSerialConnection.h"
#include "ArTcpConnection.h"
#include "ArArgumentBuilder.h"
#include "ArArgumentParser.h"
#include "ariaUtil.h"

class ArRobot;
class ArSonarConnector;
class ArBatteryConnector;
//class ArLCDConnector;

/** Connect to robot or simulator based on program command line parameters.

   ArRobotConnector makes a robot connection either through a TCP
   port (for the simulator or for robots with Ethernet-serial bridge
   devices instead of onboard computers), or through a direct serial 
   port connection.  Normally, it first attempts a TCP connection on 
   @a localhost port 8101, to use a simulator if running. If the simulator
   is not running, then it normally then connects using the serial port
   (the first serial port, COM1, by default).  Various other connection
   parameters are configurable through command-line arguments.
  
   When you create your ArRobotConnector, pass it command line parameters via
   either the argc and argv variables from main(), or pass it an
   ArArgumentBuilder or ArArgumentParser object. (ArArgumentBuilder
   is able to obtain command line parameters from a Windows program
   that uses WinMain() instead of main()).
   ArRobotConnector registers a callback with the global Aria class. Use
   Aria::parseArgs() to parse all command line parameters to the program, and
   Aria::logOptions() to print out information about all registered command-line parameters.

   The following command-line arguments are checked:
   @verbinclude ArRobotConnector_options

   You can then prepare an ArRobot object for connection (with various connection
   options configured via the command line parameters) and initiate the connection
   attempt by that object by calling connectRobot().
    
   After it's connected, you must then begin the robot processing cycle by calling
   ArRobot::runAsync() or ArRobot::run().

   @sa ArLaserConnector
   @sa ArRobot

   @since 2.7.0

   @ingroup ImportantClasses
   @ingroup DeviceClasses

**/
class ArRobotConnector
{
public:
  /// Constructor that takes argument parser
  AREXPORT ArRobotConnector(ArArgumentParser *parser, ArRobot *robot, 
			    bool autoParseArgs = true, bool connectAllComponents = true);
  /// Destructor
  AREXPORT ~ArRobotConnector(void);
  /// Sets up the robot then connects it
  AREXPORT bool connectRobot(void);
  /// Sets up the robot then connects it
  AREXPORT bool connectRobot(ArRobot *robot);
  void setResetOldConnection(bool r) { myTryHarderToConnect = r; }
  /// Configures connection, but does not connect, connectRobot() is recommended instead in most cases.
  AREXPORT bool setupRobot(void);
  /// Configures connection, but does not connect, connectRobot() is recommended instead in most cases.
  AREXPORT bool setupRobot(ArRobot *robot);
  /// Function to parse the arguments given in the constructor
  AREXPORT bool parseArgs(void);
  /// Function to parse the arguments given in an arbitrary parser
  AREXPORT bool parseArgs(ArArgumentParser *parser);
  /// Log the options the simple connector has
  AREXPORT void logOptions(void) const;
  /// Gets the remote host, if one was used, or NULL if it wasn't
  AREXPORT const char *getRemoteHost(void) const;
  /// Gets if the remote connection is a sim
  AREXPORT bool getRemoteIsSim(void) const;
  /// Call for forcing the remote to be a sim (mostly for internal use)
  AREXPORT void setRemoteIsSim(bool remoteIsSim);
  /// Gets the robot this connector is using (mostly for backwards compatibility stuff)
  AREXPORT ArRobot *getRobot(void);
  /// Disconnect from any robot components connected to. If connectAllComponents
  /// was not set to false in constructor, then connections to all robot components, and the
  /// robot itself, are disconnected.
  AREXPORT bool disconnectAll();
protected:
  // the robot we've set up (so we can find its params)
  ArRobot *myRobot; 
  // if we're using the sim or not
  bool myUsingSim;
  // if we're connecting via tcp (not to the sim), what remote host
  const char *myRemoteHost;
  // robot port, if there isn't one this'll be NULL, which will just
  // be the default of ArUtil::COM1
  const char *myRobotPort;
  // baud for the serial
  int myRobotBaud;
  // robot tcp port if we're doing a remote host (defaults to 8101)
  int myRemoteRobotTcpPort;
  
  // whether we're connecting to a remote sim or not (so we don't try
  // to open a port for the laser)
  bool myRemoteIsSim;
  // force a sim to not look like a sim (for laser debugging mostly)
  bool myRemoteIsNotSim;
  // variables to hold if we're logging or not
  bool myRobotLogPacketsReceived;
  bool myRobotLogPacketsSent;
  bool myRobotLogMovementReceived;
  bool myRobotLogMovementSent;
  bool myRobotLogVelocitiesReceived;
  bool myRobotLogActions;

  // if we're auto parsing the arguments when setupRobot or
  // connectRobot are called
  bool myAutoParseArgs;
  // If we've parsed the args already or not
  bool myHaveParsedArgs;

  // If true, create additional connectors such as sonar, battery, lcd.
  bool myConnectAllComponents;
  

  // our parser
  ArArgumentParser *myParser;
  bool myOwnParser;

  // a few device connections to use to connect to the robot
  ArTcpConnection myRobotTcpConn;
  ArSerialConnection myRobotSerConn;

  ArRetFunctorC<bool, ArRobotConnector> myParseArgsCB;
  ArConstFunctorC<ArRobotConnector> myLogOptionsCB;

//  ArLCDConnector *myLCDConnector;
  ArBatteryConnector *myBatteryConnector;
  ArSonarConnector *mySonarConnector;

  bool myTryHarderToConnect;
};


#endif
