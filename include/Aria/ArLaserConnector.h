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
#ifndef ARLASERCONNECTOR_H
#define ARLASERCONNECTOR_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ArArgumentParser.h"
#include "Aria/ariaUtil.h"
#include "Aria/ArRobotConnector.h"

class ArLaser;
class ArRobot;



/// Create laser interface objects (for any kind of laser supported by ARIA) and connect to lasers based on parameters from robot parameter file and command-line arguments
/**

   ArLaserConnector makes a laser connection (e.g. through serial port, 
   TCP network connection, or to simulator connection as a special case if
   the robot connection is to a simulator.)
   Parameters are configurable through command-line arguments or in the robot
   parameter file. 

  
   When you create your ArLaserConnector, pass it command line parameters via
   either the argc and argv variables from main(), or pass it an
   ArArgumentBuilder or ArArgumentParser object. (ArArgumentBuilder
   is able to obtain command line parameters from a Windows program
   that uses WinMain() instead of main()).
   ArLaserConnector registers a callback with the global Aria class. Use
   Aria::parseArgs() to parse all command line parameters to the program, and
   Aria::logOptions() to print out information about all registered command-line parameters.
   ArLaserConnector will be included in these.

   Then, to connect to any lasers that were set up in the robot parameter file or
   via command line arguments, call connectLasers().  If successful, 
   connectLasers() will return true and add an entry for each laser connected
   in the ArRobot object's list of lasers.  These ArLaser objects can be
   accessed from your ArRobot object via ArRobot::findLaser() or ArRobot::getLaserMap(). 
   
   (The internal interface used by ARIA to connect to configured lasers and add
   them to ArRobot is also
   available if you need to use it: See addLaser(); but this is normally not
neccesary for almost all cases.)

   The following command-line arguments are checked:
   @verbinclude ArLaserConnector_options

   @since 2.7.0
   @ingroup ImportantClasses
   @ingroup DeviceClasses
 **/
class ArLaserConnector
{
public:
  /// Constructor that takes argument parser
  AREXPORT ArLaserConnector(
	  ArArgumentParser *parser, 
	  ArRobot *robot, ArRobotConnector *robotConnector,
	  bool autoParseArgs = true,
	  ArLog::LogLevel infoLogLevel = ArLog::Verbose,
	  ArRetFunctor1<bool, const char *> *turnOnPowerOutputCB = NULL,
	  ArRetFunctor1<bool, const char *> *turnOffPowerOutputCB = NULL);
  //AREXPORT ~ArLaserConnector();
  /// Connects all the lasers the robot has that should be auto connected
  AREXPORT bool connectLasers(bool continueOnFailedConnect = false,
			      bool addConnectedLasersToRobot = true,
			      bool addAllLasersToRobot = false,
			      bool turnOnLasers = true,
			      bool powerCycleLaserOnFailedConnect = true,
			      int *failedOnLaser = NULL);
  /// Sets up a laser to be connected
  AREXPORT bool setupLaser(ArLaser *laser, 
			   int laserNumber = 1);
  /// Connects the laser synchronously (will take up to a minute)
  AREXPORT bool connectLaser(ArLaser *laser,
			     int laserNumber = 1,
			     bool forceConnection = true);
  /// Adds a laser so parsing will get it
  AREXPORT bool addLaser(ArLaser *laser,
			 int laserNumber = 1);
  /// Adds a laser for parsing but where connectLaser will be called later
  AREXPORT bool addPlaceholderLaser(ArLaser *placeholderLaser,
				    int laserNumber = 1,
				    bool takeOwnershipOfPlaceholder = false);
  /// Function to parse the arguments given in the constructor
  AREXPORT bool parseArgs();
  /// Function to parse the arguments given in an arbitrary parser
  AREXPORT bool parseArgs(ArArgumentParser *parser);
  /// Log the command-line options available to the user
  AREXPORT void logOptions() const;
  /// Internal function to get the laser (only useful between parseArgs and connectLasers)
  AREXPORT ArLaser *getLaser(int laserNumber);

  /// Internal function to replace the laser (only useful between parseArgs and connectLasers) but not the laser data
  AREXPORT bool replaceLaser(ArLaser *laser, int laserNumber);

