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
#include "ArSonarMTX.h"
#include "ArSensorReading.h"
//#include "ArRobot.h"

#include "ariaOSDef.h"
#include "ArSerialConnection.h"
#include "ariaInternal.h"
#include <time.h>


//#define TRACE 1
#if (defined(TRACE))
#define IFDEBUG(code) {code;}
#else
#define IFDEBUG(code)
#endif


AREXPORT ArSonarMTX::ArSonarMTX (int sonarBoardNum,
																	const char *name, ArDeviceConnection *conn,
                                 ArRobot *robot) :
	mySensorInterpTask (this, &ArSonarMTX::sensorInterp),
	myConn (conn),
	myName (name),
  myTimeoutSeconds(1.0),
  myRobotRunningAndConnected(false),
	myReadingCurrentCount(0),
	myReadingCount(0),
	myRobot(NULL),
	myBoardNum (sonarBoardNum),
	myWarnedAboutExtraSonar(false),
	mySendTracking(false),
	myRecvTracking(false),
  mySendTrackingSet(false),
  myRecvTrackingSet(false),
	myReceiver(NULL),
	mySender(NULL),
	myFirmwareVersion(0),
	myAriaExitCB (this, &ArSonarMTX::disconnect)
{

	mySonarMap.clear();

	myInfoLogLevel = ArLog::Verbose;
  ArThread::setLogLevel(myInfoLogLevel);

	clear();

	setRobot (robot);
	sonarSetName(name);

	sprintf(myNameWithBoard, "%s%d", name, sonarBoardNum);

	ArLog::log (myInfoLogLevel, "%s::ArSonarMTX initializing",
		            getNameWithBoard());


	Aria::addExitCallback (&myAriaExitCB, -10);



}

AREXPORT ArSonarMTX::~ArSonarMTX()
{
	if (myRobot != NULL) {
		myRobot->remSensorInterpTask (&myProcessCB);
	}
  Aria::remExitCallback(&myAriaExitCB);
}


AREXPORT void ArSonarMTX::setDeviceConnection (
  ArDeviceConnection *conn)
{
	myConn = conn;
  myConn->setDeviceName(getName());
}


AREXPORT ArDeviceConnection *ArSonarMTX::getDeviceConnection (void)
{
	return myConn;
}

AREXPORT void ArSonarMTX::setRobot (ArRobot *robot)
{
	myRobot = robot;


	// this is the code from the laser, i changed the priority to 92 from 90
	// also it puts in mysensorinterptask instead of myprocesscb
	if (myRobot != NULL) {
		myRobot->remSensorInterpTask (&mySensorInterpTask);
		myRobot->addSensorInterpTask (myName.c_str(), 92, &mySensorInterpTask);
	}

	if ((robot != NULL) && (robot->getRobotParams())) {

		myBoardDelay = robot->getRobotParams()->getSonarMTXBoardDelay (myBoardNum);

		// The following params can be overridden if they are configured in the units
		myBoardGain = robot->getRobotParams()->getSonarMTXBoardGain (myBoardNum);

		myBoardDetectionThreshold = robot->getRobotParams()->getSonarMTXBoardDetectionThreshold (myBoardNum);

/* - no longer supported
		myBoardNoiseDelta = robot->getRobotParams()->getSonarMTXBoardNoiseDelta (myBoardNum);
*/

		myBoardMaxRange = robot->getRobotParams()->getSonarMTXBoardMaxRange (myBoardNum);
		myBoardMaxRange = myBoardMaxRange/17;

		myBoardUseForAutonomousDriving = robot->getRobotParams()->getSonarMTXBoardUseForAutonomousDriving (myBoardNum);

		ArLog::log (ArLog::Verbose, "%s::setRobot() Sonar board %d delay %d",
		            getNameWithBoard(), myBoardNum, myBoardDelay);
		ArLog::log (ArLog::Verbose, "%s::setRobot() Sonar board %d gain %d",
		            getNameWithBoard(), myBoardNum, myBoardGain);
		ArLog::log (ArLog::Verbose, "%s::setRobot() Sonar board %d detection threshold %d",
		            getNameWithBoard(), myBoardNum, myBoardDetectionThreshold);
/* - no longer supported
		ArLog::log (ArLog::Verbose, "%s::setRobot() Sonar board %d noise delta %d",
		            getNameWithBoard(), myBoardNum, myBoardNoiseDelta);
*/
		ArLog::log (ArLog::Verbose, "%s::setRobot() Sonar board %d max range %d (note it's value is divided by 17 from config page)",
		            getNameWithBoard(), myBoardNum, myBoardMaxRange);
		ArLog::log (ArLog::Verbose, "%s::setRobot() Sonar board %d use for autonomous driving %d",
		            getNameWithBoard(), myBoardNum, myBoardUseForAutonomousDriving);

		// go thru each configure sonarunit, see if it's for this board, if
		// so then load the config values into our map

		myNumConfiguredTransducers = 0;
		myTransducerMaskLSB = 0;
		myTransducerMaskMSB = 0;
		myAutonomousDrivingTransducerMaskLSB = 0;
		myAutonomousDrivingTransducerMaskMSB = 0;
		myTransducersAreOn = false;

//* MPL for PS TODO 

    int n = robot->getRobotParams()->getNumSonarUnits();

		ArLog::log (myInfoLogLevel, "%s::setRobot() Sonar board %d can have a max %d sonar units",
		            getNameWithBoard(), myBoardNum, n);
    if(n <= 0)
    {
      ArLog::log(ArLog::Normal, "%s: Error: no sonar parameters set in robot parameters!", getNameWithBoard());
		  return;
    }

    for (int i = 0; i < n; ++i) {

			//if (mySonars.find(i) == mySonars.end()) {


			int t = robot->getRobotParams()->getSonarMTXBoardUnitPosition (i);
 
			// PS 1/24/13 - decrement t by 1 as on the board itself the 
			// units are numbered 1-8, but the protocol uses 0-7

			if (t == 0) {
				ArLog::log (ArLog::Normal, "%s::setRobot(): Sonar %d has SonarBoardUnitPosition of 0 - ignoring",
		            getNameWithBoard(), i+1);

				continue;
			}
			t = t-1;


			// end PS

			mySonarMap[t][SONAR_IS_CONFIGURED] = false;

			if (robot->getRobotParams()->getSonarMTXBoard (i) != myBoardNum) {
				ArLog::log (ArLog::Normal, "%s::setRobot(): Sonar_%d has a mismatched SonarBoard number %d - ignoring",
		            getNameWithBoard(), i+1, myBoardNum);
				continue;
			}

			if (t > 7)
				myTransducerMaskMSB = (1 << t) | myTransducerMaskMSB;
			else
				myTransducerMaskLSB = (1 << t) | myTransducerMaskLSB;

			mySonarMap[t][SONAR_IS_CONFIGURED] = true;
			mySonarMap[t][SONAR_MAPPING] = i;
			mySonarMap[t][SONAR_X] = robot->getRobotParams()->getSonarX (i);
			mySonarMap[t][SONAR_Y] = robot->getRobotParams()->getSonarY (i);
			mySonarMap[t][SONAR_TH] = robot->getRobotParams()->getSonarTh (i);
			mySonarMap[t][SONAR_GAIN] = robot->getRobotParams()->getSonarGain (i);
			mySonarMap[t][SONAR_DETECTION_THRES] = robot->getRobotParams()->getSonarDetectionThreshold (i);
/* - no longer supported
			mySonarMap[t][SONAR_NOISE_DELTA] = robot->getRobotParams()->getSonarNoiseDelta (i);
*/
			mySonarMap[t][SONAR_MAX_RANGE] = robot->getRobotParams()->getSonarMaxRange (i)/17;
			mySonarMap[t][SONAR_USE_FOR_AUTONOMOUS_DRIVING] = robot->getRobotParams()->getSonarUseForAutonomousDriving (i);

			myNumConfiguredTransducers++;

			if (mySonarMap[t][SONAR_GAIN] == 0)
				mySonarMap[t][SONAR_GAIN] = myBoardGain;

			if (mySonarMap[t][SONAR_DETECTION_THRES] == 0)
				mySonarMap[t][SONAR_DETECTION_THRES] = myBoardDetectionThreshold;


			if (mySonarMap[t][SONAR_USE_FOR_AUTONOMOUS_DRIVING]) {
				if (t > 7) 
					myAutonomousDrivingTransducerMaskMSB = (1 << t) | myAutonomousDrivingTransducerMaskMSB;
				else 
					myAutonomousDrivingTransducerMaskLSB = (1 << t) | myAutonomousDrivingTransducerMaskLSB;
			}

/* - no longer supported
			if (mySonarMap[t][SONAR_NOISE_DELTA] == 0)
				mySonarMap[t][SONAR_NOISE_DELTA] = myBoardNoiseDelta;
*/

			if (mySonarMap[t][SONAR_MAX_RANGE] == 0)
				mySonarMap[t][SONAR_MAX_RANGE] = myBoardMaxRange;

			ArLog::log (myInfoLogLevel, "%s::setRobot() Sonar_%d params %d %d %d %d %d %d %d %d",
			            getNameWithBoard(), i+1, mySonarMap[t][SONAR_MAPPING]+1,
			            mySonarMap[t][SONAR_X],
			            mySonarMap[t][SONAR_Y],
			            mySonarMap[t][SONAR_TH],
			            mySonarMap[t][SONAR_GAIN],
									/*
			            mySonarMap[t][SONAR_NOISE_DELTA],
									*/
			            mySonarMap[t][SONAR_DETECTION_THRES],
			            mySonarMap[t][SONAR_MAX_RANGE],
									mySonarMap[t][SONAR_USE_FOR_AUTONOMOUS_DRIVING]);

		} // end for

		ArLog::log (ArLog::Verbose, "%s::setRobot() Number of configured sonar units = %d",
			            getNameWithBoard(), myNumConfiguredTransducers);

		ArLog::log (ArLog::Verbose, "%s::setRobot() Sonar mask MSB (0x%x) LSB (0x%x)",
			            getNameWithBoard(), myTransducerMaskMSB, myTransducerMaskLSB);

		ArLog::log (ArLog::Verbose, "%s::setRobot() Sonar use for autonomous driving mask MSB (0x%x) LSB (0x%x)",
			            getNameWithBoard(), myAutonomousDrivingTransducerMaskMSB, myAutonomousDrivingTransducerMaskLSB);

	}

}


