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
#include "ArBatteryMTX.h"
#include "ArSensorReading.h"
//#include "ArRobot.h"
#include "ariaOSDef.h"
#include "ArSerialConnection.h"
#include "ariaInternal.h"
#include <time.h>

//#define ARBATTERYMTXDEBUG

#if (defined(ARBATTERYMTXDEBUG))
#define IFDEBUG(code) {code;}
#else
#define IFDEBUG(code)
#endif


AREXPORT ArBatteryMTX::ArBatteryMTX (int batteryBoardNum, const char *name, 
																			ArDeviceConnection *conn,
																			ArRobot *robot) :
	mySensorInterpTask (this, &ArBatteryMTX::sensorInterp),
	myConn (conn),
	myName (name),
	myBoardNum (batteryBoardNum),
	myAriaExitCB (this, &ArBatteryMTX::disconnect)
{

	myInfoLogLevel = ArLog::Normal;
	clear();
	setRobot (robot);

	mySendTracking = false;
	myRecvTracking = false;
	myAsyncConnectState = -1;

	// now from the robot params - read each unit
	// searching for the units configured for this
	// board, then create the unit map
	if (robot && robot->getRobotParams()) {
		ArLog::log (ArLog::Verbose, "%s::ArBatteryMTX Battery board %d params",
		            getName(), myBoardNum);
	}
	Aria::addExitCallback (&myAriaExitCB, -10);
	//myLogLevel = ArLog::Verbose;
	//myLogLevel = ArLog::Terse;
	myLogLevel = ArLog::Normal;
  myRobotRunningAndConnected = false;

  myStatusFlags = 0;
  myLastStatusFlags = 0;
  myErrorFlags = 0;
  myLastErrorFlags = 0;
  myFirstErrorFlagsCheck = true;
  myHaveSetRTC = false;
}


AREXPORT ArBatteryMTX::~ArBatteryMTX()
{
	if (myRobot != NULL) {
		myRobot->remSensorInterpTask (&myProcessCB);
	}
  Aria::remExitCallback(&myAriaExitCB);
}

AREXPORT int ArBatteryMTX::getAsyncConnectState()
{
	return myAsyncConnectState;
}


AREXPORT void ArBatteryMTX::setDeviceConnection (
  ArDeviceConnection *conn)
{
	myConn = conn;
  myConn->setDeviceName(getName());
}

AREXPORT ArDeviceConnection *ArBatteryMTX::getDeviceConnection (void)
{
	return myConn;
}

AREXPORT void ArBatteryMTX::requestContinuousSysInfoPackets (void)
{
	if (myIsConnected) {
		ArLog::log (ArLog::Verbose,
		            "%s:requestContinuousSysInfoPackets - Sending....",
		            getName());
		sendSystemInfo (SEND_CONTINUOUS);
		myRequestedSysInfoBatteryPackets = true;
	}
}


AREXPORT void ArBatteryMTX::stopSysInfoPackets (void)
{
	if (myIsConnected) {
		ArLog::log (ArLog::Verbose,
		            "%s:stopSysInfoPackets - Stopping....",
		            getName());
		sendSystemInfo (STOP_SENDING);
		myRequestedSysInfoBatteryPackets = false;
	}
}

AREXPORT bool ArBatteryMTX::haveRequestedSysInfoPackets (void)
{
	return myRequestedSysInfoBatteryPackets;
}

AREXPORT void ArBatteryMTX::requestContinuousCellInfoPackets (void)
{
	if (myIsConnected) {
		ArLog::log (ArLog::Verbose,
		            "%s:requestContinuousCellInfoPackets - Sending....",
		            getName());
		sendCellInfo (SEND_CONTINUOUS);
		myRequestedCellInfoBatteryPackets = true;
	}
}

AREXPORT void ArBatteryMTX::stopCellInfoPackets (void)
{
	if (myIsConnected) {
		ArLog::log (ArLog::Verbose,
		            "%s:stopCellInfoPackets - Stopping....",
		            getName());
		sendCellInfo (STOP_SENDING);
		myRequestedCellInfoBatteryPackets = false;
	}
}

AREXPORT bool ArBatteryMTX::haveRequestedCellInfoPackets (void)
{
	return myRequestedCellInfoBatteryPackets;
}

AREXPORT void ArBatteryMTX::setRobot (ArRobot *robot)
{
	myRobot = robot;
	// this is the code from the laser, i changed the priority to 20 from 90
	// also it puts in mysensorinterptask instead of myprocesscb
	if (myRobot != NULL) {
		myRobot->remSensorInterpTask (&mySensorInterpTask);
		myRobot->addSensorInterpTask (myName.c_str(), 20, &mySensorInterpTask);
	}
}

void ArBatteryMTX::clear (void)
{
	myIsConnected = false;
	myTryingToConnect = false;
	myStartConnect = false;
}