  /// Log all currently set parameter values
  AREXPORT void logLaserData();
  
protected:
  /// Class that holds information about the laser data
  class LaserData
  {
  public:
    LaserData(int number, ArLaser *laser, bool laserIsPlaceholder = false, bool ownPlaceholder = false) :
      myLaser(laser),
      myNumber(number),
      myLaserIsPlaceholder(laserIsPlaceholder),
      myOwnPlaceholder(ownPlaceholder)
    { 
/* initialized in declarations
      myNumber = number; 
      myLaser = laser; 
      myConn = NULL;
      myLaserIsPlaceholder = laserIsPlaceholder;
      myOwnPlaceholder = ownPlaceholder;
      myConnect = false; 
      myConnectReallySet = false;
      myPort = NULL; 
      myPortType = NULL;
      myRemoteTcpPort = 0; 
      myRemoteTcpPortReallySet = false;
      myFlipped = false; 
      myFlippedReallySet = false; 
      myDegreesStart = HUGE_VAL; 
      myDegreesStartReallySet = false; 
      myDegreesEnd = -HUGE_VAL; 
      myDegreesEndReallySet = false; 
      myDegrees = NULL; 
      myIncrementByDegrees = -HUGE_VAL; 
      myIncrementByDegreesReallySet = false; 
      myIncrement = NULL; 
      myUnits = NULL; 
      myReflectorBits = NULL;
      myPowerControlled = true; 
      myPowerControlledReallySet = false; 
      myStartingBaud = NULL;
      myAutoBaud = NULL;
      myMaxRange = UINT_MAX; 
      myMaxRangeReallySet = false; 
      myAdditionalIgnoreReadings = NULL;
*/
    }

    /// The actual pointer to this laser
    ArLaser *myLaser = nullptr;
    // our connection
    ArDeviceConnection *myConn = nullptr;
    // the port we want to connect the laser on
    const char *myPort = nullptr;
    // the type of port we want to connect to the laser on
    const char *myPortType = nullptr;
    // what degrees to start at 
    double myDegreesStart = HUGE_VAL;
    // what degrees to end at 
    double myDegreesEnd = -HUGE_VAL;
    // the degrees we want wto use
    const char *myDegrees = nullptr;
    // what increment to use
    double myIncrementByDegrees = -HUGE_VAL;
    // the increment we want to use
    const char *myIncrement = nullptr;
    /// the units we want to use 
    const char *myUnits = nullptr;
    /// the reflector bits we want to use 
    const char *myReflectorBits = nullptr;
    /// the starting baud we want to use
    const char *myStartingBaud = nullptr;
    /// the auto baud we want to use
    const char *myAutoBaud = nullptr;
    /// the additional laser ignore readings
    const char *myAdditionalIgnoreReadings = nullptr;
    /// The number of this laser
    int myNumber = -1;
    // laser tcp port if we're doing a remote host
    int myRemoteTcpPort = 0;  
    // if we set a new max range from the command line
    unsigned int myMaxRange = UINT_MAX;
    /// If the laser is a placeholder for parsing
    bool myLaserIsPlaceholder = false;
    /// If we own the placeholder laser
    bool myOwnPlaceholder = false;
    // if we want to connect the laser
    bool myConnect = false;
    // if myConnect was really set
    bool myConnectReallySet = false;
    // if our remote laser tcp port was really set
    bool myRemoteTcpPortReallySet = false;
    // if we have the laser flipped
    bool myFlipped = false;
    // if our flipped was really set
    bool myFlippedReallySet = false;
    // if our start degrees was really set
    bool myDegreesStartReallySet = false;
    // if our end degrees was really set
    bool myDegreesEndReallySet = false;
    // if our end degrees was really set
    bool myIncrementByDegreesReallySet = false;
    // if we are controlling the laser power
    bool myPowerControlled = true;
    // if our flipped was really set
    bool myPowerControlledReallySet = false;
    // if our new max range was really set
    bool myMaxRangeReallySet = false;
  };
  std::map<int, LaserData *> myLasers;
  
  /// Parses the laser arguments
  AREXPORT bool parseLaserArgs(ArArgumentParser *parser, 
			       LaserData *laserData);
  /// Logs the laser command line option help text. 
  AREXPORT void logLaserOptions(LaserData *laserdata, bool header = true, bool metaOpts = true) const;
  // Sets the laser parameters
  bool internalConfigureLaser(LaserData *laserData);

  std::string myLaserTypes;

  // our parser
  ArArgumentParser *myParser;
  bool myOwnParser;
  // if we should autoparse args or toss errors 
  bool myAutoParseArgs;
  bool myParsedArgs;

  ArRobot *myRobot;
  ArRobotConnector *myRobotConnector;

  ArLog::LogLevel myInfoLogLevel;

  ArRetFunctor1<bool, const char *> *myTurnOnPowerOutputCB;
  ArRetFunctor1<bool, const char *> *myTurnOffPowerOutputCB;

  ArRetFunctorC<bool, ArLaserConnector> myParseArgsCB;
  ArConstFunctorC<ArLaserConnector> myLogOptionsCB;
};

#endif // ARLASERCONNECTOR_H