void ArSonarMTX::clear (void)
{
	myIsConnected = false;
	myTryingToConnect = false;
	myStartConnect = false;

}

AREXPORT void ArSonarMTX::sonarSetName (const char *name)
{
	myName = name;
	myDeviceMutex.setLogNameVar ("%s::myDeviceMutex", getNameWithBoard());
	myPacketsMutex.setLogNameVar ("%s::myPacketsMutex", getNameWithBoard());
	myDataMutex.setLogNameVar ("%s::myDataMutex", getNameWithBoard());
	myAriaExitCB.setNameVar ("%s::exitCallback", getNameWithBoard());
  myDisconnectOnErrorCBList.setNameVar(
	  "%s::myDisconnectOnErrorCBList", myName.c_str());
  
	
}


AREXPORT int ArSonarMTX::getReadingCount()
{
  if (myTimeLastReading == time(NULL))
    return myReadingCount;
  if (myTimeLastReading == time(NULL) - 1)
    return myReadingCurrentCount;
  return 0;
}

AREXPORT void ArSonarMTX::internalGotReading(void)
{
  IFDEBUG(puts("internalGotReading");)
  if (myTimeLastReading != time(NULL)) 
  {
    myTimeLastReading = time(NULL);
    myReadingCount = myReadingCurrentCount;
    myReadingCurrentCount = 0;
  }
  myReadingCurrentCount++;

  myLastReading.setToNow();
  
}

AREXPORT bool ArSonarMTX::disconnect (void)
{
	if (!isConnected())
		return true;

	ArLog::log (ArLog::Normal, "%s: Disconnecting", getNameWithBoard());

  if(myConn)
    myConn->close();

	return true;
}

void ArSonarMTX::failedToConnect (void)
{

	ArLog::log (ArLog::Normal,
	            "%s:failedToConnect Cound not connect to sonar",
	            getNameWithBoard());

	myDeviceMutex.lock();
	myTryingToConnect = true;
	myDeviceMutex.unlock();
}

void ArSonarMTX::sensorInterp (void)
{
	//ArSonarMTXPacket *packet;
	ArRobotPacket *packet;

	int sonarNum;
	int sonarRange;
	int sonarRangeConverted;

	while (1) {
		myPacketsMutex.lock();

		if (myPackets.empty()) {
			myPacketsMutex.unlock();
			return;
		}

		myLastReading.setToNow();
		internalGotReading();

		packet = myPackets.front();
		myPackets.pop_front();
		myPacketsMutex.unlock();

		unsigned char *buf = (unsigned char *) packet->getBuf();

		// make sure its a data packet with 7 bytes
//packet->getLength()
		if ( (packet->getID() != START_SCAN) || (packet->getLength() != 10)) {
			ArLog::log (ArLog::Normal,
			            "%s:sensorInterp Could not process packet, command or packet length is invalid %02x %02x %02x %02x %02x %02x %02x %02x %d",
			            getNameWithBoard(), buf[0], buf[1], buf[2], buf[3],buf[4], buf[5], buf[6],packet->getLength());

			delete packet;
			continue;
		}

		ArTime time = packet->getTimeReceived();

		sonarNum = (buf[5] << 8) | (buf[4]);

		// make sure the sonar number is in range

#if 0 // for raw trace
				char obuf[256];
				obuf[0] = '\0';
				int j = 0;
				for (int i = 0; i < packet->getLength() - 2; i++) {
					sprintf (&obuf[j], "_%02x", buf[i]);
					j= j+3;
				}
				ArLog::log (ArLog::Normal,
				            "%s::sensorInterp() packet = %s",getNameWithBoard(), obuf);
#endif

		if (sonarNum > myNumTransducers) {
			ArLog::log (ArLog::Normal,
			            "%s:sensorInterp Could not process packet, transducer number is invalid %d %d",
			            getNameWithBoard(), sonarNum, myNumTransducers);
			delete packet;
			continue;
		}


		// map the sonar num to the configured sonar num

		std::map<int, std::map<int, int> >::iterator iter2 = 
										mySonarMap.find(sonarNum);

		if (iter2 == mySonarMap.end()) {
				if (!myWarnedAboutExtraSonar)  {
					ArLog::log(ArLog::Normal, "Robot gave back extra sonar reading!  Either the parameter file for the robot or the firmware needs updating.");
					myWarnedAboutExtraSonar = true;
				}

				delete packet;
				continue;
		}

		// ignore any readings from unconfigured transducers
		if (mySonarMap[sonarNum][SONAR_IS_CONFIGURED] == false)
		{
		  delete packet;
		  continue;
		}

		int mappedSonarNum = mySonarMap[sonarNum][SONAR_MAPPING];

#if 0
		if ( (buf[7] == 0xff) && (buf[6] == 0xff)) {
			ArLog::log(ArLog::Normal,
					"%s:sensorInterp Could not process packet, range is invalid for %d",
					getNameWithBoard(), sonarNum);
			delete packet;
			continue;
		}
#endif


		sonarRange = (buf[7] << 8) | (buf[6]);

		sonarRangeConverted = ArMath::roundInt (
		                        sonarRange * myRobot->getRobotParams()->getRangeConvFactor());


		//ArLog::log(ArLog::Normal,
		//			"%s:sensorInterp range is valid = %d for transducer %d mapped sonar = %d",
		//					getNameWithBoard(), sonarRangeConverted, sonarNum, mappedSonarNum);

		myDeviceMutex.lock();

			if (sonarRangeConverted == 0) {
				// make range max range 0xffff + 1
				sonarRangeConverted = 65536;
			}
#if 0 // for raw trace
			if (sonarRangeConverted == 0) {
				char obuf[256];
				obuf[0] = '\0';
				int j = 0;
				for (int i = 0; i < packet->getLength() - 2; i++) {
					sprintf (&obuf[j], "_%02x", buf[i]);
					j= j+3;
				}
				ArLog::log (ArLog::Normal,
				            "%s::sensorInterp() packet = %s num = %d mapped = %d range = %d",getNameWithBoard(), obuf, sonarNum, mappedSonarNum, sonarRangeConverted);
}
#endif

		mySonarMap[sonarNum][SONAR_LAST_READING] = sonarRangeConverted;

		//char charSonarNum = (char)sonarNum;
		//sprintf(charSonarNum, "%d", sonarNum);
		myRobot->processNewSonar (mappedSonarNum, sonarRangeConverted, time);

		myDeviceMutex.unlock();

		delete packet;

	} // end while
}