AREXPORT void ArBatteryMTX::batterySetName (const char *name)
{
	myName = name;
	myDeviceMutex.setLogNameVar ("%s::myDeviceMutex", getName());
	myPacketsMutex.setLogNameVar ("%s::myPacketsMutex", getName());
	myDataMutex.setLogNameVar ("%s::myDataMutex", getName());
	myAriaExitCB.setNameVar ("%s::exitCallback", getName());
  myDisconnectOnErrorCBList.setNameVar(
	  "%s::myDisconnectOnErrorCBList", myName.c_str());

}

AREXPORT bool ArBatteryMTX::disconnect (void)
{
	if (!isConnected())
		return true;
	ArLog::log (ArLog::Normal, "%s: Disconnecting", getName());
  if(myConn)
    myConn->close();
	return true;
}

AREXPORT int ArBatteryMTX::getReadingCount()
{
  if (myTimeLastReading == time(NULL))
    return myReadingCount;
  if (myTimeLastReading == time(NULL) - 1)
    return myReadingCurrentCount;
  return 0;
}

AREXPORT void ArBatteryMTX::internalGotReading(void)
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

void ArBatteryMTX::failedToConnect (void)
{
	ArLog::log (ArLog::Normal,
	            "%s:failedToConnect Cound not connect to battery",
	            getName());
	myDeviceMutex.lock();
	myTryingToConnect = true;
	myDeviceMutex.unlock();
}

void ArBatteryMTX::sensorInterp (void)
{
	//ArBatteryMTXPacket *packet;
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
		unsigned char *buf = (unsigned char *) packet->getBuf();
		// make sure its a basic info packet with 11 bytes (includes checksum and header)
		switch (packet->getID()) {
		case
				SYSTEM_INFO: {
				if (packet->getLength() != SYSTEM_INFO_SIZE) {
					ArLog::log (ArLog::Normal,
					            "%s:sensorInterp Could not process packet, command (%d) or packet length (%d) is invalid",
					            getName(), buf[3], packet->getLength());
					delete packet;
					continue;
				}
				//ArLog::log (ArLog::Normal,
				//							"%s:sensorInterp Received System Info packet",
				//							getName());

				// PS 12/11/12 - count system packets 
				myLastReading.setToNow();
				internalGotReading();

				updateSystemInfo (&buf[3]);
				delete packet;
			}
			break;
		case
				CELL_INFO: {
				if (packet->getLength() != CELL_INFO_SIZE) {
					ArLog::log (ArLog::Normal,
					            "%s:sensorInterp Could not process packet, command (%d) or packet length (%d) is invalid",
					            getName(), buf[3], packet->getLength());
					delete packet;
					continue;
				}

				// PS 12/11/12 - count system packets 
				myLastReading.setToNow();
				internalGotReading();

				//ArLog::log (ArLog::Normal,
				//            "%s:sensorInterp Received Cell Info packet",
				//            getName());
				updateCellInfo (&buf[3]);
				delete packet;
			}
			break;
		case
				BASIC_INFO: {
				if (packet->getLength() != BASIC_INFO_SIZE) {
					ArLog::log (ArLog::Normal,
					            "%s:sensorInterp Could not process packet, command (%d) or packet length (%d) is invalid",
					            getName(), buf[3], packet->getLength());
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
					sprintf (&obuf[j], "_%02x", buf[i]);
					j= j+3;
				}
				ArLog::log (ArLog::Normal,
				            "%s::sensorInterp() packet = %s",getName(), obuf);
#endif
				updateBasicInfo (&buf[3]);
				/*
				ArLog::log(ArLog::Normal,
				"%s:sensorInterp charge estimate is %.2f%%",getName(), myChargeEstimate);
				ArLog::log(ArLog::Normal,
				"%s:sensorInterp current draw is %d",getName(), myCurrentDraw);
				ArLog::log(ArLog::Normal,
				"%s:sensorInterp pack voltage is %d",getName(), myPackVoltage);
				ArLog::log(ArLog::Normal,
				"%s:sensorInterp status flag %x",getName(), myStatusFlags);
				ArLog::log(ArLog::Normal,
				"%s:sensorInterp error flag %x",getName(), myErrorFlags);
				*/
				myDeviceMutex.lock();
				interpBasicInfo();
				myDeviceMutex.unlock();
				delete packet;
			}
			break;
		} // end switch status
	} // end while
}


