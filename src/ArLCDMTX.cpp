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
#include "ArLCDMTX.h"
#include "ArSensorReading.h"
#include "ariaOSDef.h"
#include "ArSerialConnection.h"
#include "ariaInternal.h"
#include "ArSystemStatus.h"

#include <time.h>
//#include "../ArNetworking/include/ArServerMode.h"

#include <sys/types.h>
#ifndef WIN32
#include <dirent.h>
#endif
#include <ctype.h>


#define ARLCDMTXDEBUG
#if (defined(ARLCDMTXDEBUG))
#define IFDEBUG(code) {code;}
#else
#define IFDEBUG(code)
#endif


std::string ArLCDMTX::ourFirmwareBaseDir("/usr/local/apps/marcDownload/");

AREXPORT ArLCDMTX::ArLCDMTX(int lcdBoardNum, const char *name,
	ArDeviceConnection *conn,
	ArRobot *robot) :
	mySensorInterpTask(this, &ArLCDMTX::sensorInterp),
	myConn(conn),
	myName(name),
	myBoardNum(lcdBoardNum),
	myConnFailOption(false),
	myFirmwareVersion(""),
	myAriaExitCB(this, &ArLCDMTX::disconnect)
{

	myInfoLogLevel = ArLog::Normal;
	clear();
	setRobot(robot);
	myRobotIdentifierChanged = false;
	myRobotIdentifier[0] = '\0';

	mySendTracking = false;
	myRecvTracking = false;

	myLoggedBatteryError = false;

	// now from the robot params - read each unit
	// searching for the units configured for this
	// board, then create the unit map
	if (robot && robot->getRobotParams()) {
		ArLog::log(ArLog::Normal, "%s::ArLCDMTX LCD board %d params",
			getName(), myBoardNum);
	}
	Aria::addExitCallback(&myAriaExitCB, -10);
	//myLogLevel = ArLog::Verbose;
	//myLogLevel = ArLog::Terse;
	myLogLevel = ArLog::Normal;
	myRobotRunningAndConnected = false;

}


AREXPORT ArLCDMTX::~ArLCDMTX()
{
	if (myRobot != NULL) {
		myRobot->remSensorInterpTask(&myProcessCB);
	}
}


AREXPORT void ArLCDMTX::setDeviceConnection(
	ArDeviceConnection *conn)
{
	myConn = conn;
	myConn->setDeviceName(getName());
}

AREXPORT ArDeviceConnection *ArLCDMTX::getDeviceConnection(void)
{
	return myConn;
}


AREXPORT void ArLCDMTX::setRobot(ArRobot *robot)
{
	myRobot = robot;

	if ((robot != NULL) && (robot->getRobotParams())) {

		myConnFailOption = robot->getRobotParams()->getLCDMTXBoardConnFailOption(myBoardNum);
	}

	// this is the code from the laser, i changed the priority to 20 from 90
	// also it puts in mysensorinterptask instead of myprocesscb
	if (myRobot != NULL) {
		myRobot->remSensorInterpTask(&mySensorInterpTask);
		myRobot->addSensorInterpTask(myName.c_str(), 20, &mySensorInterpTask);
	}
}

void ArLCDMTX::clear(void)
{
	myIsConnected = false;
	myTryingToConnect = false;
	myStartConnect = false;
}

AREXPORT void ArLCDMTX::lcdSetName(const char *name)
{
	myName = name;
	myDeviceMutex.setLogNameVar("%s::myDeviceMutex", getName());
	myPacketsMutex.setLogNameVar("%s::myPacketsMutex", getName());
	myDataMutex.setLogNameVar("%s::myDataMutex", getName());
	myAriaExitCB.setNameVar("%s::exitCallback", getName());
	myDisconnectOnErrorCBList.setNameVar(
		"%s::myDisconnectOnErrorCBList", myName.c_str());

}

AREXPORT bool ArLCDMTX::disconnect(void)
{
	if (!isConnected())
		return true;
	ArLog::log(ArLog::Normal, "%s: Disconnecting", getName());
	return true;
}

AREXPORT int ArLCDMTX::getReadingCount()
{
	if (myTimeLastReading == time(NULL))
		return myReadingCount;
	if (myTimeLastReading == time(NULL) - 1)
		return myReadingCurrentCount;
	return 0;
}

AREXPORT void ArLCDMTX::internalGotReading(void)
{
	if (myTimeLastReading != time(NULL))
	{
		myTimeLastReading = time(NULL);
		myReadingCount = myReadingCurrentCount;
		myReadingCurrentCount = 0;
	}
	myReadingCurrentCount++;

	myLastReading.setToNow();

}

void ArLCDMTX::failedToConnect(void)
{
	ArLog::log(ArLog::Normal,
		"%s:failedToConnect Cound not connect to lcd",
		getName());
	myDeviceMutex.lock();
	myTryingToConnect = true;
	myDeviceMutex.unlock();
}

void ArLCDMTX::sensorInterp(void)
{
	//ArLCDMTXPacket *packet;
	ArRobotPacket *packet;

	while (1) {
		myPacketsMutex.lock();
		if (myPackets.empty()) {
			myPacketsMutex.unlock();
			return;
		}

		packet = myPackets.front();
		myPackets.pop_front();
		myPacketsMutex.unlock();
		unsigned char *buf = (unsigned char *)packet->getBuf();

		if (packet->getID() != KEEP_ALIVE) {
			ArLog::log(ArLog::Normal,
				"%s:sensorInterp Could not process packet, command invalid 0x%02x of %d length",
				getName(), packet->getID(), packet->getLength());

			delete packet;
			continue;
		}

		myLastReading.setToNow();
		internalGotReading();

		ArTime time = packet->getTimeReceived();

#if 0 // for raw trace

		char obuf[256];
		obuf[0] = '\0';
		int j = 0;
		for (int i = 0; i < packet->getLength() - 2; i++) {
			sprintf(&obuf[j], "_%02x", buf[i]);
			j = j + 3;
		}
		ArLog::log(ArLog::Normal,
			"%s::sensorInterp() packet = %s", getName(), obuf);
#endif

		myDeviceMutex.lock();

		myDeviceMutex.unlock();
		delete packet;

	} // end while
}


