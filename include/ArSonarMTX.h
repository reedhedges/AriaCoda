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
#ifndef ARSONARMTX_H
#define ARSONARMTX_H

#include "ariaTypedefs.h"
#include "ArRangeDevice.h"
#include "ArFunctor.h"
#include "ArRobot.h"
#include "ArRobotPacket.h"



// Packets are in the format of 
// 2 bytes header (0xfa 0xfb)
// 1 byte length
// 1 byte command
// xx bytes command specific / args
// 2 bytes checksum
//
// Alive packet 
//		request 	- 0xfa 0xfb 0x03 0x00 0x00 0x00
//		response 	- 0xfa 0xfb 0x03 0x00 0x00 0x00
//
// Start Scan packet 
//		request 	- 0xfa 0xfb 0x03 0x01 0x00 0x01
//		reading	 	- 0xfa 0xfb 0x07 0x01 0x0b 0x00 0xff 0xff 0x02 0xf5
//
// Stop Scan packet 
//		request 	- 0xfa 0xfb 0x03 0x02 0x00 0x02
//
// Reset packet
//		request 	- 0xfa 0xfb 0x03 0x03 0x00 0x03
//
// Get Number of Transducers (12 here)
//		request 	- 0xfa 0xfb 0x03 0x11 0x00 0x11
//		response 	- 0xfa 0xfb 0x04 0x11 0x0c 0x11 0x0c
//
// Get Echo Sample Size packet (also called max range)
//		request 	- 0xfa 0xfb 0x04 0x14 0x00 0x14 0x00
//		response 	- 0xfa 0xfb 0x06 0x14 0x01 0x00 0x01 0x14 0x01
//    note: byte 5 (0x01) is transducer # (starts at 0)
//    note: bytes 6 & 7 are value                              
//
// Get Gain packet 
//		request 	- 0xfa 0xfb 0x04 0x17 0x00 0x17 0x00
//		response 	- 0xfa 0xfb 0x05 0x17 0x00 0x05 0x17 0x04
//    note: byte 5 (0x01) is transducer # (starts at 0)
//    note: bytes 6 is gain                
//
// Get Mask
//		request 	- 0xfa 0xfb 0x03 0x12 0x00 0x12
//		response 	- 0xfa 0xfb 0x05 0x12 0xff 0x0f 0x12 0xf0
//
// Get Number of Thresholds
//		request 	- 0xfa 0xfb 0x03 0x18 0x00 0x18
//		response 	- 0xfa 0xfb 0x04 0x18 0x03 0x18 0x03
//
// Get Sonar Delay
//		request 	- 0xfa 0xfb 0x03 0x13 0x00 0x13
//		response 	- 0xfa 0xfb 0x04 0x13 0x01 0x13 0x01
//
// Get Thresholds
//		request 	- 0xfa 0xfb 0x04 0x19 0x00 0x19 0x00
//		response 	- 0xfa 0xfb 0x0a 0x19 0x00 0xb8 0x0b 0xdc 0x05 0xd0 0x07 0x7d 0x17
//		note: byte 5 (0x00) is transducer #
//
// Get Version
//		request 	- 0xfa 0xfb 0x03 0x10 0x00 0x10
//		response 	- 0xfa 0xfb 0x04 0x10 0x01 0x10 0x01
//
// Set Echo Sample Size (also set max range)
//		request 	- 0xfa 0xfb 0x06 0x24 0x00 0x02 0x00 0x26 0x00
//		note: byte 5 (0x00) is transducer #
//		note: there is no response - use get echo sample size to test
//
// Set Gain
//		request 	- 0xfa 0xfb 0x05 0x27 0x00 0x05 0x27 0x05
//		note: byte 5 (0x00) is transducer #, gain is byte 6
//		note: there is no response - use get gain to test
//
// Set Mask
//		request 	- 0xfa 0xfb 0x05 0x22 0xc0 0x81 0x22 0x41
//		note: byte 5 & 6 are the masks
//		note: there is no response - use get mask test
//
// Set Sonar Delay
//		request 	- 0xfa 0xfb 0x04 0x23 0xd9 0x23 0xd9
//		note: there is no response - use get sonar delay to test
//
// Set Thresholds
//		request 	- 0xfa 0xfb 0x0a 0x29 0x00 0x38 0xae 0x06 0xaf 0xc9 0x63 0x31 0xc0
//		note: byte 5 is transducer number
//		note: there is no response - use get thresholds to test
//