void ArBatteryMTX::interpBasicInfo(void)
{
  // If we don't think we're on a charger, or if the charger isn't on
  // put us to the not charging state
  if (!(myStatusFlags & STATUS_ON_CHARGER) ||

      (!(myStatusFlags & STATUS_CHARGER_ON) && 
       !(myStatusFlags & STATUS_CHARGING)))
  {
    myChargeState = ArRobot::CHARGING_NOT;
  } 
  else
  {

    /* Taking this out so that we won't think we're docked when we're
    // not really charging...  if we're in the not charging state but
    // we see the charger is on toss it in bulk
    if (myChargeState == ArRobot::CHARGING_NOT && 
	(myStatusFlags & STATUS_CHARGING))
      myChargeState = ArRobot::CHARGING_BULK;
    */
    // if we're in the not charging state or if the charger is on but
    // we start charging toss it in overcharge
    if ((myChargeState == ArRobot::CHARGING_NOT ||
	 myChargeState == ArRobot::CHARGING_BULK)
	 && (myStatusFlags & STATUS_CHARGING))
      myChargeState = ArRobot::CHARGING_OVERCHARGE;

    // if we're in overcharge but we aren't charging anymore then bump
    // us up to float (don't worry about if we're not on the charger,
    // since that's handled above)
    if (myChargeState == ArRobot::CHARGING_OVERCHARGE &&
	! (myStatusFlags & STATUS_CHARGING))
      myChargeState = ArRobot::CHARGING_FLOAT;

  }

  if (myBoardNum == 1) {
    myRobot->setBatteryInfo(myPackVoltage, myPackVoltage / 2.0,
			    true, myChargeEstimate);
    myRobot->setChargeState(myChargeState);
    myRobot->setIsChargerPowerGood((myStatusFlags & STATUS_CHARGER_ON));
  }

  /* MPL This isn't working correctly right now since there's an issue
   * with firmware (or hardware).  Taking it out for now before the
   * clicking it can cause when stuck drives anyone crazy.

  if (myStatusFlags & STATUS_ON_BUTTON_PRESSED)
  {
    ArLog::log(ArLog::Normal, 
	       "BatteryMTX(%d) enabling motors because on button pressed",
	       myBoardNum);
    myRobot->enableMotors();
  }
  */

  // process the status info
  if ((myStatusFlags & STATUS_BATTERY_POWERING_OFF) && 
      !(myLastStatusFlags & STATUS_BATTERY_POWERING_OFF))
  {
    myBatteryPoweringOffCBList.invoke();
  }

  if (!(myStatusFlags & STATUS_BATTERY_POWERING_OFF) && 
      (myLastStatusFlags & STATUS_BATTERY_POWERING_OFF))
  {
    myBatteryPoweringOffCancelledCBList.invoke();
  }

  myLastStatusFlags = myStatusFlags;

  // process the error info
  interpErrors();

}

void ArBatteryMTX::interpErrors(void)
{
  // reset the current error conditions 
  myErrorString = "";
  myErrorCount = 0;

  // if this is our first error flag check and we have no errors then
  // just note we have no errors and clear that it's our first check
  if (myFirstErrorFlagsCheck && myErrorFlags == 0)
  {
    ArLog::log(ArLog::Normal, "%s: Connected with no errors", getName());
  } 
  // if our errors have changed or it's our first one then log the errors
  else if (myErrorFlags != myLastErrorFlags || myFirstErrorFlagsCheck)
  {
    checkAndSetCurrentErrors(ERROR_BATTERY_OVERVOLTAGE, "Battery_Overvoltage");
    checkAndSetCurrentErrors(ERROR_BATTERY_UNDERVOLTAGE, "Battery_Undervoltage");
    checkAndSetCurrentErrors(ERROR_OVERCURRENT, "Overcurrent");
    checkAndSetCurrentErrors(ERROR_BLOWNFUSE, "Blownfuse");
    checkAndSetCurrentErrors(ERROR_RTC_ERROR, "RTC_Error");
    checkAndSetCurrentErrors(ERROR_OVER_TEMPERATURE, "Over_Temperature");
    checkAndSetCurrentErrors(ERROR_MASTER_SWITCH_FAULT, "Master_Switch_Fault");
    checkAndSetCurrentErrors(ERROR_SRAM_ERROR, "SRAM_Error");
    checkAndSetCurrentErrors(ERROR_CHARGER_OUT_OF_VOLTAGE_RANGE, "Charger_Out_of_Voltage_Range");
    checkAndSetCurrentErrors(ERROR_CHARGER_CIRCUIT_FAULT, "Charger_Circuit_Fault");

    if (myFirstErrorFlagsCheck)
      ArLog::log(ArLog::Normal, "%s: Connected with error flag of 0x%04x, count of %d, and string of '%s'.", 
		 getName(),
		 myErrorFlags, myErrorCount, myErrorString.c_str());
    else
      ArLog::log(ArLog::Normal, "%s: Error flags changed to 0x%04x, count of %d, and string of '%s'.  Last error flags 0x%04x, count of %d, and string of '%s'.", 
		 getName(), 
		 myErrorFlags, myErrorCount, myErrorString.c_str(),
		 myLastErrorFlags, myLastErrorCount, myLastErrorString.c_str());

    if ((myErrorFlags & ERROR_RTC_ERROR))
    {
      if (!myHaveSetRTC) 
      {
	ArLog::log(ArLog::Normal, "%s: Setting RTC since it hasn't been set this run and there's an error with it", getName());
	sendSetRealTimeClock(time(NULL));
	myHaveSetRTC = true;
      }
      else
      {
	ArLog::log(ArLog::Normal, "%s: Not setting RTC since it's already been set once this run", getName());
      }	
    }
  }

  myFirstErrorFlagsCheck = false;
  myLastErrorFlags = myErrorFlags;
  myLastErrorCount = myErrorCount;
  myLastErrorString = myErrorString;
}