AREXPORT bool ArLCDMTX::blockingConnect(bool sendTracking, bool recvTracking,
	int lcdNumber, ArFunctor1<int> *onCallback,
	ArFunctor1<int> *offCallback)
{

	mySendTracking = sendTracking;
	myRecvTracking = recvTracking;


	myDeviceMutex.lock();
	if (myConn == NULL) {
		ArLog::log(ArLog::Terse,
			"%s: Could not connect because there is no connection defined",
			getName());
		myDeviceMutex.unlock();
		failedToConnect();
		return false;
	}
	ArSerialConnection *serConn = NULL;
	serConn = dynamic_cast<ArSerialConnection *> (myConn);
	if (serConn != NULL)
		serConn->setBaud(115200);
	if (myConn->getStatus() != ArDeviceConnection::STATUS_OPEN
		&& !myConn->openSimple()) {
		ArLog::log(
			ArLog::Terse,
			"%s: Could not connect because the connection was not open and could not open it",
			getName());
		myDeviceMutex.unlock();
		failedToConnect();
		return false;
	}

	//myRecvTracking = true;
	//mySendTracking = true;
	myReceiver = new ArRobotPacketReceiver(myConn, true, HEADER1, HEADER2,
		myRecvTracking,
		"ArLCDMTX");

	mySender = new ArRobotPacketSender(myConn, HEADER1, HEADER2,
		mySendTracking,
		"ArLCDMTX");

	myTryingToConnect = true;
	myDeviceMutex.unlock();
	ArTime timeDone;
	if (!timeDone.addMSec(30 * 1000)) {
		ArLog::log(ArLog::Normal,
			"%s::blockingConnect() error adding msecs (30 * 1000)",
			getName());
	}

	bool stopSent = false;
	bool versionVerified = false;
	bool sysInfoVerified = false;
	ArRobotPacket *packet;

	do {

		// first have the lcd stop sending
		// ???? not sure if there's a response to this
		if ((!stopSent) && (!sendSystemInfo(SYS_INFO_STOP))) {
			ArLog::log(ArLog::Normal,
				"%s::blockingConnect() Could not send system info request to LCD", getName());
			failedToConnect();
			return false;
		}

		stopSent = true;

		ArUtil::sleep(300);

		if (!versionVerified) {
			if (!sendVersion()) {
				ArLog::log(ArLog::Normal,
					"%s::blockingConnect() Could not send version request to LCD", getName());
				failedToConnect();
				return false;
			}

			packet = myReceiver->receivePacket(1000);

			if (packet == NULL) {
				ArLog::log(ArLog::Normal, "%s::blockingConnect() Did not get response to version request (%d) - resending",
					getName(), timeDone.mSecTo());
				continue;
			}

			unsigned char *versionBuf = (unsigned char *)packet->getBuf();

			// verify get num trans received
			if (versionBuf[3] != VERSION)  {

				ArLog::log(ArLog::Normal,
					"%s::blockingConnect() Invalid response from lcd to send version (0x%x)",
					getName(), versionBuf[3]);
				delete packet;
				continue;
			}
			else {

				char tempBuf[256];

				snprintf(tempBuf, sizeof(tempBuf), "%s", &versionBuf[4]);
				myFirmwareVersion = tempBuf;

				ArLog::log(ArLog::Normal, "%s::blockingConnect() LCD firmware version = %s",
					getName(), myFirmwareVersion.c_str());
				delete packet;

			}

			// if the version dosn't verify - we'll need to download new firmware

			if (verifyFwVersion()) {

				// power cycle the HMI

				if (offCallback != NULL)
					offCallback->invoke(lcdNumber);

				ArUtil::sleep(3000);

				if (onCallback != NULL)
					onCallback->invoke(lcdNumber);

				downloadFirmware();

			}
		}
		versionVerified = true;

		// send a system info

		if (!sysInfoVerified) {
			if (!sendSystemInfo(SYS_INFO_ONCE)) {
				ArLog::log(ArLog::Normal,
					"%s::blockingConnect() Could not send sys info request to LCD", getName());
				failedToConnect();
				return false;
			}

			packet = myReceiver->receivePacket(1000);

			if (packet == NULL) {
				ArLog::log(ArLog::Normal, "%s::blockingConnect() Did not get response to sys info request (%d) - resending",
					getName(), timeDone.mSecTo());
				continue;
			}

			unsigned char *sysInfoBuf = (unsigned char *)packet->getBuf();

			// verify get num trans received
			if (sysInfoBuf[3] != SYSTEM_INFO)  {

				ArLog::log(ArLog::Normal,
					"%s::blockingConnect() Invalid response from lcd to send system info (0x%x)",
					getName(), sysInfoBuf[3]);
				delete packet;
				continue;
			}
			else {

				myStatusFlags = sysInfoBuf[4];
				myCurrentScreen = sysInfoBuf[5];

				ArLog::log(ArLog::Normal, "%s::blockingConnect() LCD status flag = %d current screen = %d",
					getName(), myStatusFlags, myCurrentScreen);
				delete packet;

			}
		}

		sysInfoVerified = true;

		if (!sendKeepAlive()) {
			ArLog::log(ArLog::Normal,
				"%s::blockingConnect() Could not send keep alive request to LCD", getName());
			failedToConnect();
			return false;
		}

		packet = myReceiver->receivePacket(1000);

		if (packet == NULL) {
			ArLog::log(ArLog::Normal, "%s::blockingConnect() Did not get response to keep alive request (%d) - resending",
				getName(), timeDone.mSecTo());
			continue;
		}

		unsigned char keepAliveCmd = packet->bufToByte();

		// verify 
		if (keepAliveCmd != KEEP_ALIVE) {
			ArLog::log(ArLog::Normal,
				"%s::blockingConnect() Invalid response from lcd to send keep alive (0x%x)",
				getName(), keepAliveCmd);
			delete packet;
			continue;
		}
		else {
			delete packet;
			myIsConnected = true;
			myTryingToConnect = false;
			ArLog::log(ArLog::Normal, "%s::blockingConnect() Connection successful",
				getName());

			myLastReading.setToNow();

			runAsync();
			return true;
		}
	} while (timeDone.mSecTo() >= 0);

	ArLog::log(ArLog::Normal,
		"%s::blockingConnect() Could not send keep alive Request to LCD", getName());
	failedToConnect();
	return false;

} // end blockingConnect

AREXPORT const char * ArLCDMTX::getName(void) const
{
	return myName.c_str();
}

