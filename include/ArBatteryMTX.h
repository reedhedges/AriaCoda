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
#ifndef ARBATTERYMTX_H
#define ARBATTERYMTX_H

#include "ariaTypedefs.h"
#include "ArRangeDevice.h"
#include "ArFunctor.h"
#include "ArRobot.h"
#include "ArRobotPacket.h"
#include "ArRobotConnector.h"



// Packets are in the format of 
// 2 bytes header (0xfa 0xba)
// 1 byte length
// 1 byte command
// xx bytes command specific / args
// 2 bytes checksum
//


/** 
    @since 2.8.0
*/
class ArBatteryMTX : public ArASyncTask
{
public:
  /// Constructor
  AREXPORT ArBatteryMTX( 
			 int batteryBoardNum = 0,
				const char * name = "MTXBattery", 
				ArDeviceConnection *conn = NULL,
				ArRobot *robot = NULL);
  /// Destructor
  AREXPORT virtual ~ArBatteryMTX();
  // Grabs the new readings from the robot and adds them to the buffers
  // (Primarily for internal use.)
  //AREXPORT void processReadings(void);

	int getBoardNum(void)
		{ return myBoardNum; }

  /// Sets the robot pointer, also attaches its process function to the
  /// robot as a Sensor Interpretation task.
  AREXPORT virtual void setRobot(ArRobot *robot);

  /// Very Internal call that gets the packet sender, shouldn't be used
  ArRobotPacketSender *getPacketSender(void)
    { return mySender; }
  /// Very Internal call that gets the packet sender, shouldn't be used
  ArRobotPacketReceiver *getPacketReceiver(void)
    { return myReceiver; }

  /// Sets the device this instance receives packets from
  AREXPORT void setDeviceConnection(ArDeviceConnection *conn);
  /// Gets the device this instance receives packets from
  AREXPORT ArDeviceConnection *getDeviceConnection(void);

	AREXPORT int getAsyncConnectState(void);

	ArRobotPacket getCellPacket()
	{ return myCellPacket; }

  AREXPORT virtual bool blockingConnect(bool sendTracking, bool recvTracking);
  AREXPORT virtual bool disconnect(void);
  virtual bool isConnected(void) { return myIsConnected; }
  virtual bool isTryingToConnect(void) 
    { 
    if (myStartConnect)
			return true;
		else if (myTryingToConnect)
			return true; 
		else
			return false;
    }  

  /// Lock this device
  virtual int lockDevice() { return(myDeviceMutex.lock());}
  /// Try to lock this device
  virtual int tryLockDevice() {return(myDeviceMutex.tryLock());}
  /// Unlock this device
  virtual int unlockDevice() {return(myDeviceMutex.unlock());}

  AREXPORT void logBatteryInfo(ArLog::LogLevel level = ArLog::Normal);
  AREXPORT void logCellInfo(ArLog::LogLevel level = ArLog::Normal);
  void log(ArLog::LogLevel level = ArLog::Normal)
  {
    logBatteryInfo(level);
    logCellInfo(level);
  }

  AREXPORT bool sendPowerOff();
  AREXPORT bool sendPowerOffCancel();
  AREXPORT bool sendStopCharging();
  AREXPORT bool sendStartCharging();
  AREXPORT bool sendSetPowerOffDelay(unsigned int msDelay);
  AREXPORT bool sendSetRealTimeClock(unsigned int secSinceEpoch);
  AREXPORT bool sendResetCellData();
  AREXPORT bool sendSetReserveValue(unsigned short hundredthOfPercent);
  AREXPORT bool sendSetBalanceValue(unsigned short hundredthOfPercent);
  AREXPORT bool sendEmergencyPowerOff();
  AREXPORT bool sendSystemInfo(unsigned char dataValue);
  AREXPORT bool sendCellInfo(unsigned char dataValue);
  AREXPORT bool sendBasicInfo(unsigned char dataValue);

  AREXPORT void updateSystemInfo(unsigned char *buf);
  AREXPORT void updateCellInfo(unsigned char *buf);
  AREXPORT void updateBasicInfo(unsigned char *buf);

	// need to figure out how to pass back the system and cell info 
	//AREXPORT bool fetchSystemInfo();
	//AREXPORT bool fetchCellInfo();

	// basic info
  /// Charge estimate (in percentage, 0-100)
	double getChargeEstimate(void) const
		{ return myChargeEstimate; }
  /// Current draw (amps, negative is charging)
	double getCurrentDraw(void) const
		{ return myCurrentDraw; }
  /// volts
	double getPackVoltage(void) const
		{ return myPackVoltage; }
	int getStatusFlags(void) const
		{ return myStatusFlags; }
	int getErrorFlags(void) const
		{ return myErrorFlags; }

