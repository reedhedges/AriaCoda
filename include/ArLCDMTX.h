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
#ifndef ARLCDMTX_H
#define ARLCDMTX_H

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

/** Interface to LCD interface panel on an MTX-type robot.

This interface can be used to display text strings on the MTX LCD panel.
One or more ArLCDMTX objects are automatically created by
ArRobotConnector (or ArLCDConnector if used separately from
ArRobotConnector) if connecting to an MTX robot, and a successful
connection to the LCD panel was also made. A pointer
to the first ArLCDMTX object can be obtained via ArRobot::findLCD(),
passing an index of 1.

@since 2.8.0
@ingroup MTX
*/
class ArLCDMTX : public ArASyncTask
{
public:
	/// Constructor
	AREXPORT ArLCDMTX(
		int lcdBoardNum = 0,
		const char * name = "MTXLCD",
		ArDeviceConnection *conn = NULL,
		ArRobot *robot = NULL);
	/// Destructor
	AREXPORT virtual ~ArLCDMTX();

	// Grabs the new readings from the robot and adds them to the buffers
	// (Primarily for internal use.)
	//AREXPORT void processReadings(void);

	AREXPORT int getBoardNum(void)
	{
		return myBoardNum;
	}

	/// Sets the robot pointer, also attaches its process function to the
	/// robot as a Sensor Interpretation task.
	AREXPORT virtual void setRobot(ArRobot *robot);

	/// Sets the device this instance receives packets from
	AREXPORT void setDeviceConnection(ArDeviceConnection *conn);
	/// Gets the device this instance receives packets from
	AREXPORT ArDeviceConnection *getDeviceConnection(void);

	AREXPORT virtual bool blockingConnect(bool sendTracking, bool recvTracking,
		int lcdNumber, ArFunctor1<int> *onCallback,
		ArFunctor1<int> *offCallback);
	AREXPORT virtual bool disconnect(void);
	AREXPORT virtual bool isConnected(void) { return myIsConnected; }
	AREXPORT virtual bool isTryingToConnect(void)
	{
		if (myStartConnect)
			return true;
		else if (myTryingToConnect)
			return true;
		else
			return false;
	}


	/// Lock this device
	AREXPORT virtual int lockDevice() { return(myDeviceMutex.lock()); }
	/// Try to lock this device
	AREXPORT virtual int tryLockDevice() { return(myDeviceMutex.tryLock()); }
	/// Unlock this device
	AREXPORT virtual int unlockDevice() { return(myDeviceMutex.unlock()); }

	AREXPORT virtual const char *getName(void) const;

	AREXPORT void	setInfoLogLevel(ArLog::LogLevel infoLogLevel)
	{
		myInfoLogLevel = infoLogLevel;
	}

	/// Gets the default port type for the lcd
	const char *getDefaultPortType(void) { return myDefaultPortType.c_str(); }

	/// Gets the default port type for the lcd
	const char *getDefaultTcpPort(void) { return myDefaultTcpPort.c_str(); }

	AREXPORT bool verifyFwVersion();


	/// Sets the numter of seconds without a response until connection assumed lost
	AREXPORT virtual void setConnectionTimeoutSeconds(double seconds)
	{
		ArLog::log(ArLog::Normal,
			"%s::setConnectionTimeoutSeconds: Setting timeout to %g secs",
			getName(), seconds);
		myTimeoutSeconds = seconds;
	}
	/// Gets the number of seconds without a response until connection assumed lost
	AREXPORT virtual double getConnectionTimeoutSeconds(void)
	{
		return myTimeoutSeconds;
	}
	/// check for lost connections
	AREXPORT bool checkLostConnection(void);
	/// disconnect 
	AREXPORT void disconnectOnError(void);
	/// Gets the time data was last receieved
	ArTime getLastReadingTime(void) { return myLastReading; }
	/// Gets the number of lcd readings received in the last second
	AREXPORT int getReadingCount(void);
	// Function called in sensorInterp to indicate that a
	// reading was received