AREXPORT void * ArLCDMTX::runThread(void *arg)
{


	ArRobotPacket *packet;

	// initialize the variouse settable values
	myScreenNumberChanged = false;
	myBootTextChanged = false;
	myChangedBootText[0] = '\0';
	myMainScreenStatusChanged = false;
	myChangedStatusText[0] = '\0';
	myMainScreenModeChanged = false;
	myChangedModeText[0] = '\0';
	myBackLightChanged = false;
	myIpAddress = "0.0.0.0";

	getIpAddress();

	setRobotIPStatus(myIpAddress.c_str());
	ArUtil::sleep(300);

	/* test code
	std::string buf = "going to goal x";
	std::string buf1 = "goal seeking";
	std::string buf2 = "Aram is startup up...";

	setMTXLCDMainScreenStatus(buf.c_str());

	setMTXLCDMainScreenMode(buf1.c_str());

	setMTXLCDBootScreenText(buf2.c_str());

	setMTXLCDBackLight(true);

	setMTXLCDScreenNumber(1);
	*/

	while (getRunning())
	{

		if (myRobotIdentifierChanged) {
			setRobotIdStatus(myRobotIdentifier.c_str());
			myRobotIdentifierChanged = false;
			ArUtil::sleep(300);
		}

		if (myScreenNumberChanged) {
			setScreenNumber(myChangedScreenNumber);
			myScreenNumberChanged = false;
			ArUtil::sleep(300);
		}

		if (myBackLightChanged) {
			setBacklight(myChangedBackLight);
			myBackLightChanged = false;
			ArUtil::sleep(300);
		}

		if (myBootTextChanged) {
			setBootStatus(myChangedBootText);
			myBootTextChanged = false;
			myChangedBootText[0] = '\0';
			ArUtil::sleep(300);
		}

		if (myMainScreenStatusChanged) {
			setMainStatus(myChangedStatusText);
			myMainScreenStatusChanged = false;
			myChangedStatusText[0] = '\0';
			ArUtil::sleep(300);
		}

		if (myMainScreenModeChanged) {
			setTextStatus(myChangedModeText);
			myMainScreenModeChanged = false;
			myChangedModeText[0] = '\0';
			ArUtil::sleep(300);
		}

		if (hasIpAddressChanged()) {
			setRobotIPStatus(myIpAddress.c_str());
			ArUtil::sleep(300);
		}


		if (!setSystemMeters(getBatteryPercentage(), getWifiPercentage())) {
			IFDEBUG(ArLog::log(ArLog::Normal,
				"%s::runThread() call to setSystemMeters failed", getName()));
		}

		ArUtil::sleep(300);


		if (!sendKeepAlive()) {
			IFDEBUG(ArLog::log(ArLog::Normal,
				"%s::runThread() call to sendKeepAlive failed", getName()));
		}

		while (getRunning() && myIsConnected &&
			(packet = myReceiver->receivePacket(500)) != NULL) {
			myPacketsMutex.lock();
			myPackets.push_back(packet);
			myPacketsMutex.unlock();
			if (myRobot == NULL)
				sensorInterp();
		}

		// if we have a robot but it isn't running yet then don't have a
		// connection failure
		if (getRunning() && myIsConnected && checkLostConnection()) {
			ArLog::log(ArLog::Terse,
				"%s::runThread()  Lost connection to the MTX lcd because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
				myLastReading.mSecSince() / 1000.0,
				getConnectionTimeoutSeconds());
			myIsConnected = false;
			if (myConnFailOption)
				disconnectOnError();
			continue;
		}

	}
	// if we have a robot but it isn't running yet then don't have a
	// connection failure
	/* MPL PS TODO This should lose connection if we
	haven't heard from it in long enough... but this is
	loosing connection anytime we lose one packet
	(which'll always happen sometimes on serial).
	if (getRunning() && myIsConnected) {
	//ArLog::log (ArLog::Normal,
	//            "%s::runThread()  Lost connection to the lcd because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
	//            myLastReading.mSecSince() / 1000.0,
	//            getConnectionTimeoutSeconds() );
	ArLog::log (ArLog::Normal,
	"%s::runThread()  Lost connection to the lcd because of error %d %d", getName(), getRunning(), myIsConnected);
	myIsConnected = false;
	//laserDisconnectOnError();
	continue;
	}
	*/
	//ArUtil::sleep(1);
	//ArUtil::sleep(2000);
	//ArUtil::sleep(300);

	return NULL;
}


#if 0 // debug runthread code
AREXPORT void * ArLCDMTX::runThread(void *arg)
{
	//ArLCDMTXPacket *packet;
	ArRobotPacket *packet;
	unsigned char wifi;
	unsigned char battery;
	bool lightstat = true;

	char text1[10000];
	std::string buf = "going to goal x";
	std::string buf1 = "goal seaking";
	std::string buf2 = "Aram is startup up...";
	std::string buf3 = "Nopey";
	std::string buf4 = "10.0.152.155";


	unsigned char screenNum;
	unsigned char batt;
	unsigned char wi;



	ArUtil::sleep(300);


	if (!setScreenNumber(STATUS_SCREEN)) {
		IFDEBUG(ArLog::log(ArLog::Normal,
			"%s::runThread() call to setScreenNumber failed", getName()));
	}

	ArUtil::sleep(300);

	if (!getScreenNumber(&screenNum))
		ArLog::log(ArLog::Normal,
		"%s::runThread() call to getScreenNum failed", getName());
	else
		ArLog::log(ArLog::Normal,
		"%s::runThread() current screen num = %d", getName(), screenNum);


	int mod;
	int cnt = 0;


	ArUtil::sleep(300);

	if (!setTextStatus(buf1.c_str())) {
		IFDEBUG(ArLog::log(ArLog::Normal,
			"%s::runThread() call to setTextStatus failed", getName()));
	}

	ArUtil::sleep(300);

	if (!setMainStatus(buf.c_str())) {
		IFDEBUG(ArLog::log(ArLog::Normal,
			"%s::runThread() call to setMainStatus failed", getName()));
	}


	ArUtil::sleep(300);

	if (!setBootStatus(buf2.c_str())) {
		IFDEBUG(ArLog::log(ArLog::Normal,
			"%s::runThread() call to setBootStatus failed", getName()));
	}

	ArUtil::sleep(300);

	if (!setRobotIdStatus(buf3.c_str())) {
		IFDEBUG(ArLog::log(ArLog::Normal,
			"%s::runThread() call to setRobotIdStatus failed", getName()));
	}
	ArUtil::sleep(300);

	if (!setRobotIPStatus(buf4.c_str())) {
		IFDEBUG(ArLog::log(ArLog::Normal,
			"%s::runThread() call to setRobotIPStatus failed", getName()));
	}

	ArUtil::sleep(300);

	while (getRunning())
	{

		//#if 0
		if (mod++ % 100 == 0) {

			// #if 0
			text1[0] = '\0';
			if (!getBootStatus(&text1[0])) {
				IFDEBUG(ArLog::log(ArLog::Normal,
					"%s::runThread() call to getBootStatus failed", getName()));
			}
			else {
				ArLog::log(ArLog::Normal,
					"%s::runThread() Boot Status \"%s\" ", getName(), text1);
				text1[0] = '\0';
			}

			ArUtil::sleep(300);

			if (!getMainStatus(&text1[0])) {
				IFDEBUG(ArLog::log(ArLog::Normal,
					"%s::runThread() call to getMainStatus failed", getName()));
			}
			else {
				ArLog::log(ArLog::Normal,
					"%s::runThread() Main Status  \"%s\" ", getName(), text1);
				text1[0] = '\0';
			}

			ArUtil::sleep(300);

			if (!getTextStatus(&text1[0])) {
				IFDEBUG(ArLog::log(ArLog::Normal,
					"%s::runThread() call to getTextStatus failed", getName()));
			}
			else {
				ArLog::log(ArLog::Normal,
					"%s::runThread() Text Status  \"%s\" ", getName(), text1);
				text1[0] = '\0';
			}

			ArUtil::sleep(300);


			//#endif
			/*
			if (cnt == 0) {
			if (!setTextStatus(buf.c_str())) {
			IFDEBUG(ArLog::log (ArLog::Normal,
			"%s::runThread() call to setTextStatus failed", getName()));
			}

			if (!getTextStatus(&text1[0])) {
			IFDEBUG(ArLog::log (ArLog::Normal,
			"%s::runThread() call to getTextStatus failed", getName()));
			}
			else {
			ArLog::log (ArLog::Normal,
			"%s::runThread() Text Status  \"%s\" ", getName(), text1);
			text1[0] = '\0';
			}

			cnt = 1;
			}
			else if (cnt == 1) {

			if (!setMainStatus(buf1.c_str())) {
			IFDEBUG(ArLog::log (ArLog::Normal,
			"%s::runThread() call to setMainStatus failed", getName()));
			}

			if (!getMainStatus(&text1[0])) {
			IFDEBUG(ArLog::log (ArLog::Normal,
			"%s::runThread() call to getMainStatus failed", getName()));
			}
			else {
			ArLog::log (ArLog::Normal,
			"%s::runThread() Main Status  \"%s\" ", getName(), text1);
			text1[0] = '\0';
			}

			cnt = 2;
			}
			else {

			if (!setBootStatus(buf2.c_str())) {
			IFDEBUG(ArLog::log (ArLog::Normal,
			"%s::runThread() call to setBootStatus failed", getName()));
			}

			if (!getBootStatus(&text1[0])) {
			IFDEBUG(ArLog::log (ArLog::Normal,
			"%s::runThread() call to getBootStatus failed", getName()));
			}
			else {
			ArLog::log (ArLog::Normal,
			"%s::runThread() Boot Status \"%s\" ", getName(), text1);

			text1[0] = '\0';
			}

			cnt = 0;
			}
			*/

			if (!getScreenNumber(&screenNum))
				ArLog::log(ArLog::Normal,
				"%s::runThread() call to getScreenNum failed", getName());
			else
				ArLog::log(ArLog::Normal,
				"%s::runThread() current screen num = %d", getName(), screenNum);




			/*
			ArUtil::sleep(300);

			if (!setBacklight(lightstat)) {
			IFDEBUG(ArLog::log (ArLog::Normal,
			"%s::runThread() call to setBacklight failed", getName()));
			}

			// just using lightstat here as it's 0 or 1 for the 2 screens
			if (!setScreenNumber(lightstat)) {
			IFDEBUG(ArLog::log (ArLog::Normal,
			"%s::runThread() call to setScreenNumber failed", getName()));
			}


			if (lightstat)
			lightstat = false;
			else
			lightstat = true;


			ArUtil::sleep(1000);

			unsigned char backlight;
			if (!getBacklight(&backlight))
			ArLog::log (ArLog::Normal,
			"%s::runThread() call to getBacklight failed", getName());
			else
			ArLog::log (ArLog::Normal,
			"%s::runThread() current backlight = %d", getName(), backlight);

			ArUtil::sleep(300);
			*/
			if (!getSystemMeters(&batt, &wi))
				ArLog::log(ArLog::Normal,
				"%s::runThread() call to getSystemMeters failed", getName());
			else
				ArLog::log(ArLog::Normal,
				"%s::runThread() current battery = %d wifi = %d", getName(), batt, wi);

			wifi = getWifiPercentage();
			battery = getBatteryPercentage();

			ArUtil::sleep(300);

			if (!setSystemMeters(battery, wifi)) {
				IFDEBUG(ArLog::log(ArLog::Normal,
					"%s::runThread() call to setSystemMeters failed", getName()));
			}
		}
		//#endif




		if (!sendKeepAlive()) {
			IFDEBUG(ArLog::log(ArLog::Normal,
				"%s::runThread() call to sendKeepAlive failed", getName()));
		}

		while (getRunning() && myIsConnected &&
			(packet = myReceiver->receivePacket(500)) != NULL) {
			myPacketsMutex.lock();
			myPackets.push_back(packet);
			myPacketsMutex.unlock();
			if (myRobot == NULL)
				sensorInterp();
		}

		// if we have a robot but it isn't running yet then don't have a
		// connection failure
		if (getRunning() && myIsConnected && checkLostConnection()) {
			ArLog::log(ArLog::Terse,
				"%s::runThread()  Lost connection to the MTX lcd because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
				myLastReading.mSecSince() / 1000.0,
				getConnectionTimeoutSeconds());
			myIsConnected = false;
			if (myConnFailOption)
				disconnectOnError();
			continue;
		}

	}
	// if we have a robot but it isn't running yet then don't have a
	// connection failure
	/* MPL PS TODO This should lose connection if we
	haven't heard from it in long enough... but this is
	loosing connection anytime we lose one packet
	(which'll always happen sometimes on serial).
	if (getRunning() && myIsConnected) {
	//ArLog::log (ArLog::Normal,
	//            "%s::runThread()  Lost connection to the lcd because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
	//            myLastReading.mSecSince() / 1000.0,
	//            getConnectionTimeoutSeconds() );
	ArLog::log (ArLog::Normal,
	"%s::runThread()  Lost connection to the lcd because of error %d %d", getName(), getRunning(), myIsConnected);
	myIsConnected = false;
	//laserDisconnectOnError();
	continue;
	}
	*/
	//ArUtil::sleep(1);
	//ArUtil::sleep(2000);
	//ArUtil::sleep(300);

	return NULL;
}
#endif

