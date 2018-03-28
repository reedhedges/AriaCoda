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
#ifndef ARLMS2XX_H
#define ARLMS2XX_H


#include "ariaTypedefs.h"
#include "ArLMS2xxPacket.h"
#include "ArLMS2xxPacketReceiver.h"
#include "ArRobotPacket.h"
#include "ArLaser.h"   
#include "ArFunctor.h"
#include "ArCondition.h"

/// Interface to a SICK LMS-200 laser range device
/**
 * This class processes incoming data from a SICK LMS-200
 * laser rangefinding device in a background thread, and provides
 * it through the standard ArRangeDevice API, to be used via ArRobot
 * (see ArRobot::addRangeDevice()), used by an ArAction, or used directly.
 *
 * Instances are intended to be created vi ArLaserConnector. 
 *
 * @note The "extra int" on the raw readings returned by
 * ArRangeDevice::getRawReadings() is like other laser
 * devices and is the reflectance value, if enabled, ranging between 0 and 255.
 *
 * ArLMS2xx uses the following buffer parameters by default (see ArRangeDevice
 * documentation):
 * <dl>
 *  <dt>MinDistBetweenCurrent <dd>50 mm
 *  <dt>MaxDistToKeepCumulative <dd>6000 mm
 *  <dt>MinDistBetweenCumulative <dd>200 mm
 *  <dt>MaxSecondsToKeepCumulative <dd>30 sec
 *  <dt>MaxINsertDistCumulative <dd>3000 mm
 * </dl>
 * The current buffer is replaced for each new set of readings.
 *
 * @see ArLaserConnector
 * @see ArLaser
 * @see ArRangeDevice
 *
 * @since 2.7.0
**/
class ArLMS2xx : public ArLaser
{
public:
  /// Constructor
  AREXPORT ArLMS2xx(int laserNumber,
		    const char *name = "lms2xx",
		    bool appendLaserNumberToName = true);

  /// Destructor
  AREXPORT virtual ~ArLMS2xx();

  /// Connect to the laser while blocking
  AREXPORT virtual bool blockingConnect(void);
  /// Connect to the laser asyncronously
  AREXPORT bool asyncConnect(void);
  /// Disconnect from the laser
  AREXPORT virtual bool disconnect(void);
  /// Sees if this is connected to the laser
  AREXPORT virtual bool isConnected(void) 
    { if (myState == STATE_CONNECTED) return true; else return false; }
  AREXPORT virtual bool isTryingToConnect(void) 
    { 
      if (myState != STATE_CONNECTED && myState != STATE_NONE) 
	return true; 
      else if (myStartConnect)
	return true;
      else 
	return false; 
    }

  /// Sets the device connection
  AREXPORT virtual void setDeviceConnection(ArDeviceConnection *conn);

  /** The internal function used by the ArRangeDeviceThreaded
   *  @internal
   */
  AREXPORT virtual void * runThread(void *arg);
  AREXPORT virtual void setRobot(ArRobot *robot);
protected:
  // The packet handler for when connected to the simulator
  AREXPORT bool simPacketHandler(ArRobotPacket * packet);
  // The function called if the laser isn't running in its own thread and isn't simulated
  AREXPORT void sensorInterpCallback(void);
  // An internal function for connecting to the sim
  AREXPORT bool internalConnectSim(void);
  /// An internal function, single loop event to connect to laser
  AREXPORT int internalConnectHandler(void);
  // The internal function which processes the sickPackets
  AREXPORT void processPacket(ArLMS2xxPacket *packet, ArPose pose, 
			      ArPose encoderPose, unsigned int counter,
			      bool deinterlace, ArPose deinterlaceDelta);
  // The internal function that gets does the work
  AREXPORT void runOnce(bool lockRobot);
  // Internal function, shouldn't be used, drops the conn because of error
  AREXPORT void dropConnection(void);
  // Internal function, shouldn't be used, denotes the conn failed
  AREXPORT void failedConnect(void);
  // Internal function, shouldn't be used, does the after conn stuff
  AREXPORT void madeConnection(void);

  /// Internal function that gets whether the laser is simulated or not (just for the old ArSick)
  AREXPORT bool sickGetIsUsingSim(void);

  /// Internal function that sets whether the laser is simulated or not (just for the old ArSick)
  AREXPORT void sickSetIsUsingSim(bool usingSim);

  /// internal function to runOnRobot so that ArSick can do that while this class won't
  AREXPORT bool internalRunOnRobot(void);

  /// Finishes getting the unset parameters from the robot then
  /// setting some internal variables that need it
  bool finishParams(void);

  AREXPORT virtual bool laserCheckParams(void);

  AREXPORT virtual void laserSetName(const char *name);

  enum State {
    STATE_NONE, ///< Nothing, haven't tried to connect or anything
    STATE_INIT, ///< Initializing the laser
    STATE_WAIT_FOR_POWER_ON, ///< Waiting for power on
    STATE_CHANGE_BAUD, ///< Change the baud, no confirm here
    STATE_CONFIGURE, ///< Send the width and increment to the laser
    STATE_WAIT_FOR_CONFIGURE_ACK, ///< Wait for the configuration Ack
    STATE_INSTALL_MODE, ///< Switch to install mode
    STATE_WAIT_FOR_INSTALL_MODE_ACK, ///< Wait until its switched to install mode
    STATE_SET_MODE, ///< Set the mode (mm/cm) and extra field bits
    STATE_WAIT_FOR_SET_MODE_ACK, ///< Waiting for set-mode ack
    STATE_START_READINGS, ///< Switch to monitoring mode
    STATE_WAIT_FOR_START_ACK, ///< Waiting for the switch-mode ack
    STATE_CONNECTED ///< We're connected and getting readings
  };
  /// Internal function for switching states
  AREXPORT void switchState(State state);
  State myState;
  ArTime myStateStart;
  ArFunctorC<ArLMS2xx> myRobotConnectCB;
  ArRetFunctor1C<bool, ArLMS2xx, ArRobotPacket *> mySimPacketHandler;
  ArFunctorC<ArLMS2xx> mySensorInterpCB;
  std::list<ArSensorReading *>::iterator myIter;
  bool myStartConnect;
  bool myRunningOnRobot;

  // range buffers to hold current range set and assembling range set
  std::list<ArSensorReading *> *myAssembleReadings;
  std::list<ArSensorReading *> *myCurrentReadings;

  bool myProcessImmediately;
  bool myInterpolation;
  // list of packets, so we can process them from the sensor callback
  std::list<ArLMS2xxPacket *> myPackets;

  // these two are just for the sim packets
  unsigned int myWhichReading;
  unsigned int myTotalNumReadings;

  // some variables so we don't have to do a tedios if every time
  double myOffsetAmount;
  double myIncrementAmount;

  // packet stuff
  ArLMS2xxPacket myPacket;
  bool myUseSim;
  
  int myNumReflectorBits;
  bool myInterlaced;

  // stuff for the sim packet
  ArPose mySimPacketStart;
  ArTransform mySimPacketTrans;
  ArTransform mySimPacketEncoderTrans;
  unsigned int mySimPacketCounter;

  // connection
  ArLMS2xxPacketReceiver myLMS2xxPacketReceiver;

  ArMutex myStateMutex;
  ArRetFunctorC<bool, ArLMS2xx> myAriaExitCB;
};


#endif 
