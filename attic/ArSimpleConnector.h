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
#ifndef ARSIMPLECONNECTOR_H
#define ARSIMPLECONNECTOR_H

#include "ariaTypedefs.h"
#include "ArSerialConnection.h"
#include "ArTcpConnection.h"
#include "ArArgumentBuilder.h"
#include "ArArgumentParser.h"
#include "ariaUtil.h"
#include "ArRobotConnector.h"
#include "ArLaserConnector.h"

class ArSick;
class ArRobot;



/// Legacy connector for robot and laser
/**
   This is deprecated but is left in for compatibility with old code,
   Instead use ArRobotConnector to set up ArRobot's connection to the robot, and
   ArLaserConnector to set up connections with laser rangefinder devices.

   @deprecated Use ArRobotConnector and ArLaserConnector instead
 **/
class ArSimpleConnector
{
public:
  /// Constructor that takes args from the main
  AREXPORT ArSimpleConnector(int *argc, char **argv);
  /// Constructor that takes argument builder
  AREXPORT ArSimpleConnector(ArArgumentBuilder *arguments);
  /// Constructor that takes argument parser
  AREXPORT ArSimpleConnector(ArArgumentParser *parser);
  /// Destructor
  AREXPORT ~ArSimpleConnector(void);
  /// Sets up the robot to be connected
  AREXPORT bool setupRobot(ArRobot *robot);
  /// Sets up the robot then connects it
  AREXPORT bool connectRobot(ArRobot *robot);
  /// Sets up the laser to be connected
  AREXPORT bool setupLaser(ArSick *laser);
  /// Sets up a second laser to be connected
  AREXPORT bool setupSecondLaser(ArSick *laser);
  /// Sets up a laser t obe connected (make sure you setMaxNumLasers)
  AREXPORT bool setupLaserArbitrary(ArSick *laser, 
				    int laserNumber);
  /// Connects the laser synchronously (will take up to a minute)
  AREXPORT bool connectLaser(ArSick *laser);
  /// Connects the laser synchronously (will take up to a minute)
  AREXPORT bool connectSecondLaser(ArSick *laser);
  /// Connects the laser synchronously  (make sure you setMaxNumLasers)
  AREXPORT bool connectLaserArbitrary(ArSick *laser, int laserNumber);
  /// Function to parse the arguments given in the constructor
  AREXPORT bool parseArgs(void);
  /// Function to parse the arguments given in an arbitrary parser
  AREXPORT bool parseArgs(ArArgumentParser *parser);
  /// Log the options the simple connector has
  AREXPORT void logOptions(void) const;
  /// Sets the number of possible lasers 
  AREXPORT void setMaxNumLasers(int maxNumLasers = 1);
protected:
  /// Finishes the stuff the constructor needs to do
  void finishConstructor(void);

  ArArgumentParser *myParser;
  bool myOwnParser;
  ArRobotConnector *myRobotConnector;
  ArLaserConnector *myLaserConnector;
};

#endif // ARSIMPLECONNECTOR_H