/**
This will check if the lcd has lost connection.  If there is no
robot it is a straightforward check of last reading time against
getConnectionTimeoutSeconds.  If there is a robot then it will not
start the check until the lcd is running and connected.
**/
AREXPORT bool ArLCDMTX::checkLostConnection(void)
{

	if ((myRobot == NULL || myRobotRunningAndConnected) &&
		getConnectionTimeoutSeconds() > 0 &&
		myLastReading.mSecSince() >  getConnectionTimeoutSeconds() * 1000)
		return true;

	if (!myRobotRunningAndConnected && myRobot != NULL &&
		myRobot->isRunning() && myRobot->isConnected())
	{
		myRobotRunningAndConnected = true;
		myLastReading.setToNow();
	}

	return false;
}

AREXPORT void ArLCDMTX::disconnectOnError(void)
{
	ArLog::log(ArLog::Normal, "%s: Disconnected because of error", getName());
	myDisconnectOnErrorCBList.invoke();
}

AREXPORT bool ArLCDMTX::sendKeepAlive()
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(KEEP_ALIVE);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log(ArLog::Terse,
			"%s::sendKeepAlive() Could not send keep alive request to LCD", getName());
		return false;
	}

	//IFDEBUG (
	//
	//	ArLog::log (ArLog::Normal,
	//             "%s::sendKeepAlive() keep alive sent to LCD",
	//              getName());

	//); // end IFDEBUG

	return true;

}

AREXPORT bool ArLCDMTX::sendVersion()
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(VERSION);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log(ArLog::Terse,
			"%s::sendVersion() Could not send version request to LCD", getName());
		return false;
	}

	IFDEBUG(

		ArLog::log(ArLog::Normal,
		"%s::sendVersion() version sent to LCD",
		getName());

	); // end IFDEBUG

	return true;

}

AREXPORT bool ArLCDMTX::sendSystemInfo(unsigned char command)
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(SYSTEM_INFO);
	sendPacket.byteToBuf(command);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log(ArLog::Terse,
			"%s::sendSystemInfo() Could not send sys info request to LCD", getName());
		return false;
	}

	IFDEBUG(

		ArLog::log(ArLog::Normal,
		"%s::sendSystemInfo() sys info sent to LCD",
		getName());

	); // end IFDEBUG

	return true;

}

AREXPORT bool ArLCDMTX::sendReboot()
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(REBOOT);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log(ArLog::Terse,
			"%s::sendReboot() Could not send reboot request to LCD", getName());
		return false;
	}

	IFDEBUG(

		ArLog::log(ArLog::Normal,
		"%s::sendReboot() reboot sent to LCD",
		getName());

	); // end IFDEBUG

	return true;

}