/// Receives sonar data from an MTX robot
/// Use ArSonarConnector to establish the connection and create and initiate the ArSonarMTX thread.
/// @since 2.8.0
class ArSonarMTX : public ArASyncTask
{
public:
  /// Constructor
  AREXPORT ArSonarMTX( 
			  int sonarBoardNum = 0,
				const char * name = "MTXSonar", ArDeviceConnection *conn = NULL,
			 ArRobot *robot = NULL);
  /// Destructor
  AREXPORT virtual ~ArSonarMTX();

  /// Sets the robot pointer, also attaches its process function to the
  /// robot as a Sensor Interpretation task.
  AREXPORT virtual void setRobot(ArRobot *robot);

	int getBoardNum(void)
		{ return myBoardNum; }

  /// Sets the device this instance receives packets from
  AREXPORT void setDeviceConnection(ArDeviceConnection *conn);
  /// Gets the device this instance receives packets from
  AREXPORT ArDeviceConnection *getDeviceConnection(void);

  /// Very Internal call that gets the packet sender, shouldn't be used
  ArRobotPacketSender *getPacketSender(void)
    { return mySender; }
  /// Very Internal call that gets the packet sender, shouldn't be used
  ArRobotPacketReceiver *getPacketReceiver(void)
    { return myReceiver; }

  AREXPORT virtual bool blockingConnect(bool sendTracking, bool recvTracking);
	/// Connect used for debug replay
  AREXPORT virtual bool fakeConnect();
  AREXPORT virtual bool disconnect(void);
  virtual bool isConnected(void) { return myIsConnected; }
	virtual bool isTryingToConnect (void)
	{
		if (myStartConnect)
			return true;
		else if (myTryingToConnect)
			return true;
		else
			return false;
	}
  /// Logs the information about the sensor
  AREXPORT void log(void);

  /// Lock this device
  virtual int lockDevice() { return(myDeviceMutex.lock());}
  /// Try to lock this device
  virtual int tryLockDevice() {return(myDeviceMutex.tryLock());}
  /// Unlock this device
  virtual int unlockDevice() {return(myDeviceMutex.unlock());}

  AREXPORT virtual const char *getName(void) const;

  AREXPORT virtual const char *getNameWithBoard(void) const;

  void	setInfoLogLevel(ArLog::LogLevel infoLogLevel)
  { myInfoLogLevel = infoLogLevel; }

  /// Gets the default port type for the sonar
  const char *getDefaultPortType(void) { return myDefaultPortType.c_str(); }

  /// Gets the default port type for the sonar
  const char *getDefaultTcpPort(void) { return myDefaultTcpPort.c_str(); }