  bool onCharger(void) const 
    { return (myStatusFlags & STATUS_ON_CHARGER); }
  ArRobot::ChargeState getChargeState(void) const
    { return myChargeState; }
  int getChargeStateAsInt(void) const
    { return myChargeState; }

	// system info 
	int getId(void) const
		{ return myId; }
	int getFirmwareVersion(void) const
		{ return myFirmwareVersion; }
	int getSerialNumber(void) const
		{ return mySerialNumber; }
	//int getCurrentTime(void) const
	//	{ return myCurrentTime; }
	long long getCurrentTime(void) const
		{ return myCurrentTime; }
	long long getLastChargeTime(void) const
		{ return myLastChargeTime; }
	int getChargeRemainingEstimate(void) const
		{ return myChargeRemainingEstimate; }
	int getCapacityEstimate(void) const
		{ return myCapacityEstimate; }
	double getDelay(void) const
		{ return myDelay; }
	int getCycleCount(void) const
		{ return myCycleCount; }
	double getTemperature(void) const
		{ return myTemperature; }
	double getPaddleVolts(void) const
		{ return myPaddleVolts; }
	double getVoltage(void) const
		{ return myVoltage; }
	double getFuseVoltage(void) const
		{ return myFuseVoltage; }
	double getChargeCurrent(void) const
		{ return myChargeCurrent; }
	double getDisChargeCurrent(void) const
		{ return myDisChargeCurrent; }
	double getCellImbalance(void) const
		{ return myCellImbalance; }
	double getImbalanceQuality(void) const
		{ return myImbalanceQuality; }
	double getReserveChargeValue(void) const
		{ return myReserveChargeValue; }

	// cell info
	int getNumCells(void) const
		{ return myNumCells; }


	int getCellFlag(int cellNum) const
		{
		std::map<int, CellInfo *>::const_iterator iter = 
				myCellNumToInfoMap.find(cellNum);
		if (iter == myCellNumToInfoMap.end()) 
			return -1;
		else {
			CellInfo *info = iter->second;
			return(info->myCellFlags);
		} }

	int getCellCapacity(int cellNum) const
		{
		std::map<int, CellInfo *>::const_iterator iter = 
				myCellNumToInfoMap.find(cellNum);
		if (iter == myCellNumToInfoMap.end()) 
			return -1;
		else {
			CellInfo *info = iter->second;
			return(info->myCellCapacity);
		} }

	int getCellCharge(int cellNum) const
		{
		std::map<int, CellInfo *>::const_iterator iter = 
				myCellNumToInfoMap.find(cellNum);
		if (iter == myCellNumToInfoMap.end()) 
			return -1;
		else {
			CellInfo *info = iter->second;
			return(info->myCellCharge);
		} }

	double getCellVoltage(int cellNum) const
		{
		std::map<int, CellInfo *>::const_iterator iter = 
				myCellNumToInfoMap.find(cellNum);
		if (iter == myCellNumToInfoMap.end()) 
			return -1;
		else {
			CellInfo *info = iter->second;
			return(info->myCellVoltage);
		} }

  /// Request a continous stream of packets
  AREXPORT void requestContinuousSysInfoPackets(void);
  /// Stop the stream of packets
  AREXPORT void stopSysInfoPackets(void);
  /// See if we've requested packets
  AREXPORT bool haveRequestedSysInfoPackets(void);

  /// Request a continous stream of packets
  AREXPORT void requestContinuousCellInfoPackets(void);
  /// Stop the stream of packets
  AREXPORT void stopCellInfoPackets(void);
  /// See if we've requested packets
  AREXPORT bool haveRequestedCellInfoPackets(void);

  AREXPORT virtual const char *getName(void) const;

  void	setInfoLogLevel(ArLog::LogLevel infoLogLevel)
  { myInfoLogLevel = infoLogLevel; }
	
  /// Gets the default port type for the battery
  const char *getDefaultPortType(void) { return myDefaultPortType.c_str(); }

  /// Gets the default port type for the battery
  const char *getDefaultTcpPort(void) { return myDefaultTcpPort.c_str(); }