AREXPORT bool ArSonarMTX::blockingConnect (bool sendTracking, bool recvTracking )
{
  if (!mySendTrackingSet)
	  mySendTracking = sendTracking;
  if (!myRecvTrackingSet)
	  myRecvTracking = recvTracking;

	myDeviceMutex.lock();

  if( myNumConfiguredTransducers == 0 || (myRobot && myRobot->getNumSonar() == 0) )
  {
    ArLog::log(ArLog::Terse, "%s: Error: No transducers configured or No sonar set up in parameters.",
      getNameWithBoard());
    myDeviceMutex.unlock();
    failedToConnect();
    return false;
  }

	if (myConn == NULL) {
		ArLog::log (ArLog::Terse,
		            "%s: Could not connect because there is no connection defined",
		            getNameWithBoard());
		myDeviceMutex.unlock();
		failedToConnect();
		return false;
	}

	ArSerialConnection *serConn = NULL;
	serConn = dynamic_cast<ArSerialConnection *> (myConn);

	if (serConn != NULL)
  {
    ArLog::log(myInfoLogLevel, "ArSonarMTX::blockingConnect: Forcing baud rate to 115200...");
		serConn->setBaud (115200);
  }

	if (myConn->getStatus() != ArDeviceConnection::STATUS_OPEN
	    && !myConn->openSimple()) {
		ArLog::log (
		  ArLog::Terse,
		  "%s: Could not connect because the connection was not open and could not open it",
		  getNameWithBoard());
		myDeviceMutex.unlock();
		failedToConnect();
		return false;
	}

	// PS - set logging level and laser type in packet receiver class

	myReceiver = new ArRobotPacketReceiver(myConn, true, HEADER1, HEADER2, 
																					myRecvTracking,
																					"ArSonarMTX");

	mySender = new ArRobotPacketSender(myConn, HEADER1, HEADER2, 
																					mySendTracking,
																					"ArSonarMTX");

	//myReceiver->setMyInfoLogLevel (myInfoLogLevel);
	//myReceiver->setMyName (getNameWithBoard());
	/// MPL added these lines to help someone debug sonar ESD stuff
  if (myRecvTracking)
	  myReceiver->setTracking(true);
	myReceiver->setTrackingLogName(getName());
  if (mySendTracking)
    mySender->setTracking(true);
  mySender->setTrackingLogName(getName());
	

//	myReceiver->setDeviceConnection (myConn);
	myDeviceMutex.unlock();

	myDeviceMutex.lock();
	myTryingToConnect = true;
	myDeviceMutex.unlock();


	ArTime timeDone;

	if (!timeDone.addMSec (30 * 1000)) {
		ArLog::log (ArLog::Normal,
		            "%s::blockingConnect() error adding msecs (30 * 1000)",
		            getNameWithBoard());
	}

	//ArSonarMTXPacket *packet;
	ArRobotPacket *packet;

	if (!sendAlive()) {
		ArLog::log (ArLog::Normal,
		            "%s::blockingConnect() Could not send Alive to Sonar", getNameWithBoard());
		failedToConnect();
		return false;
	}

	do {
		//ArLog::log(ArLog::Normal,
		//		"%s::blockingConnect() calling receive packet", getNameWithBoard());
		packet = myReceiver->receivePacket (1000);

		if (packet != NULL) {
			// verify alive received
			//ArLog::log (ArLog::Normal,
		     //       "%s::blockingConnect() Response to Alive received from Sonar", getNameWithBoard());
			//unsigned char command = packet->bufToUByte();
			if (packet->getID() == ALIVE) {
				delete packet;
				packet = NULL;
				myDeviceMutex.lock();
				myIsConnected = true;
				myTryingToConnect = false;
				myDeviceMutex.unlock();

				IFDEBUG(ArLog::log(ArLog::Normal, "%s::blockingConnect() Alive message received from sonar", getNameWithBoard());)

        const int cmdDelay = 100;

				ArUtil::sleep(cmdDelay);

        
				// send a stop
				if (!sendStop()) {
					ArLog::log (ArLog::Normal,
					            "%s::blockingConnect() Could not send stop to Sonar", getNameWithBoard());
					failedToConnect();
					return false;
				}

				ArUtil::sleep(cmdDelay);

				if (!queryFirmwareVersion()) {
					ArLog::log (ArLog::Normal,
					            "%s::blockingConnect() Could not get firmware version", getNameWithBoard());
					failedToConnect();
					return false;
				}

				ArUtil::sleep(cmdDelay);

				if (!validateTransducers()) {
					ArLog::log (ArLog::Normal,
					            "%s::blockingConnect() Could not validate transducers", getNameWithBoard());
					failedToConnect();
					return false;
				}

				ArUtil::sleep(cmdDelay);

#if 0 // temp for adam

				if (!validateDelay()) {
					ArLog::log (ArLog::Normal,
					            "%s::blockingConnect() Could not validate delay", getNameWithBoard());
					failedToConnect();
					return false;
				}
#endif // tem for adam
				ArUtil::sleep(cmdDelay);

				if (!validateGain()) {
					ArLog::log (ArLog::Normal,
					            "%s::blockingConnect() Could not validate gain", getNameWithBoard());
					failedToConnect();
					return false;
				}

				ArUtil::sleep(cmdDelay);

#if 0
				if (!validateNumThresholdRanges()) {
					ArLog::log (ArLog::Normal,
					            "%s::blockingConnect() Could not validate num threshold ranges", getNameWithBoard());
					failedToConnect();
					return false;
				}

        ArUtil::sleep(cmdDelay);
#endif

				if (!validateThresholds()) {
					ArLog::log (ArLog::Normal,
					            "%s::blockingConnect() Could not validate thresholds", getNameWithBoard());
					failedToConnect();
					return false;
				}
 

/* this is no longer supported - 

				ArUtil::sleep(cmdDelay);

				if (!validateNoiseDelta()) {
					ArLog::log (ArLog::Normal,
					            "%s::blockingConnect() Could not validate noise delta", getNameWithBoard());
					failedToConnect();
					return false;
				}
*/
				ArUtil::sleep(cmdDelay);

      
//#if 0 // temp for adam

				if (!validateMaxRange()) {
					ArLog::log (ArLog::Normal,
					            "%s::blockingConnect() Could not validate max range", getNameWithBoard());
					failedToConnect();
					return false;
				}

//#endif // end temp for adam
#if 0				
				unsigned char bitMask[8] = {0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
        unsigned char lsb, msb;				

				if (myNumTransducers < 9) { 
					lsb = bitMask[myNumTransducers - 1];
					msb = 0;
				}
				else {
					lsb = 0xff;
					msb = bitMask[myNumTransducers - 8];
				}
#endif

				ArUtil::sleep(cmdDelay);
       


				if (!sendSetMask (myTransducerMaskLSB, myTransducerMaskMSB)) {
					ArLog::log (ArLog::Normal,
					            "%s::blockingConnect() Could not send set mask to Sonar", getNameWithBoard());
					failedToConnect();
					return false;
				}

				ArUtil::sleep(cmdDelay);

  


				if (!sendGetMask()) {
					ArLog::log (ArLog::Normal,
		            "%s::blockingConnect() Could not send getmask to Sonar", getNameWithBoard());
					failedToConnect();
					return false;
				}


				packet = myReceiver->receivePacket (1000);

				if (packet == NULL) {
					ArLog::log (ArLog::Normal,
										"%s::blockingConnect() Receive of transducer mask failed", getNameWithBoard());
					failedToConnect();
					return false;
				}

				unsigned char *maskBuf = (unsigned char *) packet->getBuf();

				// verify get num trans received
				if ( maskBuf[3] != GET_TRANSDUCER_MASK) {
					ArLog::log (ArLog::Normal,
			            "%s::blockingConnect() Receive invalid response to get transducer mask", getNameWithBoard());
					failedToConnect();
					return false;

				}


      IFDEBUG(
				ArLog::log (ArLog::Normal,
			            "%s::blockingConnect() Transducer mask LSB (0x%02x) MSB (0x%02x)", 
									getNameWithBoard(), maskBuf[5], maskBuf[4]);
      )

				myTransducersAreOn = true;
		
    	  // send start
        ArUtil::sleep(cmdDelay);
        if(mySendTracking)
          ArLog::log(ArLog::Normal, "%s::blockingConnect(): sending START...", getNameWithBoard());
				if (!sendStart()) {
					ArLog::log (ArLog::Normal,
					            "%s::blockingConnect() Could not send start scan to Sonar", getNameWithBoard());
					failedToConnect();
					return false;
				}

#if 0
				int j = 3;
				int t;

				for (t = 0; t < j; t++) {
					packet = myReceiver->receivePacket (1000);


					if (packet != NULL) {
						int i;
						unsigned char *buf1 = (unsigned char *) packet->getBuf();

						for (i = 0; i < packet->getDataLength() - 2; i++) {
							printf ("_0x%x",buf1[i]);
						}

						printf ("\n\n");
						//ArLog::log(ArLog::Normal,
						//	"%s::blockingConnect() packet received - command = %d length = %d", getNameWithBoard(), packet->getCommand(), packet->getDataLength() );
						delete packet;
						packet = NULL;

					} else {
						ArLog::log (ArLog::Normal,
						            "%s::blockingConnect() No packet received", getNameWithBoard());
					}
				}

#endif
				//sendStop();
				//packet = myReceiver->receivePacket (1000);

				// connect worked - return success

				myIsConnected = true;
				myTryingToConnect = false;

				ArLog::log (ArLog::Verbose, "%s::blockingConnect() Connection successful",
				            getNameWithBoard());

				myLastReading.setToNow();

				runAsync();

				return true;

			} else {
				ArLog::log (ArLog::Normal, "%s::blockingConnect() Wrong command recieved from Alive request = 0x%x - resending",
				            getNameWithBoard(), packet->getID());
				delete packet;
				packet = NULL;

				if (!sendAlive()) {
					ArLog::log (ArLog::Normal,
					            "%s::blockingConnect() Could not send Alive to Sonar", getNameWithBoard());
					failedToConnect();
					return false;
				}
			}

		} else {
			ArLog::log (ArLog::Normal, "%s::blockingConnect() Did not get response to Alive request (%d msec until timeout) - resending",
			            getNameWithBoard(), timeDone.mSecTo());


			if (!sendAlive()) {
				ArLog::log (ArLog::Normal,
				            "%s::blockingConnect() Could not send Alive to Sonar", getNameWithBoard());
				failedToConnect();
				return false;
			}
		}
	} while (timeDone.mSecTo() >= 0);

	ArLog::log (ArLog::Normal,
	            "%s::blockingConnect()  Connection to sonar failed",
	            getNameWithBoard());
	failedToConnect();
	return false;

}



AREXPORT bool ArSonarMTX::fakeConnect ()
{

	if (myConn == NULL) {
		ArLog::log (ArLog::Terse,
		            "%s: Could not connect because there is no connection defined",
		            getNameWithBoard());
		failedToConnect();
		return false;
	}

	ArSerialConnection *serConn = NULL;
	serConn = dynamic_cast<ArSerialConnection *> (myConn);

	if (myConn->getStatus() != ArDeviceConnection::STATUS_OPEN
	    && !myConn->openSimple()) {
		ArLog::log (
		  ArLog::Terse,
		  "%s: Could not connect because the connection was not open and could not open it",
		  getNameWithBoard());
		failedToConnect();
		return false;
	}


	myReceiver = new ArRobotPacketReceiver(myConn, true, HEADER1, HEADER2, 
																					myRecvTracking,
																					"ArSonarMTX");

	mySender = new ArRobotPacketSender(myConn, HEADER1, HEADER2, 
																					mySendTracking,
																					"ArSonarMTX");


	myNumTransducers = 8;

	myIsConnected = true;
	myTryingToConnect = false;

	// PS 2/28/14 - there is a problem with debug replay - this might do the trick
	myTransducersAreOn = true;

	ArLog::log (ArLog::Normal, "%s::fakeConnect() Connection successful",
				            getNameWithBoard());

	myLastReading.setToNow();

	runAsync();

	return true;

}

AREXPORT const char * ArSonarMTX::getName (void) const
{
	return myName.c_str();
}

AREXPORT const char * ArSonarMTX::getNameWithBoard (void) const
{
	return myNameWithBoard;
}

AREXPORT void * ArSonarMTX::runThread (void *arg)
{
	//char buf[1024];

	//ArSonarMTXPacket *packet;
	ArRobotPacket *packet = NULL;

  IFDEBUG(
    ArLog::log (ArLog::Normal,
		            "%s::runThread()", getNameWithBoard());
  )

  while (getRunning() )
  {

    IFDEBUG(printf("ArSonarMTX thread running=%d isConnected=%d\n", getRunning(), myIsConnected); fflush(stdout);)

	  while (getRunning() && myIsConnected &&
	         ((packet = myReceiver->receivePacket (400)) != NULL))
    {
		  myPacketsMutex.lock();
		  myPackets.push_back (packet);
		  myPacketsMutex.unlock();
		  if (myRobot == NULL) // if no robot, then sensorinterp() won't be called as robot cycle task callback, so call it directly
			  sensorInterp();
	  }
    
    if (packet == NULL) {
      IFDEBUG(puts("NULL packet from receiver."); fflush(stdout);)
      continue;
    }

	  // if we have a robot but it isn't running yet then don't have a
	  // connection failure
	  if (getRunning() && myIsConnected && checkLostConnection()) 
    {
		  // only disconnect if transducers are on - if they are off we'll get no packets
		  if (myTransducersAreOn) 
      {

			  ArLog::log (ArLog::Terse,
		              "%s::runThread()  Lost connection to the MTX sonar because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getNameWithBoard(),
		              myLastReading.mSecSince() / 1000.0,
		              getConnectionTimeoutSeconds() );
			  myIsConnected = false;
			  disconnectOnError();
			  continue;
		  }
	  }

  }

	ArLog::log (myInfoLogLevel,
		            "%s::runThread() thread killed (getRunning is false)", getNameWithBoard());

	return NULL;
}

/**
   This will check if the sonar has lost connection.  If there is no
   robot it is a straightforward check of last reading time against
   getConnectionTimeoutSeconds.  If there is a robot then it will not
   start the check until the sonar is running and connected.
**/
AREXPORT bool ArSonarMTX::checkLostConnection(void)
{
  //puts("checkLostConnection"); fflush(stdout);
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

AREXPORT void ArSonarMTX::disconnectOnError(void)
{
  ArLog::log(ArLog::Normal, "%s: Disconnected because of error", getNameWithBoard());
  myDisconnectOnErrorCBList.invoke();
}

AREXPORT bool ArSonarMTX::sendAlive()
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID(ALIVE);

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendAlive() sending alive to Sonar", getNameWithBoard());

	); // end IFDEBUG

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendAlive() Could not send alive request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendAlive() alive sent to Sonar", getNameWithBoard());

	); // end IFDEBUG


	return true;
}