void ArBatteryMTX::checkAndSetCurrentErrors(ErrorFlags errorFlag, 
					    const char *errorString)
{
  // if there was no error return
  if (!(myErrorFlags & errorFlag))
    return;

  // if it's not the first toss a separator
  if (!myErrorString.empty())
    myErrorString += " ";
  
  // then toss in the string
  myErrorString += errorString;

  // then increment the count
  myErrorCount++;
}

AREXPORT bool ArBatteryMTX::blockingConnect (bool sendTracking, bool recvTracking)
{
	mySendTracking = sendTracking;
	myRecvTracking = recvTracking;

	myAsyncConnectState = -1;

	myDeviceMutex.lock();
	if (myConn == NULL) {
		ArLog::log (ArLog::Terse,
		            "%s: Could not connect because there is no connection defined",
		            getName());
		myDeviceMutex.unlock();
		failedToConnect();
		return false;
	}
	ArSerialConnection *serConn = NULL;
	serConn = dynamic_cast<ArSerialConnection *> (myConn);
	if (serConn != NULL)
		serConn->setBaud (115200);
	if (myConn->getStatus() != ArDeviceConnection::STATUS_OPEN
	    && !myConn->openSimple()) {
		ArLog::log (
		  ArLog::Terse,
		  "%s: Could not connect because the connection was not open and could not open it",
		  getName());
		myDeviceMutex.unlock();
		failedToConnect();
		return false;
	}
	// PS - set logging level and battery type in packet receiver class
	//myReceiver->setSync1(HEADER1);
	//m/yReceiver.setSync2(HEADER2);

	myReceiver = new ArRobotPacketReceiver(myConn, true, HEADER1, HEADER2, 
																					myRecvTracking,
																					"ArBatteryMTX");

	mySender = new ArRobotPacketSender(myConn, HEADER1, HEADER2, 
																					mySendTracking,
																					"ArBatteryMTX");

	//myReceiver->setInfoLogLevel (myInfoLogLevel);
	//myReceiver->setMyName (getName());
	//myReceiver->setDeviceConnection (myConn);
	// PS 6/5/12 - not sure why these 2 are here - commenting them out
	//myDeviceMutex.unlock();
	//myDeviceMutex.lock();
	myTryingToConnect = true;
	myDeviceMutex.unlock();
	ArTime timeDone;
	if (!timeDone.addMSec (30 * 1000)) {
		ArLog::log (ArLog::Normal,
		            "%s::blockingConnect() error adding msecs (30 * 1000)",
		            getName());
	}
	// first stop all sending by sending a stop basic, stop info and stop cell
	if (!sendBasicInfo (STOP_SENDING)) {
		ArLog::log (ArLog::Normal,
		            "%s::blockingConnect() Could not send Basic Info Request to Battery", getName());
		failedToConnect();
		return false;
	}
	if (!sendSystemInfo (STOP_SENDING)) {
		ArLog::log (ArLog::Normal,
		            "%s::blockingConnect() Could not send System Info Request to Battery", getName());
		failedToConnect();
		return false;
	}
	if (!sendCellInfo (STOP_SENDING)) {
		ArLog::log (ArLog::Normal,
		            "%s::blockingConnect() Could not send Cell Info Request to Battery", getName());
		failedToConnect();
		return false;
	}
//#if 0
	// second step is to send system and cell info requests and get responses

	if (!getSystemInfo()) {
		ArLog::log (ArLog::Normal,
		            "%s::blockingConnect() Could not get System Info from Battery", getName());
		failedToConnect();
		return false;
	}

  logBatteryInfo(ArLog::Verbose);

	// second step is send cell info requests and get response
	if (!getCellInfo()) {
		ArLog::log (ArLog::Normal,
		            "%s::blockingConnect() Could not get Cell Info from Battery", getName());
		failedToConnect();
		return false;
	}

  logCellInfo(ArLog::Verbose);


//#endif
	// third step is get one basic info packet so we can log the results
	if (!getBasicInfo()) {
		ArLog::log (ArLog::Normal,
		            "%s::blockingConnect() Could not get Basic Info from Battery", getName());
		failedToConnect();
		return false;
	}

	// set async connect to connected
	myAsyncConnectState = 3;

	if (myBoardNum == 1) {
		myRobot->setIgnoreMicroControllerBatteryInfo (true);
		myRobot->setBatteryInfo (
		  myPackVoltage,
		  myPackVoltage / 2.0,
		  true,
		  myChargeEstimate);
	}
	ArLog::log (ArLog::Normal,
	            "%s Basic Info - Charge Estimate is %g",getName(), myChargeEstimate);
	ArLog::log (ArLog::Normal,
	            "%s Basic Info - Current Draw is %g",getName(), myCurrentDraw);
	ArLog::log (ArLog::Normal,
	            "%s Basic Info - Pack Voltage is %g",getName(), myPackVoltage);
	ArLog::log (ArLog::Normal,
	            "%s Basic Info - Status Flags 0x%02x",getName(), myStatusFlags);
	ArLog::log (ArLog::Normal,
	            "%s Basic Info - Error Flags 0x%02x",getName(), myErrorFlags);
	// and finally send the basic request and start the continuous sending
	if (!sendBasicInfo (SEND_CONTINUOUS)) {
		ArLog::log (ArLog::Normal,
		            "%s::blockingConnect() Could not send Basic Info Request to Battery", getName());
		failedToConnect();
		return false;
	}
	myIsConnected = true;
	myTryingToConnect = false;
	ArLog::log (ArLog::Normal, "%s: Connection successful",
	            getName());

  myLastReading.setToNow();

	runAsync();
	return true;
} // end blockingConnect

