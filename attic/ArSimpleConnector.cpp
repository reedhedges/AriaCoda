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
#include "ArSimpleConnector.h"
#include "ArRobot.h"
#include "ArLaser.h"
#include "ArSick.h"
#include "ariaInternal.h"

AREXPORT ArSimpleConnector::ArSimpleConnector(int *argc, char **argv) 
{
  myParser = new ArArgumentParser(argc, argv);
  myOwnParser = true;
  finishConstructor();
}

/** @warning do not delete @a builder during the lifetime of this
 * ArSimpleConnector, which may need to access its contents later.
 */
AREXPORT ArSimpleConnector::ArSimpleConnector(ArArgumentBuilder *builder)
{
  myParser = new ArArgumentParser(builder);
  myOwnParser = true;
  finishConstructor();
}

/** @warning do not delete @a parser during the lifetime of this
 * ArSimpleConnector, which may need to access its contents later.
 */
AREXPORT ArSimpleConnector::ArSimpleConnector(ArArgumentParser *parser) 
{
  myParser = parser;
  myOwnParser = false;
  finishConstructor();
}

AREXPORT ArSimpleConnector::~ArSimpleConnector(void)
{

}


void ArSimpleConnector::finishConstructor(void)
{
  myRobotConnector = new ArRobotConnector(myParser, NULL);
  myLaserConnector = new ArLaserConnector(myParser, NULL, myRobotConnector);
  setMaxNumLasers();
  //myLaserConnector->addLaser
}

AREXPORT void ArSimpleConnector::setMaxNumLasers(int maxNumLasers)
{
  int i;
  for (i = 1; i <= maxNumLasers; i++)
    myLaserConnector->addPlaceholderLaser(new ArSick, i, true);

}

AREXPORT bool ArSimpleConnector::parseArgs(void)
{
  return parseArgs(myParser);
}

/**
 * Parse command line arguments held by the given ArArgumentParser.
 *
  @return true if the arguments were parsed successfully false if not

   The following arguments are used for the robot connection:

   <dl>
    <dt><code>-robotPort</code> <i>port</i></dt>
    <dt><code>-rp</code> <i>port</i></dt>
    <dd>Use the given serial port device name for a serial port connection (e.g. <code>COM1</code>, or <code>/dev/ttyS0</code> if on Linux.)
    The default is the first serial port, or COM1, which is the typical Pioneer setup.
    </dd>

    <dt><code>-remoteHost</code> <i>hostname</i></dt>
    <dt><code>-rh</code> <i>hostname</i></dt>
    <dd>Use a TCP connection to a remote computer with the given network host name instead of a serial port connection</dd>

    <dt><code>-remoteRobotTcpPort</code> <i>port</i></dt>
    <dt><code>-rrtp</code> <i>port</i></dt>
    <dd>Use the given TCP port number if connecting to a remote robot using TCP due to <code>-remoteHost</code> having been given.</dd>

    <dt><code>-remoteIsSim</code></dt>
    <dt><code>-ris</code></dt>
    <dd>The remote TCP robot given by <code>-remoteHost</code> or <code>-rh</code> is actually a simulator. Use any alternative
     behavior intended for the simulator (e.g. tell the laser device object to request laser data from the simulator rather
     than trying to connect to a real laser device on the local computer)</dd>

    <dt><code>-robotBaud</code> <i>baudrate</i></dt>
    <dt><code>-rb</code> <i>baudrate</i></dt>
    <dd>Use the given baud rate when connecting over a serial port, instead of trying to use the normal rate.</dd>
  </dl>

  The following arguments are accepted for laser connections.  A program may request support for more than one laser
  using setMaxNumLasers(); if multi-laser support is enabled in this way, then these arguments must have the laser index
  number appended. For example, "-laserPort" for laser 1 would instead by "-laserPort1", and for laser 2 it would be
  "-laserPort2".

  <dl>
    <dt>-laserPort <i>port</i></dt>
    <dt>-lp <i>port</i></dt>
    <dd>Use the given port device name when connecting to a laser. For example, <code>COM2</code> or on Linux, <code>/dev/ttyS1</code>.
    The default laser port is COM2, which is the typical Pioneer laser port setup.
    </dd>

    <dt>-laserFlipped <i>true|false</i></dt>
    <dt>-lf <i>true|false</i></dt>
    <dd>If <code>true</code>, then the laser is mounted upside-down on the robot and the ordering of readings
    should be reversed.</dd>

    <dt>-connectLaser</dt>
    <dt>-cl</dt>
    <dd>Explicitly request that the client program connect to a laser, if it does not always do so</dd>

    <dt>-laserPowerControlled <i>true|false</i></dt>
    <dt>-lpc <i>true|false</i></dt>
    <dd>If <code>true</code>, then the laser is powered on when the serial port is initially opened, so enable
    certain features when connecting such as a waiting period as the laser initializes.</dd>

    <dt>-laserDegrees <i>degrees</i></dt>
    <dt>-ld <i>degrees</i></dt>
    <dd>Indicate the size of the laser field of view, either <code>180</code> (default) or <code>100</code>.</dd>

    <dt>-laserIncrement <i>increment</i></dt>
    <dt>-li <i>increment</i></dt>
    <dd>Configures the laser's angular resolution. If <code>one</code>, then configure the laser to take a reading every degree.
     If <code>half</code>, then configure it for a reading every 1/2 degrees.</dd>

    <dt>-laserUnits <i>units</i></dt>
    <dt>-lu <i>units</i></dt>
    <dd>Configures the laser's range resolution.  May be 1mm for one milimiter, 1cm for ten milimeters, or 10cm for one hundred milimeters.</dd>

    <dt>-laserReflectorBits <i>bits</i></dt>
    <dt>-lrb <i>bits</i></dt>
    <dd>Enables special reflectance detection, and configures the granularity of reflector detection information. Using more bits allows the laser to provide values for several different
    reflectance levels, but also may force a reduction in range.  (Note, the SICK LMS-200 only detects high reflectance on special reflector material
    manufactured by SICK.)
    </dd>
  </dl>

 **/