  /// Sets the numter of seconds without a response until connection assumed lost
  virtual void setConnectionTimeoutSeconds(double seconds)
    { ArLog::log(ArLog::Normal, 
		 "%s::setConnectionTimeoutSeconds: Setting timeout to %g secs", 
		 getName(), seconds);
      myLastReading.setToNow(); myTimeoutSeconds = seconds; }
  /// Gets the number of seconds without a response until connection assumed lost
  AREXPORT virtual double getConnectionTimeoutSeconds(void)
	{return myTimeoutSeconds; }
	/// check for lost connections
	AREXPORT bool checkLostConnection(void);
	/// disconnect 
	AREXPORT void disconnectOnError(void);
  /// Gets the time data was last receieved
  ArTime getLastReadingTime(void) { return myLastReading; }
  /// Gets the number of sonar readings received in the last second
  AREXPORT int getReadingCount(void);
  // Function called in sensorInterp to indicate that a
  // reading was received
  AREXPORT virtual void internalGotReading(void);

protected:
  AREXPORT bool sendAlive();
  AREXPORT bool sendReset();
  AREXPORT bool sendStart();
  AREXPORT bool sendStop();
  AREXPORT bool sendGetTransducerCount();
  AREXPORT bool sendGetGain(unsigned char transducerNumber);
  AREXPORT bool sendSetGain(unsigned char transducerNumber, unsigned char gain);
  AREXPORT bool sendGetMaxRange(unsigned char transducerNumber);
  AREXPORT bool sendSetMaxRange(unsigned char transducerNumber, int echoSampleSize);
  AREXPORT bool sendGetThresholds(unsigned char transducerNumber);
  AREXPORT bool sendSetThresholds(unsigned char transducerNumber, int thres);
	AREXPORT bool sendGetNumThresholdRanges();
  /*
	AREXPORT bool sendGetNoiseDelta(unsigned char transducerNumber);
  AREXPORT bool sendSetNoiseDelta(unsigned char transducerNumber, int noiseDelta);
	*/
  AREXPORT bool sendGetDelay();
  AREXPORT bool sendSetDelay(unsigned char delay);
  AREXPORT bool sendSetMask(unsigned char maskLsb, unsigned char maskMsb);
  AREXPORT bool sendGetMask();
	AREXPORT bool validateTransducers();
	AREXPORT bool validateGain();
	AREXPORT bool validateDelay();
	AREXPORT bool validateThresholds();
	AREXPORT bool validateMaxRange();
	/*
	AREXPORT bool validateNoiseDelta();
	*/
	AREXPORT bool validateNumThresholdRanges();
  AREXPORT bool requestFirmwareVersion();
  AREXPORT bool queryFirmwareVersion();

public:
  /// Adds a callback for when disconnection happens because of an error
  void addDisconnectOnErrorCB(ArFunctor *functor, 
			     int position = 51) 
    { myDisconnectOnErrorCBList.addCallback(functor, position); }

  /// Removes a callback for when disconnection happens because of an error
  void remDisconnectOnErrorCB(ArFunctor *functor)
    { myDisconnectOnErrorCBList.remCallback(functor); }

  /// Number of Transducers from board query
	int getNumTransducers(void) const
		{ return myNumTransducers; }
  /// Number of Configured Transducers
	int getNumConfiguredTransducers(void) const
		{ return myNumConfiguredTransducers; }
  /// Board Delay
	int getBoardDelay(void) const
		{ return myBoardDelay; }
  /// 
	int getBoardGain(void) const
		{ return myBoardGain; }
  ///
	/*
	int getBoardNoiseDelta(void) const
    { return myBoardNoiseDelta; }
	*/
  /// 
	int getBoardDetectionThreshold(void) const
  { 
    return myBoardDetectionThreshold; 
  }

	int getBoardMaxRange(void) const
  { 
    return myBoardMaxRange; 
  }

	bool getBoardUseForAutonomousDriving(void) const
	{ 
    return myBoardUseForAutonomousDriving; 
  }
 
	int getUnitMapping(int unit) const
		{
		std::map<int, std::map<int, int> >::const_iterator iter = 
				mySonarMap.find(unit);
		if (iter == mySonarMap.end()) 
			return -1;
		else {
			std::map<int, int>unitMap = iter->second;
			return unitMap[SONAR_MAPPING];
		} 
  }

  /// 
	int getUnitX(int unit) const
		{
		std::map<int, std::map<int, int> >::const_iterator iter = 
				mySonarMap.find(unit);
		if (iter == mySonarMap.end()) 
			return -1;
		else {
			std::map<int, int>unitMap = iter->second;
			return unitMap[SONAR_X];
		} 
  }