AREXPORT const char * ArBatteryMTX::getName (void) const
{
	return myName.c_str();
}

AREXPORT void * ArBatteryMTX::runThread (void *arg)
{
	//ArBatteryMTXPacket *packet;
	ArRobotPacket *packet;

while (getRunning() )
{

	while (getRunning() && myIsConnected &&
	       (packet = myReceiver->receivePacket (500)) != NULL) {
		myPacketsMutex.lock();
		myPackets.push_back (packet);
		myPacketsMutex.unlock();
		if (myRobot == NULL)
			sensorInterp();
	}

	// if we have a robot but it isn't running yet then don't have a
	// connection failure
	if (getRunning() && myIsConnected && checkLostConnection() ) {
		ArLog::log (ArLog::Terse,
		            "%s::runThread()  Lost connection to the MTX battery because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
		            myLastReading.mSecSince() / 1000.0,
		            getConnectionTimeoutSeconds() );
		myIsConnected = false;
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
			//            "%s::runThread()  Lost connection to the battery because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
			//            myLastReading.mSecSince() / 1000.0,
			//            getConnectionTimeoutSeconds() );
			ArLog::log (ArLog::Normal,
			            "%s::runThread()  Lost connection to the battery because of error %d %d", getName(), getRunning(), myIsConnected);
			myIsConnected = false;
			//laserDisconnectOnError();
			continue;
		}
		*/
		//ArUtil::sleep(1);
		//ArUtil::sleep(2000);
		//ArUtil::sleep(500);
	
	return NULL;
}

/**
   This will check if the battery has lost connection.  If there is no
   robot it is a straightforward check of last reading time against
   getConnectionTimeoutSeconds.  If there is a robot then it will not
   start the check until the battery is running and connected.
**/
AREXPORT bool ArBatteryMTX::checkLostConnection(void)
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

AREXPORT void ArBatteryMTX::disconnectOnError(void)
{
  ArLog::log(ArLog::Normal, "%s: Disconnected because of error", getName());
  myDisconnectOnErrorCBList.invoke();
}

AREXPORT bool ArBatteryMTX::sendSystemInfo (unsigned char dataValue)
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);


	sendPacket.setID (SYSTEM_INFO);
	sendPacket.uByteToBuf (dataValue);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendSystemInfo() Could not send system info request to Battery (%d)", 
								getName(), dataValue);
		return false;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sentSystemInfo() System Info request sent to Battery (%d)", 
								getName(), dataValue);
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::sendCellInfo (unsigned char dataValue)
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (CELL_INFO);
	sendPacket.uByteToBuf (dataValue);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendCellInfo() Could not send cell info request to Battery (%d)", 
								getName(), dataValue);
		return false;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sendCellInfo() Cell Info request sent to Battery (%d)", 
								getName(), dataValue);
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::sendBasicInfo (unsigned char dataValue)
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (BASIC_INFO);
	sendPacket.uByteToBuf (dataValue);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendBasicInfo() Could not send basic info request to Battery (%d)",
								getName(), dataValue);
		return false;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sendBasicInfo() Basic Info request sent to Battery (%d)", 
								getName(), dataValue);
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::sendPowerOff()
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (POWER_OFF_REQUEST);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendPowerOff() Could not send poweroff request to Battery", getName());
		return false;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sendPowerOff() Power Off request sent to Battery", getName());
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::sendPowerOffCancel()
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (POWER_OFF_CANCEL);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendPowerOffCancel() Could not send power off cancel request to Battery", getName());
		return false;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sendPowerOffCancel() Power Off Cancel request sent to Battery", getName());
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::sendStopCharging()
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (STOP_CHARGING);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendStopCharging() Could not send stop charging request to Battery", getName());
		return false;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sendStopCharging() Stop Charging request sent to Battery", getName());
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::sendStartCharging()
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (START_CHARGING);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendStartCharging() Could not send start charging request to Battery", getName());
		return false;
	}
	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sendStartCharging() Start Charging request sent to Battery", getName());
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::sendSetPowerOffDelay (unsigned int msDelay)
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (SET_POWER_OFF_DELAY);
	sendPacket.uByte4ToBuf (msDelay);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendSetPowerOffDelay() Could not send power off delay request to Battery", getName());
		return false;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sendSetPowerOffDelay() Set Power Off Delay sent to Battery", getName());
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::sendSetRealTimeClock (unsigned int secSinceEpoch)
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (SET_REAL_TIME_CLOCK);
	sendPacket.uByte4ToBuf (secSinceEpoch);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendSetRealTimeClock() Could not send set real time clock request to Battery", getName());
		return false;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sendSetRealTimeClock() Set Real Time Clock sent to Battery", getName());
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::sendResetCellData()
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (RESET_CELL_DATA);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendResetCellData() Could not send reset cell data request to Battery", getName());
		return false;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sendResetCellData() Reset Cell Data sent to Battery", getName());
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::sendSetReserveValue (unsigned short hundredthOfPercent)
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (SET_RESERVE_VALUE);
	sendPacket.uByte4ToBuf (hundredthOfPercent);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendSetReserveValue() Could not send set reserve value request to Battery", getName());
		return false;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sendSetReserveValue() Set Reserve Value sent to Battery", getName());
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::sendSetBalanceValue (unsigned short hundredthOfPercent)
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (SET_BALANCE_VALUE);
	sendPacket.uByte4ToBuf (hundredthOfPercent);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendSetBalanceValue() Could not send set balance value request to Battery", getName());
		return false;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sendSetBalanceValue() Set Balance Value sent to Battery", getName());
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::sendEmergencyPowerOff()
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	//sendPacket.uByteToBuf (EMERGENCY_OFF);
	sendPacket.setID (EMERGENCY_OFF);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendEmergencyPowerOff() Could not send emergency off request to Battery", getName());
		return false;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::sendEmergencyPowerOff() Emergency Off sent to Battery", getName());
	); // end IFDEBUG

	return true;
}

