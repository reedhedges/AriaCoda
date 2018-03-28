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
#ifndef ARBATTERYCONNECTOR_H
#define ARBATTERYCONNECTOR_H

#include "ariaTypedefs.h"
#include "ArSerialConnection.h"
#include "ArTcpConnection.h"
#include "ArArgumentBuilder.h"
#include "ArArgumentParser.h"
#include "ariaUtil.h"
#include "ArRobotConnector.h"

class ArBatteryMTX;
class ArRobot;



/// Connect to robot and battery based on run-time availablitily and command-line arguments
/**

   ArBatteryConnector makes a battery connection either through a serial port 
   connection, or through a TCP
   port (for the simulator or for robots with Ethernet-serial bridge
   devices instead of onboard computers).
   Normally, it first attempts a TCP connection on 
   @a localhost port 8101, to use a simulator if running. If the simulator
   is not running, then it normally then connects using the serial port
   Various connection
   parameters are configurable through command-line arguments or in the robot
   parameter file. (Though the internal interface used by ARIA to do this is also
   available if you need to use it: See addBattery(); otherwise don't use
   addBattery(), setupBattery(), etc.).
  
   When you create your ArBatteryConnector, pass it command line parameters via
   either the argc and argv variables from main(), or pass it an
   ArArgumentBuilder or ArArgumentParser object. (ArArgumentBuilder
   is able to obtain command line parameters from a Windows program
   that uses WinMain() instead of main()).
   ArBatteryConnector registers a callback with the global Aria class. Use
   Aria::parseArgs() to parse all command line parameters to the program, and
   Aria::logOptions() to print out information about all registered command-line parameters.

   The following command-line arguments are checked:
   @verbinclude ArBatteryConnector_options

   To connect to any batteries that were set up in the robot parameter file or
   via command line arguments, call connectBatteries().  If successful, 
   connectBatteries() will return true and add an entry for each battery connected
   in the ArRobot object's list of batteries.  These ArBatteryMTX objects can be
   accessed from your ArRobot object via ArRobot::findBattery() or ArRobot::getBatteryMap(). 
   

   @since 2.8.0

 **/
class ArBatteryConnector
{
public:
  /// Constructor that takes argument parser
  AREXPORT ArBatteryConnector(ArArgumentParser *parser, 
			    ArRobot *robot, ArRobotConnector *robotConnector,
			    bool autoParseArgs = true,
			    ArLog::LogLevel infoLogLevel = ArLog::Verbose);
  /// Destructor
  AREXPORT ~ArBatteryConnector(void);
  /// Connects all the batteries the robot has that should be auto connected
  AREXPORT bool connectBatteries(bool continueOnFailedConnect = false,
			      bool addConnectedBatteriesToRobot = true,
			      bool addAllBatteriesToRobot = false,
			      bool turnOnBatteries = true,
			      bool powerCycleBatteryOnFailedConnect = true);
  AREXPORT bool disconnectBatteries();
  /// Sets up a battery to be connected
  AREXPORT bool setupBattery(ArBatteryMTX *battery, 
			   int batteryNumber = 1);
  /// Connects the battery synchronously (will take up to a minute)
  AREXPORT bool connectBattery(ArBatteryMTX *battery,
			     int batteryNumber = 1,
			     bool forceConnection = true);
  /// Adds a battery so parsing will get it
  AREXPORT bool addBattery(ArBatteryMTX *battery,
			 int batteryNumber = 1);
  /// Function to parse the arguments given in the constructor
  AREXPORT bool parseArgs(void);
  /// Function to parse the arguments given in an arbitrary parser
  AREXPORT bool parseArgs(ArArgumentParser *parser);
  /// Log the options the simple connector has
  AREXPORT void logOptions(void) const;
  /// Internal function to get the battery (only useful between parseArgs and connectBatteries)
  AREXPORT ArBatteryMTX *getBattery(int batteryNumber);

  /// Internal function to replace the battery (only useful between parseArgs and connectBatteries) but not the battery data
  AREXPORT bool replaceBattery(ArBatteryMTX *battery, int batteryNumber);
  
protected:
/// Class that holds information about the battery data
class BatteryData
{
	public:
		BatteryData (int number, ArBatteryMTX *battery) {
			myNumber = number;
			myBattery = battery;
			myConn = NULL;
			myConnect = false; myConnectReallySet = false;
			myPort = NULL;
			myPortType = NULL;
			myType = NULL;
			myRemoteTcpPort = 0; myRemoteTcpPortReallySet = false;
			myBaud = NULL;
			myAutoConn = NULL;
		}
		virtual ~BatteryData() {}
		/// The number of this battery
		int myNumber;
		/// The actual pointer to this battery
		ArBatteryMTX *myBattery;
		// our connection
		ArDeviceConnection *myConn;
		// if we want to connect the battery
		bool myConnect;
		// if myConnect was really set
		bool myConnectReallySet;
		// the port we want to connect the battery on
		const char *myPort;
		// the type of port we want to connect to the battery on
		const char *myPortType;
		// battery Type
		const char *myType;
		// wheather to auto conn
		const char *myAutoConn;
		// battery tcp port if we're doing a remote host
		int myRemoteTcpPort;
		// if our remote battery tcp port was really set
		bool myRemoteTcpPortReallySet;
		/// the baud we want to use
		const char *myBaud;
};
  std::map<int, BatteryData *> myBatteries;
  
  /// Parses the battery arguments
  AREXPORT bool parseBatteryArgs(ArArgumentParser *parser, 
			       BatteryData *batteryData);
  /// Logs the battery command line option help text. 
  AREXPORT void logBatteryOptions(BatteryData *batterydata, bool header = true, bool metaOpts = true) const;
  // Sets the battery parameters
  bool internalConfigureBattery(BatteryData *batteryData);

  std::string myBatteryTypes;

  // our parser
  ArArgumentParser *myParser;
  bool myOwnParser;
  // if we should autoparse args or toss errors 
  bool myAutoParseArgs;
  bool myParsedArgs;

  ArRobot *myRobot;
  ArRobotConnector *myRobotConnector;

  // variables to hold if we're logging or not
  bool myBatteryLogPacketsReceived;
  bool myBatteryLogPacketsSent;

  ArLog::LogLevel myInfoLogLevel;

  ArRetFunctorC<bool, ArBatteryConnector> myParseArgsCB;
  ArConstFunctorC<ArBatteryConnector> myLogOptionsCB;
};

#endif // ARLASERCONNECTOR_H