AREXPORT bool ArSonarMTX::sendReset()
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID (RESET); // reset message

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendReset() Could not send reset request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendReset() sending reset to Sonar", getNameWithBoard());

	); // end IFDEBUG

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendReset() reset sent to Sonar", getNameWithBoard());

	); // end IFDEBUG


	return true;
}

AREXPORT bool ArSonarMTX::sendStart()
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID (START_SCAN); 

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendStart() Could not send start request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendStart() start sent to Sonar", getNameWithBoard());

	); // end IFDEBUG


	return true;
}


AREXPORT bool ArSonarMTX::sendStop()
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID (STOP_SCAN); 

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendStop() Could not send stop request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendStop() stop sent to Sonar", getNameWithBoard());

	); // end IFDEBUG

	return true;
}

AREXPORT bool ArSonarMTX::sendGetTransducerCount()
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID (GET_NUM_TRANDUCERS); 

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendGetTransducerCount() Could not send get trasnsducer count request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendGetTransducerCount() get number of transducers sent to Sonar", getNameWithBoard());

	); // end IFDEBUG

	return true;
}

AREXPORT bool ArSonarMTX::sendGetGain (unsigned char transducerNumber)
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID (GET_GAIN); 
	sendPacket.uByteToBuf (transducerNumber);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendGetGain() Could not send get gain request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendGetGain() get gain sent to Sonar 0x%x",
	              getNameWithBoard(), transducerNumber);

	); // end IFDEBUG


	return true;
}