AREXPORT bool ArBatteryMTX::getSystemInfo()
{
	//ArBatteryMTXPacket *packet;
	ArRobotPacket *packet;

	bool gotPacket = false;
	unsigned char *buf;
	for (int i = 0; i < 5; i++) {
		if (!sendSystemInfo (SEND_ONCE)) {
			ArLog::log (ArLog::Normal,
			            "%s::getSystemInfo() Could not send system info to battery",
			            getName());
			return false;
		}
		myAsyncConnectState = 0;
		
		packet = myReceiver->receivePacket (1000);
		if (packet == NULL) {
			ArLog::log (ArLog::Normal,
			            "%s::getSystemInfo() No response to get system info - resending", getName());
			continue;
		}
		buf = (unsigned char *) packet->getBuf();
		// verify
		//if ( (buf[0] != SYSTEM_INFO) || (packet->getLength() != SYSTEM_INFO_SIZE)) {
		if ( (packet->getID() != SYSTEM_INFO) || (packet->getLength() != SYSTEM_INFO_SIZE)) {
			ArLog::log (ArLog::Normal,
			            "%s::getSystemInfo() Invalid response from battery to get system info (0x%x 0x%x)",
			            getName(), packet->getID(), packet->getLength());
			delete packet;
			continue;
		} else {
			updateSystemInfo (&buf[3]);
			delete packet;
			gotPacket = true;
			break;
		}
	} // endfor
	if (!gotPacket) {
		ArLog::log (ArLog::Normal,
		            "%s::getSystemInfo() Cannot get system info from battery",
		            getName());
		return false;
	}
	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::getSystemInfo() - successful", getName()));
	return true;
}

