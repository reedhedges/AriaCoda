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
#ifndef ARSIMULATEDLASER_H
#define ARSIMULATEDLASER_H

#include "ariaTypedefs.h"
#include "ArLaser.h"

class ArRobot;
class ArRobotPacket;

/**
   This class is a subclass of ArRangeDeviceThreaded meant for any
   planar scanning lasers, like the SICK lasers, Hokoyo URG series
   lasers, etc.  Unlike most base classes this contains the superset
   of everything that may need to be configured on any of the sensors,
   this is so that the configuration and parameter files don't have to
   deal with anything sensor specific.

   To see the different things you can set on a laser, call the
   functions canSetDegrees, canChooseRange, canSetIncrement,
   canChooseIncrement, canChooseUnits, canChooseReflectorBits,
   canSetPowerControlled, canChooseStartBaud, and canChooseAutoBaud to
   see what is available (the help for each of those tells you what
   functions they are associated with, and for each function
   associated with one of those it tells you to see the associated
   function).

   @since 2.7.0
**/

class ArSimulatedLaser : public ArLaser
{
public:
  /// Constructor
  AREXPORT ArSimulatedLaser(ArLaser *laser);
  /// Destructor
  AREXPORT virtual ~ArSimulatedLaser();

  AREXPORT virtual bool blockingConnect(void);
  AREXPORT virtual bool asyncConnect(void);
  AREXPORT virtual bool disconnect(void);
  AREXPORT virtual bool isConnected(void)
    { return myIsConnected; }
  AREXPORT virtual bool isTryingToConnect(void)
    { 
      if (myStartConnect)
	return true;
      else if (myTryingToConnect)
	return true; 
      else
	return false;
    }  

protected:
  AREXPORT virtual void * runThread(void *arg);
  AREXPORT virtual bool laserCheckParams(void);
  AREXPORT bool finishParams(void);
  AREXPORT bool simPacketHandler(ArRobotPacket *packet);
  ArLaser *myLaser;

  double mySimBegin;
  double mySimEnd;
  double mySimIncrement;

  // stuff for the sim packet
  ArPose mySimPacketStart;
  ArTransform mySimPacketTrans;
  ArTransform mySimPacketEncoderTrans;
  unsigned int mySimPacketCounter;
  unsigned int myWhichReading;
  unsigned int myTotalNumReadings;

  bool myStartConnect;
  bool myIsConnected;
  bool myTryingToConnect;
  bool myReceivedData;

  std::list<ArSensorReading *>::iterator myIter;
  // range buffers to hold current range set and assembling range set
  std::list<ArSensorReading *> *myAssembleReadings;
  std::list<ArSensorReading *> *myCurrentReadings;

  ArRetFunctor1C<bool, ArSimulatedLaser, ArRobotPacket *> mySimPacketHandler;
};

#endif // ARSIMULATEDLASER_H
