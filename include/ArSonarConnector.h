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
#ifndef ARSONARCONNECTOR_H
#define ARSONARCONNECTOR_H

#include "ariaTypedefs.h"
#include "ArSerialConnection.h"
#include "ArTcpConnection.h"
#include "ArArgumentBuilder.h"
#include "ArArgumentParser.h"
#include "ariaUtil.h"
#include "ArRobotConnector.h"

class ArSonarMTX;
class ArRobot;



/// Connect to sonar based on robot parameters and command-line arguments
/**

   ArSonarConnector makes a sonar connection either through a serial port 
   connection.
   When you create your ArSonarConnector, pass it command line parameters via
   either the argc and argv variables from main(), or pass it an
   ArArgumentBuilder or ArArgumentParser object. (ArArgumentBuilder
   is able to obtain command line parameters from a Windows program
   that uses WinMain() instead of main()).
   ArSonarConnector registers a callback with the global Aria class. Use
   Aria::parseArgs() to parse all command line parameters to the program, and
   Aria::logOptions() to print out information about all registered command-line parameters.

   The following command-line arguments are checked:
   @verbinclude ArSonarConnector_options

   To connect to any sonars that were set up in the robot parameter file or
   via command line arguments, call connectSonars().  If successful, 
   connectSonars() will return true and add an entry for each sonar connected
   in the ArRobot object's list of sonars.  These ArSonarMTX objects can be
   accessed from your ArRobot object via ArRobot::findSonar() or ArRobot::getSonarMap(). 
   

   @since 2.8.0

 **/
class ArSonarConnector
{
public:
  /// Constructor that takes argument parser
  AREXPORT ArSonarConnector(
	  ArArgumentParser *parser, 
	  ArRobot *robot, ArRobotConnector *robotConnector,
	  bool autoParseArgs = true,
	  ArLog::LogLevel infoLogLevel = ArLog::Verbose,
      	  ArRetFunctor1<bool, const char *> *turnOnPowerOutputCB = NULL,
	  ArRetFunctor1<bool, const char *> *turnOffPowerOutputCB = NULL);
  /// Destructor
  AREXPORT ~ArSonarConnector(void);
  /// Connects all the sonars the robot has that should be auto connected
  AREXPORT bool connectSonars(
						bool continueOnFailedConnect = false,
			      bool addConnectedSonarsToRobot = true,
			      bool addAllSonarsToRobot = false,
			      bool turnOnSonars = true,
			      bool powerCycleSonarOnFailedConnect = true);
	// Connects all the sonars in replay mode
	AREXPORT bool connectReplaySonars(
						bool continueOnFailedConnect = false,
			      bool addConnectedSonarsToRobot = true,
			      bool addAllSonarsToRobot = false,
			      bool turnOnSonars = true,
			      bool powerCycleSonarOnFailedConnect = true);
  /// Sets up a sonar to be connected
  AREXPORT bool setupSonar(ArSonarMTX *sonar, 
			   int sonarNumber = 1);
  /// Connects the sonar synchronously (will take up to a minute)
  AREXPORT bool connectSonar(ArSonarMTX *sonar,
			     int sonarNumber = 1,
			     bool forceConnection = true);
  /// Adds a sonar so parsing will get it
  AREXPORT bool addSonar(ArSonarMTX *sonar,
			 int sonarNumber = 1);
  /// Function to parse the arguments given in the constructor
  AREXPORT bool parseArgs(void);
  /// Function to parse the arguments given in an arbitrary parser
  AREXPORT bool parseArgs(ArArgumentParser *parser);
  /// Log the options the simple connector has
  AREXPORT void logOptions(void) const;
  /// Internal function to get the sonar (only useful between parseArgs and connectSonars)
  AREXPORT ArSonarMTX *getSonar(int sonarNumber);

  /// Internal function to replace the sonar (only useful between parseArgs and connectSonars) but not the sonar data
  AREXPORT bool replaceSonar(ArSonarMTX *sonar, int sonarNumber);
  
  AREXPORT bool disconnectSonars();
protected:
/// Class that holds information about the sonar data
class SonarData
{
	public:
		SonarData (int number, ArSonarMTX *sonar) {
			myNumber = number;
			mySonar = sonar;
			myConn = NULL;
			myConnect = false; myConnectReallySet = false;
			myPort = NULL;
			myPortType = NULL;
			myType = NULL;
			myRemoteTcpPort = 0; myRemoteTcpPortReallySet = false;
			myBaud = NULL;
			myAutoConn = NULL;
		}
		virtual ~SonarData() {}
		/// The number of this sonar
		int myNumber;
		/// The actual pointer to this sonar
		ArSonarMTX *mySonar;
		// our connection
		ArDeviceConnection *myConn;
		// if we want to connect the sonar
		bool myConnect;
		// if myConnect was really set
		bool myConnectReallySet;
		// the port we want to connect the sonar on
		const char *myPort;
		// the type of port we want to connect to the sonar on
		const char *myPortType;
		// sonar Type
		const char *myType;
		// wheather to auto conn
		const char *myAutoConn;
		// sonar tcp port if we're doing a remote host
		int myRemoteTcpPort;
		// if our remote sonar tcp port was really set
		bool myRemoteTcpPortReallySet;
		/// the baud we want to use
		const char *myBaud;
    /// Set baud convert from integer
    void setBaud(int baud) 
    {
      snprintf(myBaudBuf, 256, "%d", baud);
      myBaud = (const char*)myBaudBuf;
    }
    /// set AutoConn from boolean
    void setAutoConn(bool a)
    {
      snprintf(myAutoConnBuf, 256, "%s", a?"true":"false");
      myAutoConn = (const char *)myAutoConnBuf;
    }
private:
    /// Stores baud string if converted from an integer by setBaud()
    char myBaudBuf[256];
    /// Stores AutoConn string if converted from a boolean by setAutConn()
    char myAutoConnBuf[256];
};

	/// Turns on the power for the specific board in the firmware
	AREXPORT bool turnOnPower(SonarData *sonarData);

  std::map<int, SonarData *> mySonars;
  
  /// Parses the sonar arguments
  AREXPORT bool parseSonarArgs(ArArgumentParser *parser, 
			       SonarData *sonarData);
  /// Logs the sonar command line option help text. 
  AREXPORT void logSonarOptions(SonarData *sonardata, bool header = true, bool metaOpts = true) const;
  // Sets the sonar parameters
  bool internalConfigureSonar(SonarData *sonarData);

  std::string mySonarTypes;

  // our parser
  ArArgumentParser *myParser;
  bool myOwnParser;
  // if we should autoparse args or toss errors 
  bool myAutoParseArgs;
  bool myParsedArgs;

  ArRobot *myRobot;
  ArRobotConnector *myRobotConnector;

  // variables to hold if we're logging or not
  bool mySonarLogPacketsReceived;
  bool mySonarLogPacketsSent;

  ArLog::LogLevel myInfoLogLevel;

  ArRetFunctor1<bool, const char *> *myTurnOnPowerOutputCB;
  ArRetFunctor1<bool, const char *> *myTurnOffPowerOutputCB;

  ArRetFunctorC<bool, ArSonarConnector> myParseArgsCB;
  ArConstFunctorC<ArSonarConnector> myLogOptionsCB;
};

#endif // ARLASERCONNECTOR_H