AREXPORT bool ArBatteryMTX::getBasicInfo()
{
	//ArBatteryMTXPacket *packet;
	ArRobotPacket *packet;

	bool gotPacket = false;
	unsigned char *buf;
	for (int i = 0; i < 5; i++) {
		if (!sendBasicInfo (SEND_ONCE)) {
			ArLog::log (ArLog::Normal,
			            "%s::getBasicInfo() Could not send basic info to battery",
			            getName());
			return false;
		}

		myAsyncConnectState = 2;

		packet = myReceiver->receivePacket (1000);
		if (packet == NULL) {
			ArLog::log (ArLog::Normal,
			            "%s::getBasicInfo() No response to get basic info - resending", getName());
			continue;
		}
		buf = (unsigned char *) packet->getBuf();
		// verify
		if ( (packet->getID() != BASIC_INFO) || (packet->getLength() != BASIC_INFO_SIZE)) {
			ArLog::log (ArLog::Normal,
			            "%s::getBasicInfo() Invalid response from battery to get basic info (0x%x 0x%x)",
			            getName(), packet->getID(), packet->getLength());
			delete packet;
			continue;
		} else {
			char obuf[1024];
			obuf[0] = '\0';
			int j = 0;
			for (int i = 0; i < packet->getLength() - 2; i++) {
				sprintf (&obuf[j], "_%02x", buf[i]);
				j= j+3;
			}
IFDEBUG(
			ArLog::log (ArLog::Normal,
			            "%s::getBasicInfo() packet = %s",getName(), obuf);
)
			updateBasicInfo (&buf[3]);
			delete packet;
			gotPacket = true;
			break;
		}
	} // endfor
	if (!gotPacket) {
		ArLog::log (ArLog::Normal,
		            "%s::getBasicInfo() Cannot get basic info from battery",
		            getName());
		return false;
	}
	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::getBasicInfo() - successful", getName()));
	return true;
}

AREXPORT bool ArBatteryMTX::getCellInfo()
{
	unsigned char *buf;
	//ArBatteryMTXPacket *packet;
	ArRobotPacket *packet;
	bool gotPacket = false;
	for (int i = 0; i < 5; i++) {
		if (!sendCellInfo (SEND_ONCE)) {
			ArLog::log (ArLog::Normal,
			            "%s::getCellInfo() Could not send cell info to battery",
			            getName());
			return false;
		}
		myAsyncConnectState = 1;

		packet = myReceiver->receivePacket (1000);
		if (packet == NULL) {
			ArLog::log (ArLog::Normal,
			            "%s::getCellInfo() No response to get cell info - resending", getName());
			continue;
		}

		myCellPacket.duplicatePacket(packet);

		buf = (unsigned char *) packet->getBuf();
		// verify
		if ( (packet->getID() != CELL_INFO) || (packet->getLength() != CELL_INFO_SIZE)) {
			ArLog::log (ArLog::Normal,
			            "%s::getCellInfo() Invalid response from battery to get cell info (0x%x 0x%x)",
			            getName(), packet->getID(), packet->getLength());
			delete packet;
			continue;
		} else {
			updateCellInfo (&buf[3]);
			delete packet;
			gotPacket = true;
			break;
		}
	} // endfor
	if (!gotPacket) {
		ArLog::log (ArLog::Normal,
		            "%s::getCellInfo() Cannot get cell info from battery",
		            getName());
		return false;
	}
	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::getCellInfo() - successful", getName()));
	return true;
}

AREXPORT void ArBatteryMTX::updateSystemInfo (unsigned char *buf)
{
	myId = buf[1];
	myFirmwareVersion = buf[2];
	mySerialNumber = buf[6] << 24 | buf[5] << 16 | buf[4] << 8 | buf[3];
	myCurrentTime = buf[14] << 56 | buf[13] << 48 | buf[12] << 40 | buf[11] << 32 | buf[10] << 24 | buf[9] << 16 | buf[8] << 8 | buf[7];
	myLastChargeTime = buf[22] << 56 | buf[21] << 48 | buf[20] << 40 | buf[19] << 32 | buf[18] << 24 | buf[17] << 16 | buf[16] << 8 | buf[15];
	myChargeRemainingEstimate = buf[26] << 24 | buf[25] << 16 | buf[24] << 8 | buf[23];
	myCapacityEstimate = buf[30] << 24 | buf[29] << 16 | buf[28] << 8 | buf[27];
	myRawDelay = buf[34] << 24 | buf[33] << 16 | buf[32] << 8 | buf[31];
	myDelay = myRawDelay / 1000.0;
	myCycleCount = buf[36] << 8 | buf[35];
	myRawTemperature = buf[38] << 8 | buf[37];
	myTemperature = myRawTemperature / 100.0;
	myRawPaddleVolts = buf[40] << 8 | buf[39];
	myPaddleVolts = myRawPaddleVolts / 1000.0;
	myRawVoltage = buf[42] << 8 | buf[41];
	myVoltage = myRawVoltage / 1000.0;
	myRawFuseVoltage = buf[44] << 8 | buf[43];
	myFuseVoltage = myRawFuseVoltage / 1000.0;
	myRawChargeCurrent = buf[46] << 8 | buf[45];
	myChargeCurrent = myRawChargeCurrent / 1000.0;
	myRawDisChargeCurrent = buf[48] << 8 | buf[47];
	myDisChargeCurrent = myRawDisChargeCurrent / 1000.0;
	myRawCellImbalance = buf[50] << 8 | buf[49];
	myCellImbalance = myRawCellImbalance / 100.0;
	myRawImbalanceQuality = buf[52] << 8 | buf[51];
	myImbalanceQuality = myRawImbalanceQuality / 100.0;
	myRawReserveChargeValue = buf[54] << 8 | buf[53];
	myReserveChargeValue = myRawReserveChargeValue / 100.0;
}