AREXPORT bool ArSonarMTX::sendGetMaxRange (unsigned char transducerNumber)
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID (GET_ECHO_SAMPLE_SIZE); 
	sendPacket.uByteToBuf (transducerNumber);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendGetMaxRange() Could not send get max range (echosamplesize) to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendGetMaxRange() get maxrange (echosamplesize) sent to Sonar 0x%x",
	              getNameWithBoard(), transducerNumber);

	); // end IFDEBUG


	return true;
}


AREXPORT bool ArSonarMTX::sendGetDelay()
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID (GET_SONAR_DELAY); 

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendGetDelay() Could not send get delay request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendGetDelay() get delay sent to Sonar",
	              getNameWithBoard());

	); // end IFDEBUG


	return true;
}

AREXPORT bool ArSonarMTX::sendSetGain (unsigned char transducerNumber,
                                       unsigned char gain)
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID (SET_GAIN); 
	sendPacket.uByteToBuf (transducerNumber);
	sendPacket.uByteToBuf (gain);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendSetGain() Could not send set gain request to Sonar", getNameWithBoard());
		return false;
	}
	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendSetGain() set gain sent to Sonar 0x%x 0x%x",
	              getNameWithBoard(), transducerNumber, gain);

	); // end IFDEBUG
	return true;
}



AREXPORT bool ArSonarMTX::requestFirmwareVersion ()
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID (GET_VERSION); 

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::requestFirmwareVersion() Could not send get version request to Sonar", getNameWithBoard());
		return false;
	}
	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::requestFirmwareVersion() set get version to sonar",
	              getNameWithBoard());

	); // end IFDEBUG
	return true;
}


AREXPORT bool ArSonarMTX::sendSetMaxRange (unsigned char transducerNumber,
    int echoSampleSize)
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID (SET_ECHO_SAMPLE_SIZE); 
	sendPacket.uByteToBuf (transducerNumber);

	sendPacket.uByteToBuf (echoSampleSize & 0xff);
	sendPacket.uByteToBuf (echoSampleSize >> 8);

	if (!mySender->sendPacket(&sendPacket)) {

		ArLog::log (ArLog::Terse,
		            "%s::sendSetMaxRange() Could not send set MaxRange (echosamplesize) to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendSetMaxRange() set MaxRange (echosamplesize) sent to Sonar 0x%x 0x%x",
	              getNameWithBoard(), transducerNumber, echoSampleSize);

	); // end IFDEBUG

	return true;
}


AREXPORT bool ArSonarMTX::sendSetDelay (unsigned char delay)
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID (SET_SONAR_DELAY); // set delay
	sendPacket.uByteToBuf (delay);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendASetDelay() Could not send set delay request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendSetDelay() set delay sent to Sonar 0x%x",
	              getNameWithBoard(), delay);

	); // end IFDEBUG


	return true;
}

AREXPORT bool ArSonarMTX::sendSetMask (unsigned char maskLsb, unsigned char maskMsb)
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);
	sendPacket.setID (SET_TRANSDUCER_MASK); // set mask
	sendPacket.uByteToBuf (maskLsb);
	sendPacket.uByteToBuf (maskMsb);
	
	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendSetMask() Could not send set mask request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendSetMask() set mask sent to Sonar", getNameWithBoard());

	); // end IFDEBUG


	return true;
}