AREXPORT bool ArLCDMTX::getTextField(unsigned char textNumber, char *text)
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(GET_TEXT_FIELD);
	sendPacket.byteToBuf(textNumber);

	for (int i = 0; i < 5; i++) {

		if (!mySender->sendPacket(&sendPacket)) {
			ArLog::log(ArLog::Terse,
				"%s::getTextField() Could not send get text field request to LCD", getName());
			return false;
		}

		IFDEBUG(

			ArLog::log(ArLog::Normal,
			"%s::getTextField() get text field sent to LCD",
			getName());

		); // end IFDEBUG

		ArRobotPacket *packet = myReceiver->receivePacket(1000);

		if (packet == NULL) {
			ArLog::log(ArLog::Normal,
				"%s::getTextField() No response to get text field - resending (%d)",
				getName(), i);
			delete packet;
			continue;
		}

		unsigned char *textFieldBuf = (unsigned char *)packet->getBuf();

		// verify get num trans received
		if ((textFieldBuf[3] != GET_TEXT_FIELD) || (textFieldBuf[4] != textNumber)) {
			ArLog::log(ArLog::Normal,
				"%s::getTextField() Invalid response from lcd to get text field status (0x%x) text # %d in text # %d",
				getName(), textFieldBuf[3], textFieldBuf[4], textNumber);
			continue;

		}
		else {


			char tempBuf[256];

			snprintf(tempBuf, sizeof(tempBuf), "%s", &textFieldBuf[5]);
			strcpy(text, tempBuf);

			delete packet;
			return true;
		}
	} // endfor

	return false;

}



AREXPORT bool ArLCDMTX::getScreenNumber(unsigned char *currentScreenNumber)

{
	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(GET_CURRENT_SCREEN_NUM);

	for (int i = 0; i < 5; i++) {

		if (!mySender->sendPacket(&sendPacket)) {
			ArLog::log(ArLog::Terse,
				"%s::getScreenNumber() Could not send get screen number to LCD", getName());
			return false;
		}

		IFDEBUG(

			ArLog::log(ArLog::Normal,
			"%s::getScreenNumber() get screen number sent to LCD",
			getName());

		); // end IFDEBUG


		ArRobotPacket *packet = myReceiver->receivePacket(1000);

		if (packet == NULL) {
			ArLog::log(ArLog::Normal,
				"%s::getScreenNumber() No response to get screen number - resending (%d)",
				getName(), i);
			delete packet;
			continue;
		}

		unsigned char *screenStatusBuf = (unsigned char *)packet->getBuf();

		// verify get num 
		if (screenStatusBuf[3] != GET_CURRENT_SCREEN_NUM) {
			ArLog::log(ArLog::Normal,
				"%s::getScreenNumber() Invalid response from lcd to get screen number (0x%x)",
				getName(), screenStatusBuf[3]);
			continue;

		}
		else {
			*currentScreenNumber = screenStatusBuf[4];
			delete packet;
			return true;
		}
	} // endfor

	return false;

}

AREXPORT bool ArLCDMTX::getBacklight(unsigned char *backlight)

{
	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(GET_BACKLIGHT);

	for (int i = 0; i < 5; i++) {

		if (!mySender->sendPacket(&sendPacket)) {
			ArLog::log(ArLog::Terse,
				"%s::getBacklight() Could not send get backlight to LCD", getName());
			return false;
		}

		IFDEBUG(

			ArLog::log(ArLog::Normal,
			"%s::getBacklight() get backlight sent to LCD",
			getName());

		); // end IFDEBUG


		ArRobotPacket *packet = myReceiver->receivePacket(1000);

		if (packet == NULL) {
			ArLog::log(ArLog::Normal,
				"%s::getBacklight() No response to get backlight - resending (%d)",
				getName(), i);
			delete packet;
			continue;
		}

		unsigned char *backlightBuf = (unsigned char *)packet->getBuf();

		// verify 
		if (backlightBuf[3] != GET_BACKLIGHT) {
			ArLog::log(ArLog::Normal,
				"%s::getBacklight() Invalid response from lcd to get backlight (0x%x)",
				getName(), backlightBuf[3]);
			continue;

		}
		else {
			*backlight = backlightBuf[4];
			delete packet;
			return true;
		}
	} // endfor

	return false;

}

AREXPORT bool ArLCDMTX::getSystemMeters(unsigned char *battery, unsigned char *wifi)

{
	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(GET_SYSTEM_METERS);

	for (int i = 0; i < 5; i++) {

		if (!mySender->sendPacket(&sendPacket)) {
			ArLog::log(ArLog::Terse,
				"%s::getSystemMeters() Could not send get system meters to LCD", getName());
			return false;
		}

		IFDEBUG(

			ArLog::log(ArLog::Normal,
			"%s::getSystemMeters() get system meters sent to LCD",
			getName());

		); // end IFDEBUG


		ArRobotPacket *packet = myReceiver->receivePacket(1000);

		if (packet == NULL) {
			ArLog::log(ArLog::Normal,
				"%s::getSystemMeters() No response to get system meters - resending (%d)",
				getName(), i);
			delete packet;
			continue;
		}

		unsigned char *systemMetersBuf = (unsigned char *)packet->getBuf();

		// verify 
		if (systemMetersBuf[3] != GET_SYSTEM_METERS) {
			ArLog::log(ArLog::Normal,
				"%s::getSystemMeters() Invalid response from lcd to get system meters (0x%x)",
				getName(), systemMetersBuf[3]);
			continue;

		}
		else {
			*battery = systemMetersBuf[4];
			*wifi = systemMetersBuf[5];
			delete packet;
			return true;
		}
	} // endfor

	return false;

}

AREXPORT bool ArLCDMTX::setScreenNumber(unsigned char screenNumber)
{
	if ((screenNumber == BOOT_SCREEN) || (screenNumber == STATUS_SCREEN)) {

		ArRobotPacket sendPacket(HEADER1, HEADER2);
		sendPacket.setID(SET_SCREEN_NUM);
		sendPacket.byteToBuf(screenNumber);

		if (!mySender->sendPacket(&sendPacket)) {
			ArLog::log(ArLog::Terse,
				"%s::setScreenNumber() Could not send set screen number to LCD", getName());
			return false;
		}

		IFDEBUG(

			ArLog::log(ArLog::Normal,
			"%s::setScreenNumber() set screen number %d sent to LCD",
			getName(), screenNumber);

		); // end IFDEBUG

		return true;
	}
	else { // bad screen number
		ArLog::log(ArLog::Normal,
			"%s::setScreenNumber() Invalid screen number %d", getName(), screenNumber);
		return false;
	}
}

AREXPORT bool ArLCDMTX::setTextField(unsigned char textNumber, const char *text)
{


	if ((textNumber == BOOT_STATUS_TEXT) ||
		(textNumber == MAIN_STATUS_TEXT) ||
		(textNumber == MODE_TEXT) ||
		(textNumber == ROBOT_ID_TEXT) ||
		(textNumber == ROBOT_IP_TEXT)) {

		ArRobotPacket sendPacket(HEADER1, HEADER2);
		sendPacket.setID(SET_TEXT_FIELD);
		sendPacket.byteToBuf(textNumber);
		sendPacket.strToBuf(text);

		if (!mySender->sendPacket(&sendPacket)) {
			ArLog::log(ArLog::Terse,
				"%s::setTextField() Could not send mode %d set text field \"%s\" to LCD",
				getName(), textNumber, text);
			return false;
		}

		IFDEBUG(

			ArLog::log(ArLog::Normal,
			"%s::setTextField() mode %d text \"%s\" field sent to LCD",
			getName(), textNumber, text);

		); // end IFDEBUG

		return true;
	}
	else { // bad text number
		ArLog::log(ArLog::Normal,
			"%s::setTextField() Invalid mode %d", getName(), textNumber);
		return false;
	}
}