AREXPORT void ArBatteryMTX::updateCellInfo (unsigned char *buf)
{
	myNumCells = buf[1];
	int idx;
	for (int i = 0; i < myNumCells; i++) {
		// PS - note - need to hard code this as if we used the sizeof cellinfo, we'd
		// get 12 as unsigned char flages is actually 2 bytes
		idx = 11 * i;
		CellInfo *info = new CellInfo();
		info->myCellFlags = buf[2 + idx];
		info->myRawCellVoltage = buf[4 + idx] << 8 | buf[3  + idx];
		info->myCellVoltage = info->myRawCellVoltage / 1000.0;
		info->myCellCharge = buf[8 + idx] << 24 | buf[7 + idx] << 16 | buf[6 + idx] << 8 | buf[5 + idx];
		info->myCellCapacity = buf[12 + idx] << 24 | buf[11 + idx] << 16 | buf[10 + idx] << 8 | buf[9 + idx];
		myCellNumToInfoMap[i] = info;
	}
}

AREXPORT void ArBatteryMTX::updateBasicInfo (unsigned char *buf)
{
	myRawChargeEstimate = buf[2] << 8 | buf[1];
	myChargeEstimate = myRawChargeEstimate / 100.0;
	myRawCurrentDraw = buf[4] << 8 | buf[3];
	myCurrentDraw = myRawCurrentDraw / 100.0;
	myRawPackVoltage = buf[6] << 8 | buf[5];
	myPackVoltage = myRawPackVoltage / 1000.0;
	myStatusFlags = buf[8] << 8 | buf[7];
	myErrorFlags = buf[10] << 8 | buf[9];
}


AREXPORT void ArBatteryMTX::logBatteryInfo(ArLog::LogLevel level)
{
	ArLog::log (level,
	            "%s ID = %d", getName(), myId);
	ArLog::log (level,
	            "%s Firmware Version = %d", getName(), myFirmwareVersion);
	ArLog::log (level,
	            "%s Serial Number = %d", getName(), mySerialNumber);
	ArLog::log (level,
	            "%s Current Time = %s", getName(), ctime((time_t *)&myCurrentTime));
	            //"%s Current Time = %lld", getName(), myCurrentTime);
	ArLog::log (level,
	            "%s Last Charge Time = %s", getName(), ctime((time_t *)&myLastChargeTime));
	            //"%s Last Charge Time = %lld", getName(), myLastChargeTime);
	ArLog::log (level,
	            "%s Charge Remaining Estimate = %u Coulombs", getName(), myChargeRemainingEstimate);
	ArLog::log (level,
	            "%s Capacity Estimate = %u Coulombs", getName(), myCapacityEstimate);
	ArLog::log (level,
	            "%s Delay = %g seconds", getName(), myDelay);
	ArLog::log (level,
	            "%s Cycle Count = %d", getName(), myCycleCount);
	ArLog::log (level,
	            "%s Temperature = %g Celsius", getName(), myTemperature);
	ArLog::log (level,
	            "%s Paddle Volts = %g volts", getName(), myPaddleVolts);
	ArLog::log (level,
	            "%s Voltage = %g volts", getName(), myVoltage);
	ArLog::log (level,
	            "%s Fuse Voltage = %g volts", getName(), myFuseVoltage);
	ArLog::log (level,
	            "%s Charge Current = %g amps", getName(), myChargeCurrent);
	ArLog::log (level,
	            "%s DisCharge Current = %g amps", getName(), myDisChargeCurrent);
	ArLog::log (level,
	            "%s Cell Imbalance = %g", getName(), myCellImbalance);
	ArLog::log (level,
	            "%s Imbalance Quality = %g", getName(), myImbalanceQuality);
	ArLog::log (level,
	            "%s Reserve Charge Value = %g", getName(), myReserveChargeValue);
}

AREXPORT void ArBatteryMTX::logCellInfo(ArLog::LogLevel level)
{
	ArLog::log (level,
	            "%s Number of Cells = %d", getName(), myNumCells);
	for (std::map<int, CellInfo *>::iterator iter =
	       myCellNumToInfoMap.begin();
	     iter != myCellNumToInfoMap.end();
	     iter++) {
		CellInfo *info = iter->second;
		ArLog::log (level,
		            "%s Cell(%d) Flag = 0x%02x", getName(), iter->first, info->myCellFlags);
		ArLog::log (level,
		            "%s Cell(%d) Voltage = %g volts", getName(), iter->first, info->myCellVoltage);
		ArLog::log (level,
		            "%s Cell(%d) Charge = %u Coulombs", getName(), iter->first, info->myCellCharge);
		ArLog::log (level,
		            "%s Cell(%d) Capacity = %u Coulombs", getName(), iter->first, info->myCellCapacity);
	}
}