AREXPORT bool ArSonarMTX::validateTransducers()
{

	ArRobotPacket *packet;

	// send get transducer count
	bool gotTransducerCount = false;
	unsigned char *transBuf;

	for (int i = 0; i < 10; i++) {
		if (!sendGetTransducerCount()) {
			ArLog::log (ArLog::Normal,
			            "%s::validateTransducers() Could not send gettransducercount to Sonar", getNameWithBoard());
			return false;
		}

		packet = myReceiver->receivePacket (1000);

		if (packet == NULL) {
			ArLog::log (ArLog::Normal,
			            "%s::validateTransducers() No response to get transducer count", getNameWithBoard());
			continue;
		}

		transBuf = (unsigned char *) packet->getBuf();

		// verify get num trans received
		if ( (transBuf[3] != GET_NUM_TRANDUCERS) || (transBuf[4] == 0)) {
			ArLog::log (ArLog::Normal,
			            "%s::validateTransducers() Invalid response from sonar to gettransducercount (0x%x 0x%x)",
			            getNameWithBoard(), transBuf[3], transBuf[4]);
			continue;

		} else {
			gotTransducerCount = true;
			break;
		}
	} // endfor

	if (!gotTransducerCount) {
		ArLog::log (ArLog::Normal,
		            "%s::validateTransducers() Cannot get transducer count - exiting",
		            getNameWithBoard());
		return false;
	}

	gotTransducerCount = false;
	myNumTransducers = transBuf[4];
	ArLog::log (myInfoLogLevel,
	            "%s::validateTransducers() Sonar has %d transducers", getNameWithBoard(), myNumTransducers);

	if (myNumTransducers < myNumConfiguredTransducers) {
		ArLog::log (ArLog::Normal,
		            "%s::validateTransducers() there are more transducers configured %d then there are on the board %d",
		            getNameWithBoard(), myNumConfiguredTransducers, myNumTransducers);
		return false;
	}

	delete packet;
	return true;
}


