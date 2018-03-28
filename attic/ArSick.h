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
#ifndef ARSICK_H
#define ARSICK_H

#include "ariaTypedefs.h"
#include "ArRobotPacket.h"
#include "ArLaser.h"   
#include "ArFunctor.h"
#include "ArCondition.h"
#include "ArLMS2xx.h"


/**
Compatability class used to access SICK LMS-200 laser rangefinder device in
versions of ARIA prior to 2.7.0; used alone or with ArSimpleConnector (also
deprecated).  In 2.7.0 and later, ArSick implements the same
API as in previous versions, but serves only as an interface to ArLMS2xx; the
preferred way to access laser rangefinders is to first connect to a robot using
ArRobotConnector to load robot parameter files specifying laser types
and options, then use ArLaserConnector to connect to all lasers the robot has.
ArRobot stores a list of ArLaser objects for these lasers.
@deprecated
*/
class ArSick : public ArLMS2xx
{
public:
  /// Constructor
  AREXPORT ArSick(size_t currentBufferSize = 361, 
		  size_t cumulativeBufferSize = 0,
		  const char *name = "laser",
		  bool addAriaExitCB = true,
		  bool isSecondLaser = false);
  /// Destructor
  AREXPORT virtual ~ArSick();

  enum BaudRate {
    BAUD9600, ///< 9600 Baud
    BAUD19200, ///< 19200 Baud
    BAUD38400, ///< 38400 Baud
    BAUD_INVALID ///< Invalid baud
  };
  enum Degrees {
    DEGREES180, ///< 180 Degrees
    DEGREES100, ///< 100 Degrees
    DEGREES_INVALID ///< Invalid degrees
  };
  enum Increment {
    INCREMENT_ONE, ///< One degree increments
    INCREMENT_HALF, ///< Half a degree increments
    INCREMENT_INVALID ///< Increment invalid
  };
  enum Units {
    UNITS_1MM, ///< Uses 1 mm resolution (8/16/32 meter max range)
    UNITS_1CM, ///< Uses 1 cm resolution (80/160/320 meter max range)
    UNITS_10CM, ///< Uses 10 cm resolution (150 meter max range)
    UNITS_INVALID ///< Invalid units
  };
  enum Bits {
    BITS_1REFLECTOR, ///< Uses 1 reflector bits (means 32/320/150 meter range)
    BITS_2REFLECTOR, ///< Uses 2 reflector bits (means 16/160/150 meter range)
    BITS_3REFLECTOR, ///< Uses 3 reflector bits (means 8/80/150 meter range)
    BITS_INVALID ///< Invalid bits
  };

  /// Sees if this is trying to connect to the laser at the moment (note no other range devices work like this so you probably shouldn't use this)
  bool tryingToConnect(void) 
    { return isTryingToConnect(); }

  /// Runs the laser off of the robot
  bool runOnRobot(void) { return internalRunOnRobot(); }


  /// Use this to manually configure the laser before connecting to it
  AREXPORT void configure(bool useSim = false, bool powerControl = true, 
			  bool laserFlipped = false, 
			  BaudRate baud = BAUD38400,
			  Degrees deg = DEGREES180, 
			  Increment incr = INCREMENT_ONE);
  /// Shorter configure for the laser (settings are overridden by the .p file)
  AREXPORT void configureShort(bool useSim = false, 
			       BaudRate baud = BAUD38400,
			       Degrees deg = DEGREES180, 
			       Increment incr = INCREMENT_ONE);
  /// Sets the ranging/reflector information
  AREXPORT void setRangeInformation(Bits bits = BITS_1REFLECTOR,
				    Units units = UNITS_1MM);