  /// 
	int getUnitY(int unit) const
		{
		std::map<int, std::map<int, int> >::const_iterator iter = 
				mySonarMap.find(unit);
		if (iter == mySonarMap.end()) 
			return -1;
		else {
			std::map<int, int>unitMap = iter->second;
			return unitMap[SONAR_Y];
		} 
  }

  /// 
	int getUnitTh(int unit) const
		{
		std::map<int, std::map<int, int> >::const_iterator iter = 
				mySonarMap.find(unit);
		if (iter == mySonarMap.end()) 
			return -1;
		else {
			std::map<int, int>unitMap = iter->second;
			return unitMap[SONAR_TH];
		} 
  }

  /// 
	int getUnitGain(int unit) const
		{
		std::map<int, std::map<int, int> >::const_iterator iter = 
				mySonarMap.find(unit);
		if (iter == mySonarMap.end()) 
			return -1;
		else {
			std::map<int, int>unitMap = iter->second;
			return unitMap[SONAR_GAIN];
    } 
  }

  /// 
	int getUnitDetectionThres(int unit) const
		{
		std::map<int, std::map<int, int> >::const_iterator iter = 
				mySonarMap.find(unit);
		if (iter == mySonarMap.end()) 
			return -1;
		else {
			std::map<int, int>unitMap = iter->second;
			return unitMap[SONAR_DETECTION_THRES];
		} 
  }

  /// 
	/*
	int getUnitNoiseDelta(int unit) const
		{
		std::map<int, std::map<int, int> >::const_iterator iter = 
				mySonarMap.find(unit);
		if (iter == mySonarMap.end()) 
			return -1;
		else {
			std::map<int, int>unitMap = iter->second;
			return unitMap[SONAR_NOISE_DELTA];
		} }
	*/
#if 0
  /// 
	int getUnitThresClose(int unit) const
		{
		std::map<int, std::map<int, int> >::const_iterator iter = 
				mySonarMap.find(unit);
		if (iter == mySonarMap.end()) 
			return -1;
		else {
			std::map<int, int>unitMap = iter->second;
			return unitMap[SONAR_THRES_CLOSE];
    } 
  }

  /// 
	int getUnitThresMed(int unit) const
		{
		std::map<int, std::map<int, int> >::const_iterator iter = 
				mySonarMap.find(unit);
		if (iter == mySonarMap.end()) 
			return -1;
		else 
    {
			std::map<int, int>unitMap = iter->second;
			return unitMap[SONAR_THRES_MED];
		} 
  }

  /// 
	int getUnitThresFar(int unit) const
		{
		std::map<int, std::map<int, int> >::const_iterator iter = 
				mySonarMap.find(unit);
		if (iter == mySonarMap.end()) 
			return -1;
		else 
    {
			std::map<int, int>unitMap = iter->second;
			return unitMap[SONAR_THRES_FAR];
		} 
  }
#endif
 
	int getUnitLastReading(int unit) const
		{
		std::map<int, std::map<int, int> >::const_iterator iter = 
				mySonarMap.find(unit);
		if (iter == mySonarMap.end()) 
			return -1;
		else 
    {
			std::map<int, int>unitMap = iter->second;
			return unitMap[SONAR_LAST_READING];
		} 
  }

	int getFirmwareVersion(void) const
	{ 
    return myFirmwareVersion; 
  }

	AREXPORT bool turnOnTransducers();

	AREXPORT bool turnOffTransducers();

	AREXPORT bool disableForAutonomousDriving();

  void setPacketsSentTracking(bool v = true) {
    mySendTracking = v;
    mySendTrackingSet = true;
    if (mySender)
      mySender->setTracking(true);
  }

  void setPacketsReceivedTracking(bool v = true) {
    myRecvTracking = v;
    myRecvTrackingSet = true;
    if (myReceiver) 
       myReceiver->setTracking(true);
  }

enum Headers {
	HEADER1=0xfa,
	HEADER2=0xf5 
};



protected:
  ArDeviceConnection *myConn;