	/// Adds a callback for when disconnection happens because of an error
	void addDisconnectOnErrorCB(ArFunctor *functor,
		int position = 51)
	{
		myDisconnectOnErrorCBList.addCallback(functor, position);
	}

	/// Removes a callback for when disconnection happens because of an error
	void remDisconnectOnErrorCB(ArFunctor *functor)
	{
		myDisconnectOnErrorCBList.remCallback(functor);
	}

	const char *getFirmwareVersion(void) const
	{
		return myFirmwareVersion.c_str();
	}


	/// exposed routines to set the various screen status(s) text
	/// size is limited to 247 characters... supports new lines (it
	/// can show up to 6 lines, with roughly 22-26 chars per line)

	AREXPORT bool setMTXLCDMainScreenText(const char *status);
	// we just have one text on the status now
	//AREXPORT bool setMTXLCDMainScreenMode(const char *status);
	AREXPORT bool setMTXLCDBootScreenText(const char *status);

	// get routines
	enum Screens {
		BOOT_SCREEN = 0x00,
		STATUS_SCREEN = 0x01
	};

	// only 2 valid screen numbers 
	AREXPORT bool setMTXLCDScreenNumber(unsigned char screenNumber);

	// sets backlight on/off
	AREXPORT bool setMTXLCDBackLight(bool backLight);

	AREXPORT void setIdentifier(const char *identifier);

	/// internal call (don't use it) that updates from the calling thread instead of the correct thread
	AREXPORT bool internalMTXLCDOverrideMainScreenText(const char *status);
	/// internal call (don't use it) that updates from the calling thread instead of the correct thread
	AREXPORT bool internalMTXLCDOverrideBootScreenText(const char *status);
	/// internal call (don't use it) that updates from the calling thread instead of the correct thread
	AREXPORT bool internalMTXLCDOverrideBackLight(bool backlight);

	/// Change what directory to look in for updated LCD firmware, other than default. Must be called before connecting to any LCDs.
	static void setFirmwareDir(const char *dir) {
		ourFirmwareBaseDir = dir;
	}
protected:

	/// Logs the information about the sensor
	AREXPORT void log(void);

	AREXPORT void writeToLCD();

	AREXPORT bool setMainStatus(const char *status);
	AREXPORT bool setTextStatus(const char *status);
	AREXPORT bool setRobotIdStatus(const char *status);
	AREXPORT bool setRobotIPStatus(const char *status);
	AREXPORT bool setBootStatus(const char *status);
	AREXPORT bool getMainStatus(const char *status);
	AREXPORT bool getTextStatus(const char *status);
	AREXPORT bool getBootStatus(const char *status);


	AREXPORT bool sendKeepAlive();
	AREXPORT bool sendVersion();
	AREXPORT bool sendSystemInfo(unsigned char command);
	AREXPORT bool sendReboot();

	AREXPORT bool getScreenNumber(unsigned char *currentScreenNumber);

	AREXPORT bool getTextField(unsigned char textNumber, char *text);
	AREXPORT bool getBacklight(unsigned char *backlight);
	AREXPORT bool getSystemMeters(unsigned char *battery, unsigned char *wifi);


	// set routines
	AREXPORT bool setTextField(unsigned char textNumber, const char *text);
	AREXPORT bool setBacklight(bool backlight);
	AREXPORT bool setSystemMeters(unsigned char battery, unsigned char wifi);
	AREXPORT bool setScreenNumber(unsigned char screenNumber);

	AREXPORT unsigned char getBatteryPercentage();
	AREXPORT unsigned char getWifiPercentage();

	AREXPORT void getIpAddress();

	AREXPORT bool hasIpAddressChanged();

	AREXPORT virtual void internalGotReading(void);

	ArDeviceConnection *myConn;
	std::string myName;
	std::string myDefaultPortType;
	std::string myDefaultTcpPort;