  // Gets whether the laser is simulated or not
  AREXPORT bool isUsingSim(void);
  /// Gets whether the computer is controling laser power or not
  AREXPORT bool isControllingPower(void);
  /// Gets whether the laser is flipped over or not
  AREXPORT bool isLaserFlipped(void);
  /// Gets the degrees the laser is scanning
  AREXPORT Degrees getDegrees(void);
#ifndef SWIG
        // this ends up causing problems in the Java wrapper, since it has the
        // wrong return type vs. ArLaser::getIncrement(). But we need to keep
        // this function for backwards compatability.
  /// Gets the amount each scan increments
  AREXPORT Increment getIncrement(void);
#endif
  /// Gets the Bits the laser is using
  AREXPORT Bits getBits(void);
  /// Gets the Units the laser is using
  AREXPORT Units getUnits(void);

  // Sets whether the laser is simulated or not
  AREXPORT void setIsUsingSim(bool usingSim);
  /// Sets whether the computer is controling laser power or not
  AREXPORT void setIsControllingPower(bool controlPower);
  /// Sets whether the laser is flipped over or not
  AREXPORT void setIsLaserFlipped(bool laserFlipped);



  /** You should use setCumulativeCleanDist for this now
     @deprecated
  */
  void setFilterCumulativeCleanDist(double dist)
    {
      setCumulativeCleanDist(dist);
    }
  /** You should use getCumulativeCleanDist for this now
     @deprecated
  */
  double getFilterCumulativeCleanDist(void)
    {
      return getCumulativeCleanDist();
    }
  /** You should use setCumulativeCleanInterval for this now
     @deprecated
  */
  void setFilterCleanCumulativeInterval(int milliSeconds)
    {
      setCumulativeCleanInterval(milliSeconds);
    }
  /** You should use getPlanarCumulativeCleanInterval for this now
     @deprecated
  */
  int getFilterCleanCumulativeInterval(void)
    {
      return getCumulativeCleanInterval();
    }

  /** You should use setMinDistBetweenCumulative for this now
      @deprecated
  */
  void setFilterCumulativeNearDist(double dist)
    {
      setMinDistBetweenCumulative(dist);
    }
  /** You should use getMinDistBetweenCumulative for this now
      @deprecated
  */
  double getFilterCumulativeNearDist(void)
    { return getMinDistBetweenCumulative(); }

  /** You should use setMinDistBetweenCurrent for this now
      @deprecated
  */
  void setFilterNearDist(double dist) 
    { setMinDistBetweenCurrent(dist); }
  /** You should use getMinDistBetweenCurrent for this now
      @deprecated
  */
  double getFilterNearDist(void) 
    { return getMinDistBetweenCurrent(); }

  /**
     You should use setMaxInsertDistCumulative for this now
     @deprecated
  */
  void setFilterCumulativeInsertMaxDist(double dist)
    { setMaxInsertDistCumulative(dist); }
  /**
     You should use getMaxInsertDistCumulative for this now
     @deprecated
  */
  double getFilterCumulativeInsertMaxDist(void)
    { return getMaxInsertDistCumulative(); }

  /** Used to set the min range, this is no longer used because it
      doesn't actually solve anything (because beams that see near
      things and far things average)
      @deprecated
   **/
  unsigned int getMinRange(void) { return 0; } 
  /** Used to get the min range, this is no longer used because it
      doesn't actually solve anything (because beams that see near
      things and far things average)
      @deprecated
   **/ 
  void setMinRange(unsigned int minRange) { }

  /** You should see setMaxDistToKeepCumulative (it replaces this)
      @deprecated
  */
  void setFilterCumulativeMaxDist(double dist) 
    { setMaxDistToKeepCumulative(dist); }
  /** You should see getMaxDistToKeepCumulative (it replaces this)
      @deprecated
   */
  double getFilterCumulativeMaxDist(void)
    { return getMaxDistToKeepCumulative(); }
  /** you should use setMaxSecondsToKeepCumulative instead, (replaces this)
   *  @deprecated
   */
  void setFilterCumulativeMaxAge(int seconds) 
    { setMaxSecondsToKeepCumulative(seconds); }
  /** you should use getMaxSecondsToKeepCumulative instead, (replaces this)
   *  @deprecated
   */
  int getFilterCumulativeMaxAge(void) 
    { return getMaxSecondsToKeepCumulative(); }