  /// Sets the numter of seconds without a response until connection assumed lost
  virtual void setConnectionTimeoutSeconds(double seconds)
	{ ArLog::log(ArLog::Normal, 
		     "%s::setConnectionTimeoutSeconds: Setting timeout to %g secs", 
		     getName(), seconds);
	  myTimeoutSeconds = seconds; }
  /// Gets the number of seconds without a response until connection assumed lost
  virtual double getConnectionTimeoutSeconds(void)
	{return myTimeoutSeconds; }
	/// check for lost connections
	AREXPORT bool checkLostConnection(void);
	/// disconnect 
	AREXPORT void disconnectOnError(void);
  /// Gets the time data was last receieved
  ArTime getLastReadingTime(void) { return myLastReading; }
  /// Gets the number of battery readings received in the last second
  AREXPORT int getReadingCount(void);
  // Function called in sensorInterp to indicate that a
  // reading was received
  AREXPORT virtual void internalGotReading(void);

  /// Adds a callback for when disconnection happens because of an error
  void addDisconnectOnErrorCB(ArFunctor *functor, 
			     int position = 51) 
    { myDisconnectOnErrorCBList.addCallback(functor, position); }

  /// Removes a callback for when disconnection happens because of an error
  void remDisconnectOnErrorCB(ArFunctor *functor)
    { myDisconnectOnErrorCBList.remCallback(functor); }


  /// Adds a callback for when the battery is powering off
  void addBatteryPoweringOffCB(ArFunctor *functor, 
			     int position = 51) 
    { myBatteryPoweringOffCBList.addCallback(functor, position); }

  /// Removes a callback for when the battery is powering off
  void remBatteryPoweringOffCB(ArFunctor *functor)
    { myBatteryPoweringOffCBList.remCallback(functor); }

  /// Adds a callback for when the battery is powering off
  void addBatteryPoweringOffCancelledCB(ArFunctor *functor, 
			     int position = 51) 
    { myBatteryPoweringOffCancelledCBList.addCallback(functor, position); }

  /// Removes a callback for when the battery is powering off
  void remBatteryPoweringOffCancelledCB(ArFunctor *functor)
    { myBatteryPoweringOffCancelledCBList.remCallback(functor); }

  // myStatusFlags 
  enum StatusFlags {
    STATUS_ON_CHARGER=0x0001,
    STATUS_CHARGING=0x0002,
    STATUS_BALANCING_ENGAGED=0x0004,
    STATUS_CHARGER_ON=0x0008,
    STATUS_BATTERY_POWERING_OFF=0x0010,
    /// MPL adding the rest of these since I need one of 'em
    STATUS_MASTER_SWITCH_ON=0x0020,
    STATUS_CHARGE_SWITCH_ON=0x0040,
    STATUS_COMMANDED_SHUTDOWN=0x0080,
    STATUS_OFF_BUTTON_PRESSED=0x0100,
    STATUS_ON_BUTTON_PRESSED=0x0200,
    STATUS_USER_BUTTON_PRESSED=0x0400
  };
  
  // myErrorFlags (if this is updated also change the code in interpBasicInfo
  enum ErrorFlags {
    ERROR_BATTERY_OVERVOLTAGE=0x0001,
    ERROR_BATTERY_UNDERVOLTAGE=0x0002,
    ERROR_OVERCURRENT=0x0004,
    ERROR_BLOWNFUSE=0x0008,
    ERROR_RTC_ERROR=0x0010,
    ERROR_OVER_TEMPERATURE=0x0020,
    ERROR_MASTER_SWITCH_FAULT=0x0040,
    ERROR_SRAM_ERROR=0x0080,
    ERROR_CHARGER_OUT_OF_VOLTAGE_RANGE=0x0100,
    ERROR_CHARGER_CIRCUIT_FAULT=0x0200
  };

enum Headers {
	HEADER1=0xfa,
	HEADER2=0xba
	};


protected:
  ArDeviceConnection *myConn;
	int myAsyncConnectState;
  std::string myName;
	std::string myDefaultPortType;
	std::string myDefaultTcpPort;

  double myTimeoutSeconds;
  bool myRobotRunningAndConnected;

  ArTime myLastReading;

  // packet count
  time_t myTimeLastReading;
  int myReadingCurrentCount;
  int myReadingCount;

  ArCallbackList myDisconnectOnErrorCBList;
  ArCallbackList myBatteryPoweringOffCBList;
  ArCallbackList myBatteryPoweringOffCancelledCBList;

	ArRobot *myRobot;
  ArFunctorC<ArBatteryMTX> myProcessCB;

  AREXPORT virtual void batterySetName(const char *name);
  AREXPORT virtual void * runThread(void *arg);
		

	AREXPORT bool getSystemInfo();
	AREXPORT bool getCellInfo();
	AREXPORT bool getBasicInfo();

  void interpBasicInfo(void);
  void interpErrors(void);
  void checkAndSetCurrentErrors(ErrorFlags errorFlag, const char *errorString);