AREXPORT bool ArLCDMTX::setBacklight(bool backlight)
{


	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(SET_BACKLIGHT);
	if (backlight)
		sendPacket.byteToBuf(99);
	else
		sendPacket.byteToBuf(0);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log(ArLog::Terse,
			"%s::setBacklight() Could not send set backlight %d to LCD",
			getName(), backlight);
		return false;
	}

	IFDEBUG(

		ArLog::log(ArLog::Normal,
		"%s::setBacklight() set backlight %d sent to LCD",
		getName(), backlight);

	); // end IFDEBUG

	return true;

}


AREXPORT bool ArLCDMTX::setSystemMeters(unsigned char battery, unsigned char wifi)
{


	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID(SET_BATTERY_WIFI);
	sendPacket.byteToBuf(battery);
	sendPacket.byteToBuf(wifi);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log(ArLog::Terse,
			"%s::setSystemMeters() Could not send set system meters %d %d to LCD",
			getName(), battery, wifi);
		return false;
	}

	/*
	IFDEBUG (

	ArLog::log (ArLog::Normal,
	"%s::setSystemMeters() set system meters %d %d sent to LCD",
	getName(), battery, wifi);

	); // end IFDEBUG
	*/

	return true;

}


AREXPORT void ArLCDMTX::writeToLCD()
{


#if 0
	// grab the status
	ArServerMode *netMode;
	if ((netMode = ArServerMode::getActiveMode()) != NULL)
	{
		ArLog::log(ArLog::Normal,
			"%s::writeToLCD status = %s", getName(), netMode->getStatus());
	}
	else
	{
		ArLog::log(ArLog::Normal,
			"%s::writeToLCD could not get status");

	}
#endif
}

AREXPORT unsigned char ArLCDMTX::getBatteryPercentage()
{
	if (myRobot->haveStateOfCharge())
		return myRobot->getStateOfCharge();
	else {
		if (!myLoggedBatteryError) {
			ArLog::log(ArLog::Normal,
				"%s::getBatteryPercentage() State of charge not available setting it to 0",
				getName());
			myLoggedBatteryError = true;
		}
		return 0;
	}
}

AREXPORT unsigned char ArLCDMTX::getWifiPercentage()
{
#ifndef WIN32
	// if the link signal exists - then grab the wifi%
	if (ArSystemStatus::getMTXWirelessLink() == 1) {
		return ArSystemStatus::getMTXWirelessQuality();
	}
	else
#endif
		return 0;
}

AREXPORT void ArLCDMTX::getIpAddress()
{
#ifdef WIN32
  myIpAddress = "unknown";
#else
	char ip[1000];
	sprintf(ip, "%d.%d.%d.%d",
		ArSystemStatus::getMTXWirelessIpAddress1(),
		ArSystemStatus::getMTXWirelessIpAddress2(),
		ArSystemStatus::getMTXWirelessIpAddress3(),
		ArSystemStatus::getMTXWirelessIpAddress4());

	myIpAddress = ip;
#endif
}

AREXPORT bool ArLCDMTX::hasIpAddressChanged()
{
#ifndef WIN32
	char ip[1000];
	sprintf(ip, "%d.%d.%d.%d",
		ArSystemStatus::getMTXWirelessIpAddress1(),
		ArSystemStatus::getMTXWirelessIpAddress2(),
		ArSystemStatus::getMTXWirelessIpAddress3(),
		ArSystemStatus::getMTXWirelessIpAddress4());

	std::string tempIpAddress = ip;

	if (tempIpAddress.compare(myIpAddress) != 0) {
		myIpAddress = ip;
		return true;
	}
#endif
	return false;

}

AREXPORT bool ArLCDMTX::setMainStatus(const char *status)
{
#if 0
	if (!setScreenNumber(STATUS_SCREEN)) {
		ArLog::log(ArLog::Normal,
			"%s::setMainStatus() can not set screen number",
			getName());
		return false;
	}
#endif

	if (!setTextField(MAIN_STATUS_TEXT, status)) {
		ArLog::log(ArLog::Normal,
			"%s::setMainStatus() can not set status",
			getName());
		return false;
	}

	IFDEBUG(ArLog::log(ArLog::Normal,
		"%s::setMainStatus() set status successful = %s",
		getName(), status));

	return true;


}

AREXPORT bool ArLCDMTX::setTextStatus(const char *status)
{

#if 0
	if (!setScreenNumber(STATUS_SCREEN)) {
		ArLog::log(ArLog::Normal,
			"%s::setTextStatus() can not set screen number",
			getName());
		return false;
	}
#endif
	if (!setTextField(MODE_TEXT, status)) {
		ArLog::log(ArLog::Normal,
			"%s::setTextStatus() can not set status",
			getName());
		return false;
	}

	IFDEBUG(ArLog::log(ArLog::Normal,
		"%s::setTextStatus() set status successful = %s",
		getName(), status));

	return true;

}


AREXPORT bool ArLCDMTX::setRobotIdStatus(const char *status)
{

#if 0
	if (!setScreenNumber(STATUS_SCREEN)) {
		ArLog::log(ArLog::Normal,
			"%s::setRobotIdStatus() can not set screen number",
			getName());
		return false;
	}
#endif
	if (!setTextField(ROBOT_ID_TEXT, status)) {
		ArLog::log(ArLog::Normal,
			"%s::setRobotIdStatus() can not set status",
			getName());
		return false;
	}

	IFDEBUG(ArLog::log(ArLog::Normal,
		"%s::setRobotIdStatus() set status successful = %s",
		getName(), status));

	return true;

}

AREXPORT bool ArLCDMTX::setRobotIPStatus(const char *status)
{

#if 0
	if (!setScreenNumber(STATUS_SCREEN)) {
		ArLog::log(ArLog::Normal,
			"%s::setRobotIPStatus() can not set screen number",
			getName());
		return false;
	}
#endif
	if (!setTextField(ROBOT_IP_TEXT, status)) {
		ArLog::log(ArLog::Normal,
			"%s::setRobotIdStatus() can not set status",
			getName());
		return false;
	}

	IFDEBUG(ArLog::log(ArLog::Normal,
		"%s::setRobotIPStatus() set status successful = %s",
		getName(), status));

	return true;

}


AREXPORT bool ArLCDMTX::setBootStatus(const char *status)
{

#if 0
	if (!setScreenNumber(BOOT_SCREEN)) {
		ArLog::log(ArLog::Normal,
			"%s::setBootStatus() can not set screen number",
			getName());
		return false;
	}
#endif

	ArLog::log(ArLog::Normal, "%s:setBootStatus: (BOOT_STATUS_TEXT) text '%s'",
		getName(), status);


	if (!setTextField(BOOT_STATUS_TEXT, status)) {
		ArLog::log(ArLog::Normal,
			"%s::setBootStatus() can not set status",
			getName());
		return false;
	}

	IFDEBUG(ArLog::log(ArLog::Normal,
		"%s::setBootStatus() set status successful = %s",
		getName(), status));

	return true;
}