  /// Gets the number of laser packets received in the last second
  int getSickPacCount(void)
    { return getReadingCount(); }

  /// Adds a connect callback
  void addConnectCB(ArFunctor *functor,
			     ArListPos::Pos position = ArListPos::LAST)
    {
      if (position == ArListPos::FIRST)
	ArLMS2xx::addConnectCB(functor, 75);
      else if (position == ArListPos::LAST)
	ArLMS2xx::addConnectCB(functor, 25);
      else
	ArLMS2xx::addConnectCB(functor, 25);
    }
  /// Adds a disconnect callback
  void remConnectCB(ArFunctor *functor)
    {
      ArLMS2xx::remConnectCB(functor);
    }

  /// Adds a callback for when a connection to the robot is failed
  void addFailedConnectCB(ArFunctor *functor, 
				   ArListPos::Pos position = ArListPos::LAST)
    {
      if (position == ArListPos::FIRST)
	ArLMS2xx::addFailedConnectCB(functor, 75);
      else if (position == ArListPos::LAST)
	ArLMS2xx::addFailedConnectCB(functor, 25);
      else
	ArLMS2xx::addFailedConnectCB(functor, 25);
    }
  /// Removes a callback for when a connection to the robot is failed
  void remFailedConnectCB(ArFunctor *functor)
    {
      ArLMS2xx::remFailedConnectCB(functor);
    }

  /// Adds a callback for when disconnect is called while connected
  void addDisconnectNormallyCB(ArFunctor *functor, 
				ArListPos::Pos position = ArListPos::LAST)
    {
      if (position == ArListPos::FIRST)
	ArLMS2xx::addDisconnectNormallyCB(functor, 75);
      else if (position == ArListPos::LAST)
	ArLMS2xx::addDisconnectNormallyCB(functor, 25);
      else
	ArLMS2xx::addDisconnectNormallyCB(functor, 25);
    }
  /// Removes a callback for when disconnect is called while connected
   void remDisconnectNormallyCB(ArFunctor *functor)
    {
      ArLMS2xx::remDisconnectNormallyCB(functor);
    }
  
  /// Adds a callback for when disconnection happens because of an error
  void addDisconnectOnErrorCB(ArFunctor *functor, 
			       ArListPos::Pos position = ArListPos::LAST)
    {
      if (position == ArListPos::FIRST)
	ArLMS2xx::addDisconnectOnErrorCB(functor, 75);
      else if (position == ArListPos::LAST)
	ArLMS2xx::addDisconnectOnErrorCB(functor, 25);
      else
	ArLMS2xx::addDisconnectOnErrorCB(functor, 25);
    }
  /// Removes a callback for when disconnection happens because of an error
  void remDisconnectOnErrorCB(ArFunctor *functor)
    {
      ArLMS2xx::remDisconnectOnErrorCB(functor);
    }
  /// Adds a callback that is called whenever a laser packet is processed
  void addDataCB(ArFunctor *functor,
			  ArListPos::Pos position = ArListPos::LAST)
    {
      if (position == ArListPos::FIRST)
	ArLMS2xx::addReadingCB(functor, 75);
      else if (position == ArListPos::LAST)
	ArLMS2xx::addReadingCB(functor, 25);
      else
	ArLMS2xx::addReadingCB(functor, 25);
    }
  /// Removes a callback that is called whenever a laser packet is processed
  void remDataCB(ArFunctor *functor)
    {
      ArLMS2xx::remReadingCB(functor);
    }

  /// Sets the time without a response until connection assumed lost
  void setConnectionTimeoutTime(int secs)
    { setConnectionTimeoutSeconds(secs); }
  /// Gets the time without a response until connection assumed lost
  int getConnectionTimeoutTime(void)
    { return ArMath::roundInt(getConnectionTimeoutSeconds()); }

};


#endif //ARSICK_H