AREXPORT bool ArSonarMTX::validateGain()
{
	ArRobotPacket *packet;
	// send get gain
	bool gotGain = false;
	unsigned char *gainBuf;

	for (int j = 0; j < myNumTransducers; j++) {

		// ignore any transducers that are not configured
		if (mySonarMap[j][SONAR_IS_CONFIGURED] == false) 
			continue;

		for (int i = 0; i < 10; i++) {
			if (!sendGetGain (j)) {
				ArLog::log (ArLog::Normal,
				            "%s::validateGain() Could not send get gain to Sonar", getNameWithBoard());
				return false;
			}

			packet = myReceiver->receivePacket (1000);
			
			if (packet == NULL) {
				ArLog::log (ArLog::Normal,
				            "%s::validateGain() No response to get gain - resending", getNameWithBoard());
				continue;
			}

			gainBuf = (unsigned char *) packet->getBuf();

			// verify get num trans received
			if ( (gainBuf[3] != GET_GAIN) || (gainBuf[4] != j)) {
				ArLog::log (ArLog::Normal,
				            "%s::validateGain() Invalid response from Sonar_%d to get gain (0x%x 0x%x)",
				            getNameWithBoard(), j+1, gainBuf[3], gainBuf[4]);
				continue;

			} else {
				gotGain = true;
				break;
			}
		} // endfor

		if (!gotGain) {
			ArLog::log (ArLog::Normal,
			            "%s::validateGain() Cannot get gain for Sonar_%d",
			            getNameWithBoard(), j+1);
			return false;
		}

		gotGain = false;

		unsigned char gain = gainBuf[5];
		IFDEBUG (
		  ArLog::log (ArLog::Normal,
		              "%s::validateGain() Sonar_%d has gain of %d", getNameWithBoard(), j+1, gain));

		if (mySonarMap[j][SONAR_GAIN] != gain) {
			ArLog::log (ArLog::Verbose,
			            "%s::validateGain() Sonar_%d gain %d does not match configured gain %d, setting new gain",
			            getNameWithBoard(), j+1, gain, mySonarMap[j][SONAR_GAIN]);

			if (!sendSetGain (j, mySonarMap[j][SONAR_GAIN])) {
				ArLog::log (ArLog::Normal,
				            "%s::validateGain() Could not send set gain to Sonar_%d", getNameWithBoard(), j+1);
				return false;
			}
		}

		delete packet;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::validateGain() Gain valid", getNameWithBoard()));
	return true;
}

AREXPORT bool ArSonarMTX::validateDelay()
{
	ArRobotPacket *packet;
	// send get delay
	bool gotDelay = false;
	unsigned char *delayBuf;

	for (int i = 0; i < 10; i++) {
		if (!sendGetDelay ()) {
			ArLog::log (ArLog::Normal,
			            "%s::validateDelay() Could not send get delay to Sonar", getNameWithBoard());
			return false;
		}

		packet = myReceiver->receivePacket (1000);

		if (packet == NULL) {
			ArLog::log (ArLog::Normal,
			            "%s::validateDelay() No response to get delay - resending (%d)",
			            getNameWithBoard(), i);
			continue;
		}

		delayBuf = (unsigned char *) packet->getBuf();

		// verify get num trans received
		if ( (delayBuf[3] != GET_SONAR_DELAY) || (delayBuf[4] == 0)) {
			ArLog::log (ArLog::Normal,
			            "%s::validateDelay() Invalid response from sonar to get delay (0x%x 0x%x)",
			            getNameWithBoard(), delayBuf[3], delayBuf[4]);
			continue;

		} else {
			gotDelay = true;
			break;
		}
	} // endfor

	if (!gotDelay) {
		ArLog::log (ArLog::Normal,
		            "%s::validateDelay() Cannot get delay - exiting",
		            getNameWithBoard());
		return false;
	}

	gotDelay = false;

	unsigned char delay = delayBuf[4];
	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::validateDelay() Sonar has delay of %d", getNameWithBoard(), delay));

	if (myBoardDelay != delay) {
		ArLog::log (ArLog::Verbose,
		            "%s::validateDelay() delay %d does not match configured delay %d, setting new delay",
		            getNameWithBoard(), delay, myBoardDelay);

		if (!sendSetDelay (myBoardDelay)) {
			ArLog::log (ArLog::Normal,
			            "%s::validateDelay() Could not send set delay to Sonar", getNameWithBoard());
			return false;
		}
	}

	delete packet;
	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::validateDelay() Delay valid", getNameWithBoard()));
	return true;
}

AREXPORT bool ArSonarMTX::validateNumThresholdRanges()
{
	ArRobotPacket *packet;
	bool gotNumThres = false;
	unsigned char *numBuf;

	for (int i = 0; i < 10; i++) {
		if (!sendGetNumThresholdRanges ()) {
			ArLog::log (ArLog::Normal,
			            "%s::validateNumThresholdRanges() Could not send get num threshold ranges to Sonar", getNameWithBoard());
			return false;
		}

		packet = myReceiver->receivePacket (1000);

		if (packet == NULL) {
			ArLog::log (ArLog::Normal,
			            "%s::validateNumThresholdRanges() No response to get num threshold ranges - resending (%d)",
			            getNameWithBoard(), i);
			continue;
		}

		numBuf = (unsigned char *) packet->getBuf();
		gotNumThres = true;
		break;
	}

	if (!gotNumThres) {
		ArLog::log (ArLog::Normal,
		            "%s::validateNumThresholdRanges() Cannot get num threshold ranges - exiting",
		            getNameWithBoard());
		return false;
	}

	unsigned char numThres = numBuf[4];

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::validateNumThresholdRanges() Sonar has num of threshold ranges of %d", getNameWithBoard(), numThres));

	delete packet;
	return true;
}


AREXPORT bool ArSonarMTX::queryFirmwareVersion()
{
	ArRobotPacket *packet;
	// send get delay
	bool gotVersion = false;
	unsigned char *versionBuf;

	for (int i = 0; i < 10; i++) {
		if (!requestFirmwareVersion ()) {
			ArLog::log (ArLog::Normal,
			            "%s::queryFirmwareVersion() Could not send get version to Sonar", getNameWithBoard());
			return false;
		}

		for (int z = 0; z < 10; z++) { 
			packet = myReceiver->receivePacket (1000);		

			if (packet == NULL) {
				ArLog::log (ArLog::Normal,
			            "%s::queryFirmwareVersion() No response to get version - resending (%d)",
			            getNameWithBoard(), i);
				continue;
			}

			versionBuf = (unsigned char *) packet->getBuf();

			if ( (versionBuf[3] != GET_VERSION) || (versionBuf[4] == 0)) {
				ArLog::log (ArLog::Normal,
			            "%s::queryFirmwareVersion() Invalid response from sonar to get version (0x%x 0x%x)",
			            getNameWithBoard(), versionBuf[3], versionBuf[4]);
				continue;

			} else {
				gotVersion = true;
				break;
			}
		}
		if (gotVersion)
			break;
	} // endfor

	if (!gotVersion) {
		ArLog::log (ArLog::Normal,
		            "%s::queryFirmwareVersion() Cannot get version - exiting",
		            getNameWithBoard());
		return false;
	}

	myFirmwareVersion = versionBuf[4];
	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::queryFirmwareVersion() Sonar has firmware version of %d", getNameWithBoard(), myFirmwareVersion));

	delete packet;
	return true;
}



AREXPORT bool ArSonarMTX::validateMaxRange()
{
	ArRobotPacket *packet;

	// send get echosamplesize
	bool gotEchoSampleSize = false;
	unsigned char *echosamplesizeBuf;

	for (int j = 0; j < myNumTransducers; j++) {

		// ignore any transducers that are not configured
		if (mySonarMap[j][SONAR_IS_CONFIGURED] == false) 
			continue;

		for (int i = 0; i < 10; i++) {
			if (!sendGetMaxRange (j)) {
				ArLog::log (ArLog::Normal,
				            "%s::validateMaxRange() Could not send get maxrange echosamplesize to Sonar_%d",
				            getNameWithBoard(), j+1);
				return false;
			}

			packet = myReceiver->receivePacket (1000);

			if (packet == NULL) {
				ArLog::log (ArLog::Normal,
				            "%s::validateMaxRange() No response to get maxrange - echosamplesize - resending", getNameWithBoard());
				continue;
			}

			echosamplesizeBuf = (unsigned char *) packet->getBuf();

			// verify get num max range received
			if ( (echosamplesizeBuf[3] != GET_ECHO_SAMPLE_SIZE) || (echosamplesizeBuf[4] != j)) {
				ArLog::log (ArLog::Normal,
				            "%s::validateMaxRange() Invalid response from Sonar_%d to get maxange echosamplesize (0x%x 0x%x)",
				            getNameWithBoard(), j+1, echosamplesizeBuf[3], echosamplesizeBuf[4]);
				delete packet;
				continue;

			} else {
				gotEchoSampleSize = true;
				break;
			}
			
			delete packet;

		} // endfor

		if (!gotEchoSampleSize) {
			ArLog::log (ArLog::Normal,
			            "%s::validateMaxRange() Cannot get maxrange echosamplesize from Sonar_%d",
			            getNameWithBoard(), j+1);
			return false;
		}
		gotEchoSampleSize = false;
		//unsigned char echosamplesize = echosamplesizeBuf[4];

		int echoSampleSize = (echosamplesizeBuf[6] << 8) | (echosamplesizeBuf[5]);

		IFDEBUG (
		  ArLog::log (ArLog::Normal,
		              "%s::validateMaxRange() Sonar_%d has maxrange echosamplesize of %d", getNameWithBoard(), j+1, echoSampleSize));

		if (mySonarMap[j][SONAR_MAX_RANGE] != echoSampleSize) {
			ArLog::log (ArLog::Verbose,
			            "%s::validateMaxRange() Sonar_%d maxrange echosamplesize %d does not match configured maxrange echosamplesize %d, setting new maxrange echosamplesize",
			            getNameWithBoard(), j+1, echoSampleSize, mySonarMap[j][SONAR_MAX_RANGE]);

			if (!sendSetMaxRange (j, mySonarMap[j][SONAR_MAX_RANGE])) {
				ArLog::log (ArLog::Normal,
				            "%s::validateMaxRange() Could not send set maxrange echosamplesize to Sonar_%d", getNameWithBoard(), j+1);
				return false;
			}
		}

		delete packet;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::validateMaxRange() EchoSampleSize valid", getNameWithBoard()));
	return true;
}


AREXPORT bool ArSonarMTX::validateThresholds()
{
	ArRobotPacket *packet;

	// send get threshold
	bool gotThresholds = false;
	unsigned char *thresholdBuf;

	for (int j = 0; j < myNumTransducers; j++) {
//	for (int j = 1; j < 2; j++) {

		// ignore any transducers that are not configured
		if (mySonarMap[j][SONAR_IS_CONFIGURED] == false) 
			continue;

		for (int i = 0; i < 10; i++) {
			if (!sendGetThresholds (j)) {
				ArLog::log (ArLog::Normal,
				            "%s::validateThresholds() Could not send get threshold to Sonar_%d",
				            getNameWithBoard(), j+1);
				return false;
			}

			packet = myReceiver->receivePacket (1000);

			if (packet == NULL) {
				ArLog::log (ArLog::Normal,
				            "%s::validateThresholds() No response to get threshold - resending", getNameWithBoard());
				continue;
			}

			thresholdBuf = (unsigned char *) packet->getBuf();

			// verify get
			if ( (thresholdBuf[3] != GET_THRESHOLDS) || (thresholdBuf[4] != j)) {
				ArLog::log (ArLog::Normal,
				            "%s::validateThresholds() Invalid response from Sonar_%d to get threshold (0x%x 0x%x)",
				            getNameWithBoard(), j+1, thresholdBuf[3], thresholdBuf[4]);
				delete packet;
				continue;

			} else {
				gotThresholds = true;
				break;
			}
		} // endfor

		if (!gotThresholds) {
			ArLog::log (ArLog::Normal,
			            "%s::validateThresholds() Cannot get threshold from Sonar_%d",
			            getNameWithBoard(), j+1);
			return false;
		}

		gotThresholds = false;

		int thres = (thresholdBuf[6] << 8) | (thresholdBuf[5]);

		IFDEBUG (
		  ArLog::log (ArLog::Normal,
		              "%s::validateThresholds() Sonar_%d has threshold of %d", getNameWithBoard(), j+1, thres));

		if (mySonarMap[j][SONAR_DETECTION_THRES] != thres) {
			ArLog::log (ArLog::Verbose,
			            "%s::validateThresholds() Sonar_%d has detection threshold %d, it does not match configured threshold %d, setting new threshold",
			            getNameWithBoard(), j+1, thres,
			            mySonarMap[j][SONAR_DETECTION_THRES]);

			if (!sendSetThresholds (j, mySonarMap[j][SONAR_DETECTION_THRES])) {
				ArLog::log (ArLog::Normal,
				            "%s::validateThresholds() Could not send set threshold to Sonar_%d", getNameWithBoard(), j+1);
				return false;
			}
		}
		delete packet;

	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::validateThresholds() Thresholds valid", getNameWithBoard()));
	return true;
}

AREXPORT bool ArSonarMTX::sendGetThresholds (unsigned char transducerNumber)
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (GET_THRESHOLDS); // get thresholds
	sendPacket.uByteToBuf (transducerNumber);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendGetThresholds() Could not send get thresholds request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendGetThresholds() get thresholds sent to Sonar 0x%x",
	              getNameWithBoard(), transducerNumber);

	); // end IFDEBUG


	return true;
}

