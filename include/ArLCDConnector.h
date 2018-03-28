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
#ifndef ARLCDCONNECTOR_H
#define ARLCDCONNECTOR_H

#include "ariaTypedefs.h"
#include "ArSerialConnection.h"
#include "ArTcpConnection.h"
#include "ArArgumentBuilder.h"
#include "ArArgumentParser.h"
#include "ariaUtil.h"
#include "ArRobotConnector.h"

class ArLCDMTX;
class ArRobot;



/// Connect to robot and lcd based on run-time availablitily and command-line arguments
/**

   ArLCDConnector makes a lcd connection either through a serial port 
   connection, or through a TCP
   port (for the simulator or for robots with Ethernet-serial bridge
   devices instead of onboard computers).
   Normally, it first attempts a TCP connection on 
   @a localhost port 8101, to use a simulator if running. If the simulator
   is not running, then it normally then connects using the serial port
   Various connection
   parameters are configurable through command-line arguments or in the robot
   parameter file. (Though the internal interface used by ARIA to do this is also
   available if you need to use it: See addLCD(); otherwise don't use
   addLCD(), setupLCD(), etc.).
  
   When you create your ArLCDConnector, pass it command line parameters via
   either the argc and argv variables from main(), or pass it an
   ArArgumentBuilder or ArArgumentParser object. (ArArgumentBuilder
   is able to obtain command line parameters from a Windows program
   that uses WinMain() instead of main()).
   ArLCDConnector registers a callback with the global Aria class. Use
   Aria::parseArgs() to parse all command line parameters to the program, and
   Aria::logOptions() to print out information about all registered command-line parameters.

   The following command-line arguments are checked:
   @verbinclude ArLCDConnector_options

   To connect to any lcds that were set up in the robot parameter file or
   via command line arguments, call connectLCDs().  If successful, 
   connectLCDs() will return true and add an entry for each lcd connected
   in the ArRobot object's list of lcds.  These ArLCDMTX objects can be
   accessed from your ArRobot object via ArRobot::findLCD() or ArRobot::getLCDMap(). 
   

   @since 2.8.0

 **/
class ArLCDConnector
{
public:
  /// Constructor that takes argument parser
  AREXPORT ArLCDConnector(
	  ArArgumentParser *parser, 
	  ArRobot *robot, ArRobotConnector *robotConnector,
	  bool autoParseArgs = true,
	  ArLog::LogLevel infoLogLevel = ArLog::Verbose,
	  ArRetFunctor1<bool, const char *> *turnOnPowerOutputCB = NULL,
	  ArRetFunctor1<bool, const char *> *turnOffPowerOutputCB = NULL);
  /// Destructor
  AREXPORT ~ArLCDConnector(void);
  /// Connects all the lcds the robot has that should be auto connected
  AREXPORT bool connectLCDs(bool continueOnFailedConnect = false,
			      bool addConnectedLCDsToRobot = true,
			      bool addAllLCDsToRobot = false,
			      bool turnOnLCDs = true,
			      bool powerCycleLCDOnFailedConnect = true);
  /// Sets up a lcd to be connected
  AREXPORT bool setupLCD(ArLCDMTX *lcd, 
			   int lcdNumber = 1);
  /// Connects the lcd synchronously (will take up to a minute)
  AREXPORT bool connectLCD(ArLCDMTX *lcd,
			     int lcdNumber = 1,
			     bool forceConnection = true);
  /// Adds a lcd so parsing will get it
  AREXPORT bool addLCD(ArLCDMTX *lcd,
			 int lcdNumber = 1);
  /// Function to parse the arguments given in the constructor
  AREXPORT bool parseArgs(void);
  /// Function to parse the arguments given in an arbitrary parser
  AREXPORT bool parseArgs(ArArgumentParser *parser);
  /// Log the options the simple connector has
  AREXPORT void logOptions(void) const;
  /// Internal function to get the lcd (only useful between parseArgs and connectLCDs)
  AREXPORT ArLCDMTX *getLCD(int lcdNumber);