	// PS - need this because of debug log - battery won't send continuous cell
  ArRobotPacket myCellPacket;

  void sensorInterp(void);
  void failedToConnect(void);
  void clear(void);
  bool myIsConnected;
  bool myTryingToConnect;
  bool myStartConnect;

  ArRobot::ChargeState myChargeState;

	int myBoardNum;
	unsigned char myVersion;

  ArLog::LogLevel myLogLevel;

  //ArBatteryMTXPacketReceiver myReceiver;


	ArRobotPacketReceiver *myReceiver;
  ArRobotPacketSender *mySender;

  ArMutex myPacketsMutex;
  ArMutex myDataMutex;
	ArMutex myDeviceMutex;
	
  ArLog::LogLevel myInfoLogLevel;
	
  //std::list<ArBatteryMTXPacket *> myPackets;
  std::list<ArRobotPacket *> myPackets;
  
  ArTime myPrevBatteryIntTime;

  bool myRequestedSysInfoBatteryPackets;
  bool myRequestedCellInfoBatteryPackets;

	bool mySendTracking;
	bool myRecvTracking;

// Protocol Commands


enum Commands {
	BASIC_INFO=0x00,
	SYSTEM_INFO=0x01,
	CELL_INFO=0x02,
	POWER_OFF_REQUEST=0x10,
	POWER_OFF_CANCEL=0x11,
	STOP_CHARGING=0x12,
	START_CHARGING=0x13,
	SET_POWER_OFF_DELAY=0x20,
	SET_REAL_TIME_CLOCK=0x21,
	RESET_CELL_DATA=0x22,
	SET_RESERVE_VALUE=0x23,
	SET_BALANCE_VALUE=0x24,
	EMERGENCY_OFF=0xff
	};

// SYSTEM_INFO and CELL_INFO Data
enum Data {
	STOP_SENDING=0x00,
	SEND_ONCE=0x01,
	SEND_CONTINUOUS=0x02
	};

// Length fields -
enum Sizes {
	BASIC_INFO_SIZE=16,
	SYSTEM_INFO_SIZE=60,
	CELL_INFO_SIZE=95 // this is for 8 cells
	};

	// System Info
	unsigned char myId;
	unsigned char myFirmwareVersion;
	unsigned int mySerialNumber;
	long long myCurrentTime;
	//unsigned int myCurrentTime;
	//unsigned int myLastChargeTime;
	long long myLastChargeTime;
	unsigned int myChargeRemainingEstimate;
	unsigned int myCapacityEstimate;
	unsigned int myRawDelay;
	double myDelay;
	unsigned int myCycleCount;
	unsigned short myRawTemperature;
	double myTemperature;
	unsigned short myRawPaddleVolts;
	double myPaddleVolts;
	unsigned short myRawVoltage;
	double myVoltage;
	unsigned short myRawFuseVoltage;
	double myFuseVoltage;
	unsigned short myRawChargeCurrent;
	double myChargeCurrent;
	unsigned short myRawDisChargeCurrent;
	double myDisChargeCurrent;
	unsigned short myRawCellImbalance;
	double myCellImbalance;
	unsigned short myRawImbalanceQuality;
	double myImbalanceQuality;
	unsigned short myRawReserveChargeValue;
	double myReserveChargeValue;
	
	// end system info
	
	// Cell Info

	// myCellFlags defines

	enum CellFlags {
		BALANCER_IS_ON=0x01,
		OVER_VOLTAGE=0x02,
		UNDER_VOLTAGE=0x04
	};


	struct CellInfo {
		unsigned char myCellFlags;
		unsigned short myRawCellVoltage;
		double myCellVoltage;
		unsigned short myCellCharge;
		unsigned short myCellCapacity;
	};
	
	unsigned char myNumCells;	
	std::map <int, CellInfo *> myCellNumToInfoMap;
	
	// end cell info

  // Basic Info
  
	unsigned short myRawChargeEstimate;	
  double myChargeEstimate;
	short myRawCurrentDraw;
  double myCurrentDraw;
	unsigned short myRawPackVoltage;
  double myPackVoltage;
	unsigned short myStatusFlags;
	unsigned short myErrorFlags;

  bool myHaveSetRTC;

  int myLastStatusFlags;

  bool myFirstErrorFlagsCheck;
  unsigned short myLastErrorFlags;
  std::string myErrorString;
  int myErrorCount;
  std::string myLastErrorString;
  int myLastErrorCount;

  

	// end basic info
	
  ArFunctorC<ArBatteryMTX> mySensorInterpTask;
  ArRetFunctorC<bool, ArBatteryMTX> myAriaExitCB;

};



#endif // ARBATTERYMTX_H