AREXPORT bool ArLCDMTX::getMainStatus(const char *status)
{

#if 0
	if (!setScreenNumber(STATUS_SCREEN)) {
		ArLog::log(ArLog::Normal,
			"%s::getMainStatus() can not set screen number",
			getName());
		return false;
	}
#endif

	if (!getTextField(MAIN_STATUS_TEXT, (char *)status)) {
		ArLog::log(ArLog::Normal,
			"%s::getMainStatus() can not get status",
			getName());
		return false;
	}

	IFDEBUG(ArLog::log(ArLog::Normal,
		"%s::getMainStatus() get status successful = %s",
		getName(), status));

	return true;


}
AREXPORT bool ArLCDMTX::getTextStatus(const char *status)
{

#if 0
	if (!setScreenNumber(STATUS_SCREEN)) {
		ArLog::log(ArLog::Normal,
			"%s::getTextStatus() can not set screen number",
			getName());
		return false;
	}
#endif


	if (!getTextField(MODE_TEXT, (char *)status)) {
		ArLog::log(ArLog::Normal,
			"%s::getTextStatus() can not get status",
			getName());
		return false;
	}

	IFDEBUG(ArLog::log(ArLog::Normal,
		"%s::getTextStatus() get status successful = %s",
		getName(), status));

	return true;

}

AREXPORT bool ArLCDMTX::getBootStatus(const char *status)
{

#if 0
	if (!setScreenNumber(BOOT_SCREEN)) {
		ArLog::log(ArLog::Normal,
			"%s::getBootStatus() can not set screen number",
			getName());
		return false;
	}
#endif

	if (!getTextField(BOOT_STATUS_TEXT, (char *)status)) {
		ArLog::log(ArLog::Normal,
			"%s::getBootStatus() can not get status",
			getName());
		return false;
	}

	IFDEBUG(ArLog::log(ArLog::Normal,
		"%s::getBootStatus() get status successful = %s",
		getName(), status));

	return true;
}


AREXPORT bool ArLCDMTX::verifyFwVersion()
{

	// now make sure we have a file

	std::string hmiFile;
	hmiFile.clear();

	char hmiFilePrefix[100];
	hmiFilePrefix[0] = '\0';


	sprintf(hmiFilePrefix, "AdeptHmi");

	hmiFile = searchForFile(ourFirmwareBaseDir.c_str(), hmiFilePrefix, ".ds");

	char hmiFileOut[100];
	hmiFileOut[0] = '\0';

	if (hmiFile.empty()) {
		ArLog::log(ArLog::Normal,
			"%s::verifyFwVersion() can't find hmi file with prefix = %s",
			getName(), hmiFilePrefix);
		return false;
	}
	else {
		// validate that the file dosn't match the current version

		sprintf(hmiFileOut, "AdeptHmi_%s.ds",
			myFirmwareVersion.c_str());

		if (strcmp(hmiFile.c_str(), hmiFileOut) != 0) {
			myNewFwFile = ourFirmwareBaseDir + hmiFile;
			ArLog::log(ArLog::Normal,
				"%s::verifyFwVersion() versions mismatch %s %s",
				getName(), hmiFileOut, hmiFile.c_str());
			return true;
		}
		else {
			ArLog::log(ArLog::Normal,
				"%s::verifyFwVersion() hmi file found but version matches (%s)",
				getName(), hmiFileOut);
			return false;
		}
	}

	return false;
}


AREXPORT bool ArLCDMTX::downloadFirmware()
{



	// now connect to the serial port

	ArSerialConnection *serConn = NULL;
	serConn = dynamic_cast<ArSerialConnection *> (myConn);
	if (serConn != NULL)
		serConn->setBaud(115200);
	if (myConn->getStatus() != ArDeviceConnection::STATUS_OPEN
		&& !myConn->openSimple()) {
		ArLog::log(
			ArLog::Normal,
			"%s::downloadFirmware() Could not connect because the connection was not open and could not open it",
			getName());

		return false;
	}



	//  now send hello - and see if we get a response

	unsigned char hello = 0xc1;

	//// ArBasePacket *packet;
	// while ((packet = readPacket()) != NULL)

	ArTime timeDone;

	if (!timeDone.addMSec(30 * 1000))
	{
		ArLog::log(ArLog::Normal,
			"%s::downloadFirmware() error adding msecs (30 * 1000)",
			getName());
	}

	unsigned char helloResp[4];
	bool gotResponse = false;
	int hmiVersion = 0;
	int hmiRevision = 0;

	while (timeDone.mSecTo() > 0) {

		if ((myConn->write((char *)&hello, 1)) == -1) {

			ArLog::log(ArLog::Normal,
				"%s::downloadFirmware() Could not send hello to LCD", getName());
			return false;
		}

		if ((myConn->read((char *)&helloResp[0], 4, 500)) > 0) {

			ArLog::log(ArLog::Normal,
				"%s::downloadFirmware() received hello response 0x%02x 0x%02x 0x%02x 0x%02x",
				getName(), helloResp[0], helloResp[1], helloResp[2], helloResp[3]);

			if ((helloResp[0] == 0xc0) && (helloResp[3] == 0x4b)) {
				ArLog::log(ArLog::Normal,
					"%s::downloadFirmware() received hello response",
					getName());

				gotResponse = true;
				hmiVersion = helloResp[1];
				hmiRevision = helloResp[2];

				break;

			}

		}
		else {
			ArLog::log(ArLog::Normal,
				"%s::downloadFirmware() read failed",
				getName());

		}
	}

	if (!gotResponse) {
		ArLog::log(ArLog::Normal,
			"%s::downloadFirmware() Received no hello response", getName());
		return false;
	}



	FILE *file;
	char line[10000];
	unsigned char c;

	if ((file = ArUtil::fopen(myNewFwFile.c_str(), "r")) == NULL) {
		ArLog::log(ArLog::Normal,
			"%s::downloadFirmware() Could not open file %s for reading errno (%d)",
			getName(), myNewFwFile.c_str(), errno);
		return false;
	}

	line[0] = '\0';

	ArLog::log(ArLog::Normal,
		"%s::downloadFirmware() Updating LCD firmware....",
		getName());

	while (fgets(line, sizeof (line), file) != NULL) {

		ArArgumentBuilder builder;
		builder.add(line);

		int i;

		std::string data;
		data.clear();

		for (i = 0; i < builder.getArgc(); i++) {

			if (!builder.isArgInt(i, true)) {

				ArLog::log(ArLog::Normal,
					"%s::downloadFirmware() Could not convert file", getName());
				return false;

			}

			char ch = builder.getArgInt(i, NULL, true);
			data.push_back(ch);



		} // end for


		//ArLog::log(ArLog::Normal,
		//			"%s::downloadFirmware() data = %s size = %d", getName(), data.c_str(), data.size());

		//ArLog::log(ArLog::Normal,
		//		"%s::downloadFirmware() %d %c 0x%02x", getName(), data, data, data);
		if ((myConn->write((char *)data.c_str(), data.size())) == -1) {
			ArLog::log(ArLog::Normal,
				"%s::downloadFirmware() Could not send data size(%d) to LCD errno (%d)", getName(), data.length(), errno);
			return false;
		}


		// wait a sec for the response
		if ((myConn->read((char *)&c, 1, 1000)) > 0) {

			if (c == 0x4b)
				continue;
			else {
				ArLog::log(ArLog::Normal,
					"%s::downloadFirmware() Invalid response %x02%x from LCD to load data",
					getName(), c);
				return false;
			}

		}
		else {
			ArLog::log(ArLog::Normal,
				"%s::downloadFirmware() Did not get response from LCD to load data", getName());
			return false;

		}

	} // end while
	if (feof(file)) {
		// end of file reached
		ArLog::log(ArLog::Normal,
			"%s::downloadFirmware() LCD firmware updated",
			getName());
		fclose(file);

		ArUtil::sleep(5000);

		// now reconnect to the serial port

		ArSerialConnection *serConn = NULL;
		serConn = dynamic_cast<ArSerialConnection *> (myConn);
		if (serConn != NULL)
			serConn->setBaud(115200);
		if (myConn->getStatus() != ArDeviceConnection::STATUS_OPEN
			&& !myConn->openSimple()) {
			ArLog::log(
				ArLog::Normal,
				"%s::downloadFirmware() Could not connect because the connection was not open and could not open it",
				getName());

			return false;
		}
		else {
			ArLog::log(
				ArLog::Normal,
				"%s::downloadFirmware() Reestablished the serial connection",
				getName());
		}
		// need to go get the latest fw version ?? - probably not - just use the one from the file


		if (!sendVersion()) {
			ArLog::log(ArLog::Normal,
				"%s::downloadFirmware() Could not send version request to LCD", getName());
			return false;
		}

		ArRobotPacket *packet = myReceiver->receivePacket(1000);

		if (packet == NULL) {
			ArLog::log(ArLog::Normal, "%s::downloadFirmware() Did not get response to version request (%d) - resending",
				getName(), timeDone.mSecTo());
			return false;
		}

		unsigned char *versionBuf = (unsigned char *)packet->getBuf();

		// verify get num trans received
		if (versionBuf[3] != VERSION)  {

			ArLog::log(ArLog::Normal,
				"%s::downloadFirmware() Invalid response from lcd to send version (0x%x)",
				getName(), versionBuf[3]);
			delete packet;
			return false;
		}
		else {

			char tempBuf[256];

			snprintf(tempBuf, sizeof(tempBuf), "%s", &versionBuf[4]);
			myFirmwareVersion = tempBuf;

			ArLog::log(ArLog::Normal, "%s::downloadFirmware() LCD firmware version = %s",
				getName(), myFirmwareVersion.c_str());
			delete packet;

		}

		return true;

	}
	else {

		ArLog::log(ArLog::Normal,
			"%s::downloadFirmware() failed updating LCD firmware",
			getName());
		fclose(file);

		return false;

	}
}