  /// Internal function to replace the lcd (only useful between parseArgs and connectLCDs) but not the lcd data
  AREXPORT bool replaceLCD(ArLCDMTX *lcd, int lcdNumber);

	AREXPORT void turnOnPowerCB (int);
	AREXPORT void turnOffPowerCB (int);

	AREXPORT void setIdentifier(const char *identifier);
  
protected:
/// Class that holds information about the lcd data
class LCDData
{
	public:
		LCDData (int number, ArLCDMTX *lcd) {
			myNumber = number;
			myLCD = lcd;
			myConn = NULL;
			myConnect = false; myConnectReallySet = false;
			myPort = NULL;
			myPortType = NULL;
			myType = NULL;
			myRemoteTcpPort = 0; myRemoteTcpPortReallySet = false;
			myBaud = NULL;
			myAutoConn = NULL;
			myConnFailOption = NULL;
		}
		virtual ~LCDData() {}
		/// The number of this lcd
		int myNumber;
		/// The actual pointer to this lcd
		ArLCDMTX *myLCD;
		// our connection
		ArDeviceConnection *myConn;
		// if we want to connect the lcd
		bool myConnect;
		// if myConnect was really set
		bool myConnectReallySet;
		// the port we want to connect the lcd on
		const char *myPort;
		// the type of port we want to connect to the lcd on
		const char *myPortType;
		// lcd Type
		const char *myType;
		// wheather to auto conn
		const char *myAutoConn;
		// wheather to disconnect on conn faiure 
		const char *myConnFailOption;
		// lcd tcp port if we're doing a remote host
		int myRemoteTcpPort;
		// if our remote lcd tcp port was really set
		bool myRemoteTcpPortReallySet;
		/// the baud we want to use
		const char *myBaud;
};

  std::map<int, LCDData *> myLCDs;

	/// Turns on the power for the specific board in the firmware
	AREXPORT bool turnOnPower(LCDData *LCDData);

	/// Turns off the power for the specific board in the firmware
	AREXPORT bool turnOffPower(LCDData *LCDData);

	/// Verifies the firmware version on the LCD and loads new firmware 
	/// if there is no match
	AREXPORT bool verifyFirmware(LCDData *LCDData);

	AREXPORT std::string searchForFile(
			const char *dirToLookIn, const char *prefix, const char *suffix);

  
  /// Parses the lcd arguments
  AREXPORT bool parseLCDArgs(ArArgumentParser *parser, 
			       LCDData *lcdData);
  /// Logs the lcd command line option help text. 
  AREXPORT void logLCDOptions(LCDData *lcddata, bool header = true, bool metaOpts = true) const;
  // Sets the lcd parameters
  bool internalConfigureLCD(LCDData *lcdData);

  std::string myLCDTypes;

  // our parser
  ArArgumentParser *myParser;
  bool myOwnParser;
  // if we should autoparse args or toss errors 
  bool myAutoParseArgs;
  bool myParsedArgs;

  ArRobot *myRobot;
  ArRobotConnector *myRobotConnector;

  // variables to hold if we're logging or not
  bool myLCDLogPacketsReceived;
  bool myLCDLogPacketsSent;

  ArLog::LogLevel myInfoLogLevel;

  ArRetFunctor1<bool, const char *> *myTurnOnPowerOutputCB;
  ArRetFunctor1<bool, const char *> *myTurnOffPowerOutputCB;

  ArRetFunctorC<bool, ArLCDConnector> myParseArgsCB;
  ArConstFunctorC<ArLCDConnector> myLogOptionsCB;

	ArFunctor1C<ArLCDConnector, int> myTurnOnPowerCB;
	ArFunctor1C<ArLCDConnector, int> myTurnOffPowerCB;

};

#endif // ARLASERCONNECTOR_H