  std::string myName;
  char myNameWithBoard[100];
	std::string myDefaultPortType;
	std::string myDefaultTcpPort;

  double myTimeoutSeconds;
  bool myRobotRunningAndConnected;

	bool myTransducersAreOn;

  ArTime myLastReading;

  // packet count
  time_t myTimeLastReading;
  int myReadingCurrentCount;
  int myReadingCount;

  ArCallbackList myDisconnectOnErrorCBList;
	
	ArRobot *myRobot;
  ArFunctorC<ArSonarMTX> myProcessCB;

  AREXPORT virtual void sonarSetName(const char *name);
  AREXPORT virtual void * runThread(void *arg);
	
  void sensorInterp(void);
  void failedToConnect(void);
  void clear(void);
  bool myIsConnected;
  bool myTryingToConnect;
  bool myStartConnect;
	
	int myBoardNum;
	int myNumTransducers;
	int myNumConfiguredTransducers;
	unsigned char myVersion;
	//unsigned char mySonarDelay;
	bool myWarnedAboutExtraSonar;

	unsigned int myBoardDelay;
	unsigned int myBoardGain;
	/*
	unsigned int myBoardNoiseDelta;
	*/
	unsigned int myBoardDetectionThreshold;
	unsigned int myBoardMaxRange;

	bool myBoardUseForAutonomousDriving;

	unsigned char myTransducerMaskLSB;
	unsigned char myTransducerMaskMSB;

	unsigned char myAutonomousDrivingTransducerMaskLSB;
	unsigned char myAutonomousDrivingTransducerMaskMSB;

	// first index is transducer reletive to this board
	// second index is defined below 
  std::map<int, std::map<int, int> > mySonarMap;
  enum SonarInfo 
  { 
		SONAR_IS_CONFIGURED,
		SONAR_MAPPING,
		SONAR_X,
		SONAR_Y,
		SONAR_TH,
		SONAR_GAIN,
		/*
		SONAR_NOISE_DELTA,
		*/
		SONAR_DETECTION_THRES,
		SONAR_MAX_RANGE,
		SONAR_USE_FOR_AUTONOMOUS_DRIVING,
		SONAR_LAST_READING
  };

enum Sizes {
	maxTransducers=16
	};

enum Commands {
 ALIVE=0x00,
 START_SCAN=0x01,
 STOP_SCAN=0x02,
 RESET=0x03,
 TAKE_SELF_ECHO=0x04,
 GET_VERSION=0x10,
 GET_NUM_TRANDUCERS=0x11,
 GET_TRANSDUCER_MASK=0x12,
 SET_TRANSDUCER_MASK=0x22,
 GET_SONAR_DELAY=0x13,
 SET_SONAR_DELAY=0x23,
 GET_ECHO_SAMPLE_SIZE=0x14,
 SET_ECHO_SAMPLE_SIZE=0x24,
 GET_GAIN=0x17,
 SET_GAIN=0x27,
 NUM_THRESHOLD_RANGES=0x18,
 GET_THRESHOLDS=0x19,
 SET_THRESHOLDS=0x29,
 GET_NOISE_DELTA=0x1A,
 SET_NOISE_DELTA=0x2A
	};
	
	bool mySendTracking;
	bool myRecvTracking;
  bool mySendTrackingSet;
  bool myRecvTrackingSet;

  ArRobotPacketReceiver *myReceiver;
  ArRobotPacketSender *mySender;

  ArMutex myPacketsMutex;
  ArMutex myDataMutex;
	ArMutex myDeviceMutex;
	
  ArLog::LogLevel myInfoLogLevel;

  std::list<ArRobotPacket *> myPackets;

	int myFirmwareVersion;

  ArTime myPrevSensorIntTime;

  ArFunctorC<ArSonarMTX> mySensorInterpTask;
  ArRetFunctorC<bool, ArSonarMTX> myAriaExitCB;

};



#endif // ARSONARDEVICE_H