AREXPORT std::string ArLCDMTX::searchForFile(
	const char *dirToLookIn, const char *prefix, const char *suffix)
{

	/***
	ArLog::log(ArLog::Normal,
	"ArUtil::matchCase() dirToLookIn = \"%s\" fileName = \"%s\"",
	dirToLookIn,
	fileName);
	***/
#ifndef WIN32

	DIR *dir;
	struct dirent *ent;

	/*
	for (it = split.begin(); it != split.end(); it++)
	{
	printf("@@@@@@@@ %s\n", (*it).c_str());
	}
	*/

	// how this works is we start at the base dir then read through
	// until we find what the next name we need, if entry is a directory
	// and we're not at the end of our string list then we change into
	// that dir and the while loop keeps going, if the entry isn't a
	// directory and matchs and its the last in our string list we've
	// found what we want
	if ((dir = opendir(dirToLookIn)) == NULL)
	{
		ArLog::log(ArLog::Normal,
			"AramUtil::findFile: No such directory '%s' for base",
			dirToLookIn);
		return "";
	}

	while ((ent = readdir(dir)) != NULL)
	{
		// ignore some of these
		if (ent->d_name[0] == '.')
		{
			//printf("Ignoring %s\n", ent->d_name[0]);
			continue;
		}
		//printf("NAME %s finding %s\n", ent->d_name, finding.c_str());

		// continue if the prefix should be searched for and doesn't match
		if (prefix != NULL && prefix[0] != '\0' &&
			strncasecmp(ent->d_name, prefix, strlen(prefix)) != 0)
			continue;

		// continue if the suffix should be searched for and doesn't match
		if (suffix != NULL && suffix[0] != '\0' &&
			strlen(ent->d_name) > strlen(suffix) + 1 &&
			strncasecmp(&ent->d_name[strlen(ent->d_name) - strlen(suffix)],
			suffix, strlen(suffix)) != 0)
			continue;

		std::string ret = ent->d_name;
		closedir(dir);
		return ret;
	}
	//printf("!!!!!!!! %s", finding.c_str());
	closedir(dir);

#endif // ifndef WIN32

	return "";
}

AREXPORT bool ArLCDMTX::setMTXLCDMainScreenText(const char *status)
{

	if (strlen(status) < 248) {
		myMainScreenStatusChanged = true;
		strcpy(myChangedStatusText, status);
		return true;
	}

	ArLog::log(ArLog::Normal,
		"%s::setMTXLCDMainScreenMode invalid string length (%d) needs to be < 248",
		getName(), strlen(status));
	return false;


}

/*
AREXPORT bool ArLCDMTX::setMTXLCDMainScreenMode(const char *status)
{

if (strlen(status) < 248) {
myMainScreenModeChanged = true;
strcpy(myChangedModeText, status);
return true;
}

ArLog::log (ArLog::Normal,
"%s::setMTXLCDMainScreenMode invalid string length (%d) needs to be < 248",
getName(), strlen(status));
return false;

}
*/

AREXPORT bool ArLCDMTX::setMTXLCDBootScreenText(const char *status)

{


	if (strlen(status) < 248) {
		myBootTextChanged = true;
		strcpy(myChangedBootText, status);
		return true;
	}

	ArLog::log(ArLog::Normal,
		"%s::setMTXLCDBootScreenText() invalid string length (%d) needs to be < 248",
		getName(), strlen(status));
	return false;

}

AREXPORT bool ArLCDMTX::setMTXLCDScreenNumber(unsigned char screenNumber)
{

	if ((screenNumber == BOOT_SCREEN) || (screenNumber == STATUS_SCREEN)) {

		myScreenNumberChanged = true;
		myChangedScreenNumber = screenNumber;
		return true;
	}

	ArLog::log(ArLog::Normal,
		"%s::setMTXLCDScreenNumber() invalid screen number %d",
		getName(), screenNumber);
	return false;

}

AREXPORT bool ArLCDMTX::setMTXLCDBackLight(bool backLight)
{

	myBackLightChanged = true;
	myChangedBackLight = backLight;
	return true;

}

AREXPORT void ArLCDMTX::setIdentifier(const char *identifier)
{
	myRobotIdentifierChanged = true;
	myRobotIdentifier = identifier;

}

AREXPORT bool ArLCDMTX::internalMTXLCDOverrideMainScreenText(const char *status)
{
	myMainScreenStatusChanged = false;
	return setMainStatus(status);
}

AREXPORT bool ArLCDMTX::internalMTXLCDOverrideBootScreenText(const char *status)
{
	myBootTextChanged = false;
	return setBootStatus(status);
}

AREXPORT bool ArLCDMTX::internalMTXLCDOverrideBackLight(bool backlight)
{
	myChangedBackLight = false;
	return setBacklight(backlight);
}