AREXPORT bool ArSimpleConnector::parseArgs(ArArgumentParser *parser)
{
  return myRobotConnector->parseArgs() && myLaserConnector->parseArgs();
}

AREXPORT void ArSimpleConnector::logOptions(void) const
{
  myRobotConnector->logOptions();
  myLaserConnector->logOptions();
}


/**
 * This method is normally used internally by connectRobot(), but you may 
 * use it if you wish.
 *
 * If -remoteHost was given, then open that TCP port. If it was not given,
 * then try to open a TCP port to the simulator on localhost.
 * If that fails, then use a local serial port connection.
 * Sets the given ArRobot's device connection pointer to this object.
 * Sets up internal settings determined by command line arguments such
 * as serial port and baud rate, etc.
 *
 * After calling this function  (and it returns true), then you may connect
 * ArRobot to the robot using ArRobot::blockingConnect() (or similar).
 *
 * @return false if -remoteHost was given and there was an error connecting to
 * the remote host, true otherwise.
 **/
AREXPORT bool ArSimpleConnector::setupRobot(ArRobot *robot)
{
  return myRobotConnector->setupRobot(robot);
}

/** Prepares the given ArRobot object for connection, then begins
 * a blocking connection attempt.
 * If you wish to simply prepare the ArRobot object, but not begin
 * the connection, then use setupRobot().
 */
AREXPORT bool ArSimpleConnector::connectRobot(ArRobot *robot)
{
  return myRobotConnector->connectRobot(robot);
}

/**
   Description of the logic for connection to the laser:  If
   --remoteHost then the laser will a tcp connection will be opened to
   that remoteHost at port 8102 or --remoteLaserTcpPort if that
   argument is given, if this connection fails then the setup fails.
   If --remoteHost wasn't provided and the robot connected to a
   simulator as described elsewhere then the laser is just configured
   to be simulated, if the robot isn't connected to a simulator it
   tries to open a serial connection to ArUtil::COM3 or --laserPort if
   that argument is given.
**/

AREXPORT bool ArSimpleConnector::setupLaser(ArSick *laser)
{
  return myLaserConnector->setupLaser(laser, 1);
}

/**
   Description of the logic for connecting to a second laser:  Given
   the fact that there are no parameters for the location of a second
   laser, the laser's port must be passed in to ArSimpleConnector from
   the main or from ArArgumentBuilder.  Similarly, a tcp connection must
   be explicitly defined with the --remoteLaserTcpPort2 argument.
**/
AREXPORT bool ArSimpleConnector::setupSecondLaser(ArSick *laser)
{
  return myLaserConnector->setupLaser(laser, 2);
}

AREXPORT bool ArSimpleConnector::setupLaserArbitrary(ArSick *laser,
						    int laserNumber)
{
  return myLaserConnector->setupLaser(laser, laserNumber);
}

/**
   This will setup and connect the laser if the command line switch
   was given to do so or simply return true if no connection was
   wanted.
**/

AREXPORT bool ArSimpleConnector::connectLaser(ArSick *laser)
{
  return myLaserConnector->connectLaser(laser, 1, false);
}

/**
   This will setup and connect the laser if the command line switch
   was given to do so or simply return true if no connection was
   requested.
**/
AREXPORT bool ArSimpleConnector::connectSecondLaser(ArSick *laser)
{
  return myLaserConnector->connectLaser(laser, 1, false);
}

AREXPORT bool ArSimpleConnector::connectLaserArbitrary(
	ArSick *laser, int laserNumber)
{
  return myLaserConnector->connectLaser(laser, laserNumber, false);
}