	double myTimeoutSeconds;
	bool myRobotRunningAndConnected;
	bool myConnFailOption;

	ArTime myLastReading;

	// packet count
	time_t myTimeLastReading;
	int myReadingCurrentCount;
	int myReadingCount;

	ArCallbackList myDisconnectOnErrorCBList;

	ArRobot *myRobot;
	ArFunctorC<ArLCDMTX> myProcessCB;

	AREXPORT virtual void lcdSetName(const char *name);
	AREXPORT virtual void * runThread(void *arg);

	AREXPORT bool downloadFirmware();
	AREXPORT std::string searchForFile(
		const char *dirToLookIn, const char *prefix, const char *suffix);


	void sensorInterp(void);
	void failedToConnect(void);
	void clear(void);
	bool myIsConnected;
	bool myTryingToConnect;
	bool myStartConnect;

	std::string myRobotIdentifier;
	bool myRobotIdentifierChanged;

	bool myOnCharger;
	ArRobot::ChargeState myChargeState;

	int myBoardNum;
	unsigned char myVersion;

	ArLog::LogLevel myLogLevel;

	enum Headers {
		HEADER1 = 0xfa,
		HEADER2 = 0xcd

		//	old value HEADER2=0x5c
	};

	ArRobotPacketReceiver *myReceiver;
	ArRobotPacketSender *mySender;

	ArMutex myPacketsMutex;
	ArMutex myDataMutex;
	ArMutex myDeviceMutex;

	ArLog::LogLevel myInfoLogLevel;

	std::list<ArRobotPacket *> myPackets;

	ArTime myPrevLCDIntTime;

	bool myRequestedSysInfoLCDPackets;
	bool myRequestedCellInfoLCDPackets;

	bool mySendTracking;
	bool myRecvTracking;

	// Protocol Commands


	enum Commands {
		KEEP_ALIVE = 0x00,
		VERSION = 0x01,
		SYSTEM_INFO = 0x02,
		REBOOT = 0x03,
		GET_CURRENT_SCREEN_NUM = 0x10,
		GET_TEXT_FIELD = 0x11,
		GET_BACKLIGHT = 0x12,
		GET_SYSTEM_METERS = 0x13,
		SET_SCREEN_NUM = 0x20,
		SET_TEXT_FIELD = 0x21,
		SET_BACKLIGHT = 0x22,
		SET_BATTERY_WIFI = 0x23
	};



	enum TextNumbers {
		BOOT_STATUS_TEXT = 0x00,
		MAIN_STATUS_TEXT = 0x01,
		MODE_TEXT = 0x02,
		ROBOT_ID_TEXT = 0x03,
		ROBOT_IP_TEXT = 0x04
	};


	enum SYS_INFO {
		SYS_INFO_STOP = 0x00,
		SYS_INFO_ONCE = 0x01,
		SYS_INFO_CONTINUOUS = 0x02
	};

	// System Info
	unsigned char myId;
	std::string myFirmwareVersion;
	unsigned int mySerialNumber;
	long long myCurrentTime;
	unsigned char myCurrentScreen;
	unsigned char myStatusFlags;

	// end system info

	std::string myNewFwFile;

	bool myScreenNumberChanged;
	unsigned char myChangedScreenNumber;

	bool myBootTextChanged;
	char myChangedBootText[248];

	bool myMainScreenStatusChanged;
	char myChangedStatusText[248];

	bool myMainScreenModeChanged;
	char myChangedModeText[248];

	bool myBackLightChanged;
	bool myChangedBackLight;

	bool myLoggedBatteryError;

	std::string myIpAddress;

	ArFunctorC<ArLCDMTX> mySensorInterpTask;
	ArRetFunctorC<bool, ArLCDMTX> myAriaExitCB;

	static std::string ourFirmwareBaseDir;
};



#endif // ARLCDMTX_H
