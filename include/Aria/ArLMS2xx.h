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
#ifndef ARLMS2XX_H
#define ARLMS2XX_H


#include "Aria/ariaTypedefs.h"
#include "Aria/ArLMS2xxPacket.h"
#include "Aria/ArLMS2xxPacketReceiver.h"
#include "Aria/ArRobotPacket.h"
#include "Aria/ArLaser.h"   
#include "Aria/ArFunctor.h"
#include "Aria/ArCondition.h"


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
 * @since 2.7.0 (previously named ArSick)
**/
class ArLMS2xx final : public ArLaser
{
public:
  /// Constructor
  AREXPORT explicit ArLMS2xx(int laserNumber,
		    const char *name = "lms2xx",
		    bool appendLaserNumberToName = true);

  /// Destructor
  AREXPORT virtual ~ArLMS2xx();

  /// Connect to the laser while blocking
  AREXPORT virtual bool blockingConnect() override;
  /// Connect to the laser asynchronously
  AREXPORT bool asyncConnect();
  /// Disconnect from the laser
  AREXPORT virtual bool disconnect() override;
  /// Sees if this is connected to the laser
  AREXPORT virtual bool isConnected() override
    { if (myState == STATE_CONNECTED) return true; else return false; }
  AREXPORT virtual bool isTryingToConnect() override 
    { 
      if (myState != STATE_CONNECTED && myState != STATE_NONE) 
	return true; 
      else if (myStartConnect)
	return true;
      else 
	return false; 
    }

  /// Sets the device connection
  AREXPORT virtual void setDeviceConnection(ArDeviceConnection *conn) override;

#ifndef ARIA_WRAPPER
  /** The internal function used by the ArRangeDeviceThreaded
   *  @internal
   */
  AREXPORT virtual void * runThread(void *arg) override;
#endif

  AREXPORT virtual void setRobot(ArRobot *robot) override;

private:
  // The packet handler for when connected to the simulator
  bool simPacketHandler(ArRobotPacket * packet);
  // The function called if the laser isn't running in its own thread and isn't simulated
  void sensorInterpCallback();
  // An internal function for connecting to the sim
  bool internalConnectSim();
  /// An internal function, single loop event to connect to laser
  int internalConnectHandler();
  // The internal function which processes the sickPackets
  void processPacket(ArLMS2xxPacket *packet, ArPose pose, 
			      ArPose encoderPose, unsigned int counter,
			      bool deinterlace, ArPose deinterlaceDelta);
  // The internal function that gets does the work
  void runOnce(bool lockRobot);
  // Internal function, shouldn't be used, drops the conn because of error
  void dropConnection();
  // Internal function, shouldn't be used, denotes the conn failed
  void failedConnect();
  // Internal function, shouldn't be used, does the after conn stuff
  void madeConnection();

  /// Internal function that gets whether the laser is simulated or not (just for the old ArSick)
  bool sickGetIsUsingSim();

  /// Internal function that sets whether the laser is simulated or not (just for the old ArSick)
  void sickSetIsUsingSim(bool usingSim);

  /// internal function to runOnRobot so that ArSick can do that while this class won't
  bool internalRunOnRobot();

  /// Finishes getting the unset parameters from the robot then
  /// setting some internal variables that need it
  bool finishParams();

  virtual bool laserCheckParams() override;

  virtual void laserSetName(const char *name) override;

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
  void switchState(State state);
  State myState;
  ArTime myStateStart;
  ArFunctorC<ArLMS2xx> myRobotConnectCB;
  ArRetFunctor1C<bool, ArLMS2xx, ArRobotPacket *> mySimPacketHandler;
  ArFunctorC<ArLMS2xx> mySensorInterpCB;
  bool myStartConnect;
  bool myRunningOnRobot;

  // range buffers to hold current range set and assembling range set
  // TODO change to pointers to member lists ratherh than allocating in
  // constructor.
  // TODO chang eto list<ArSensorReading> rather than list of pointers
  std::list<ArSensorReading *>::iterator myIter;
  std::list<ArSensorReading *> *myAssembleReadings;
  std::list<ArSensorReading *> *myCurrentReadings;

  bool myProcessImmediately;
  bool myInterpolation;
  // list of packets, so we can process them from the sensor callback
  std::list<ArLMS2xxPacket *> myPackets;

  // these two are just for the sim packets
  unsigned int myWhichReading;
  unsigned int myTotalNumReadings;

  // some variables so we don't have to do a tedious if every time
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