AREXPORT bool ArSonarMTX::sendGetMask ()
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (GET_TRANSDUCER_MASK); // get transducer mask

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendGetMask() Could not send get transducer mask request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendGetMask() get transducer mask sent to Sonar",
	              getNameWithBoard());

	); // end IFDEBUG


	return true;
}

AREXPORT bool ArSonarMTX::sendGetNumThresholdRanges ()
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (NUM_THRESHOLD_RANGES);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendGetNumThresholdRanges() Could not send get number threshold ranges request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendGetNumThresholdRanges() get number threshold ranges sent to Sonar",
	              getNameWithBoard());

	); // end IFDEBUG


	return true;
}

AREXPORT bool ArSonarMTX::sendSetThresholds (unsigned char transducerNumber,
    int thres)
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (SET_THRESHOLDS); // set thresholds
	sendPacket.uByteToBuf (transducerNumber);
	sendPacket.uByteToBuf (thres & 0xff);
	sendPacket.uByteToBuf (thres >> 8);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendSetThresholds() Could not send set thresholds request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendSetThresholds() set thresholds sent to Sonar 0x%x 0x%x",
	              getNameWithBoard(), transducerNumber, thres);

	); // end IFDEBUG

	return true;
}
/*
AREXPORT bool ArSonarMTX::validateNoiseDelta()
{
	ArRobotPacket *packet;

	// send get noiseDelta
	bool gotNoiseDelta = false;
	unsigned char *noiseDeltaBuf;

	for (int j = 0; j < myNumTransducers; j++) {

		// ignore any transducers that are not configured
		if (mySonarMap[j][SONAR_IS_CONFIGURED] == false) 
			continue;

		for (int i = 0; i < 10; i++) {
			if (!sendGetNoiseDelta (j)) {
				ArLog::log (ArLog::Normal,
				            "%s::validateNoiseDelta() Could not send get noiseDelta to Sonar_%d",
				            getNameWithBoard(), j+1);
				return false;
			}

			packet = myReceiver->receivePacket (1000);

			if (packet == NULL) {
				ArLog::log (ArLog::Normal,
				            "%s::validateNoiseDelta() No response to get noiseDelta - resending", getNameWithBoard());
				continue;
			}

			noiseDeltaBuf = (unsigned char *) packet->getBuf();

			// verify get
			if ( (noiseDeltaBuf[3] != GET_NOISE_DELTA) || (noiseDeltaBuf[4] != j)) {
				ArLog::log (ArLog::Normal,
				            "%s::validateNoiseDelta() Invalid response from Sonar_%d to get noiseDelta (0x%x 0x%x)",
				            getNameWithBoard(), j+1, noiseDeltaBuf[3], noiseDeltaBuf[4]);
				delete packet;
				continue;

			} else {
				gotNoiseDelta = true;
				break;
			}
		} // endfor

		if (!gotNoiseDelta) {
			ArLog::log (ArLog::Normal,
			            "%s::validateNoiseDelta() Cannot get noiseDelta from Sonar_%d",
			            getNameWithBoard(), j+1);
			return false;
		}

		gotNoiseDelta = false;
		int noiseDelta = (noiseDeltaBuf[6] << 8) | (noiseDeltaBuf[5]);

		IFDEBUG (
		  ArLog::log (ArLog::Normal,
		              "%s::validateNoiseDelta() Sonar_%d has noiseDelta of %d", getNameWithBoard(), j+1, noiseDelta));

		if (mySonarMap[j][SONAR_NOISE_DELTA] != noiseDelta) {
			ArLog::log (ArLog::Normal,
			            "%s::validateNoiseDelta() Sonar_%d has detection noiseDelta %d, it does not match configured noiseDelta %d, setting new noiseDelta",
			            getNameWithBoard(), j+1, noiseDelta,
			            mySonarMap[j][SONAR_NOISE_DELTA]);

			if (!sendSetNoiseDelta (j, mySonarMap[j][SONAR_NOISE_DELTA])) {
				ArLog::log (ArLog::Normal,
				            "%s::validateNoiseDelta() Could not send set noiseDelta to Sonar_%d", getNameWithBoard(), j+1);
				return false;
			}
		}

		delete packet;
	}

	IFDEBUG (
	  ArLog::log (ArLog::Normal,
	              "%s::validateNoiseDelta() NoiseDelta valid", getNameWithBoard()));
	return true;
}

AREXPORT bool ArSonarMTX::sendGetNoiseDelta (unsigned char transducerNumber)
{
	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (GET_NOISE_DELTA); // get noiseDelta
	sendPacket.uByteToBuf (transducerNumber);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendNoiseDelta() Could not send get noisedelta request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendGetNoiseDelta() get noisedelta sent to Sonar 0x%x",
	              getNameWithBoard(), transducerNumber);

	); // end IFDEBUG


	return true;
}

AREXPORT bool ArSonarMTX::sendSetNoiseDelta (unsigned char transducerNumber,
    int noiseDelta)
{

	ArRobotPacket sendPacket(HEADER1, HEADER2);

	sendPacket.setID (SET_NOISE_DELTA); // set noiseDelta
	sendPacket.uByteToBuf (transducerNumber);
	sendPacket.uByteToBuf (noiseDelta & 0xff);
	sendPacket.uByteToBuf (noiseDelta >> 8);

	if (!mySender->sendPacket(&sendPacket)) {
		ArLog::log (ArLog::Terse,
		            "%s::sendSetNoiseDelta() Could not send set noise delta request to Sonar", getNameWithBoard());
		return false;
	}

	IFDEBUG (

	  ArLog::log (ArLog::Normal,
	              "%s::sendSetNoiseDelta() set noise delta sent to Sonar 0x%x 0x%x",
	              getNameWithBoard(), transducerNumber, noiseDelta);

	); // end IFDEBUG

	return true;
}
*/

bool ArSonarMTX::turnOnTransducers()
{


	if (sendSetMask(myTransducerMaskLSB, myTransducerMaskMSB)) {
		ArLog::log (ArLog::Normal,
	              "%s::turnOnTransducers() turning ON transducers",
	              getNameWithBoard());
		myTransducersAreOn = true;
		myLastReading.setToNow();
		return true;

	}
	else {
		ArLog::log (ArLog::Normal,
	              "%s::turnOnTransducers() failed turning ON transducers",
	              getNameWithBoard());
		return false;

	}

}

bool ArSonarMTX::turnOffTransducers()
{

	if (sendSetMask(0, 0)) {
		ArLog::log (ArLog::Normal,
	              "%s::turnOffTransducers() turning OFF transducers",
	              getNameWithBoard());
		myTransducersAreOn = false;
		return true;

	}
	else {
		ArLog::log (ArLog::Normal,
	              "%s::turnOnTransducers() failed turning OFF transducers",
	              getNameWithBoard());
		return false;

	}

}

AREXPORT bool ArSonarMTX::disableForAutonomousDriving()
{



	if (sendSetMask(myAutonomousDrivingTransducerMaskLSB, myAutonomousDrivingTransducerMaskMSB)) {
		ArLog::log (ArLog::Normal,
	              "%s::disableForAutonomousDriving() turning OFF all non autonomous driving transducers",
	              getNameWithBoard());
		myTransducersAreOn = false;
		return true;

	}
	else {
		ArLog::log (ArLog::Normal,
	              "%s::disableForAutonomousDriving() failed turning OFF non autonomous driving transducers",
	              getNameWithBoard());
		return false;

	}


}

