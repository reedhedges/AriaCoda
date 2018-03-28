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
#include "ArSZSeries.h"
#include "ArRobot.h"
#include "ArSerialConnection.h"
#include "ariaInternal.h"
#include <time.h>

//#define TRACE
#if (defined(TRACE))
  #define IFDEBUG(code) {code;}
#else
  #define IFDEBUG(code)
#endif

AREXPORT ArSZSeriesPacket::ArSZSeriesPacket() :
ArBasePacket(10000, 1, NULL, 1) {

}

AREXPORT ArSZSeriesPacket::~ArSZSeriesPacket() {

}

AREXPORT ArTime ArSZSeriesPacket::getTimeReceived(void) {
	return myTimeReceived;
}

AREXPORT void ArSZSeriesPacket::setTimeReceived(ArTime timeReceived) {
	myTimeReceived = timeReceived;
}

AREXPORT void ArSZSeriesPacket::duplicatePacket(ArSZSeriesPacket *packet) {
	myLength = packet->getLength();
	myReadLength = packet->getReadLength();
	myTimeReceived = packet->getTimeReceived();
	myDataLength = packet->myDataLength;
	myNumReadings = packet->myNumReadings;
    myScanFrequency = packet->myScanFrequency;
	myCrcByte1 = packet->myCrcByte1;
	myCrcByte2 = packet->myCrcByte2;

	memcpy(myBuf, packet->getBuf(), myLength);
}

AREXPORT void ArSZSeriesPacket::empty(void) {
	myLength = 0;
	myReadLength = 0;
}

#if 0
AREXPORT void ArSZSeriesPacket::uByteToBuf(ArTypes::UByte val)
{
	char buf[1024];
	sprintf(buf, "%u", val);
	strToBuf(buf);
}
#endif

AREXPORT void ArSZSeriesPacket::byteToBuf(ArTypes::Byte val)
{
	char buf[1024];
	if (val > 0)
		sprintf(buf, "+%d", val);
	else
		sprintf(buf, "%d", val);
	strToBuf(buf);
}


AREXPORT ArTypes::Byte ArSZSeriesPacket::bufToByte(void)
{
	ArTypes::Byte ret=0;


	if (!isNextGood(1))
		return 0;

	if (myBuf[myReadLength] == ' ')
		myReadLength++;

	if (!isNextGood(4))
		return 0;

	unsigned char n1, n2;
	n2 = deascii(myBuf[myReadLength+6]);
	n1 = deascii(myBuf[myReadLength+7]);
	ret = n2 << 4 | n1;

	myReadLength += 4;

	return ret;
}

int ArSZSeriesPacket::deascii(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return 10 + c - 'a';
	else if (c >= 'A' && c <= 'F')
		return 10 + c - 'a';
	else
		return 0;
}

AREXPORT ArSZSeriesPacketReceiver::ArSZSeriesPacketReceiver() {

}

AREXPORT ArSZSeriesPacketReceiver::~ArSZSeriesPacketReceiver() {

}

AREXPORT void ArSZSeriesPacketReceiver::setDeviceConnection(
		ArDeviceConnection *conn) {
	myConn = conn;
}

AREXPORT ArDeviceConnection *ArSZSeriesPacketReceiver::getDeviceConnection(void) {
	return myConn;
}

ArSZSeriesPacket *ArSZSeriesPacketReceiver::receivePacket(unsigned int msWait,
		bool startMode) {

	ArSZSeriesPacket *packet;
	unsigned char c;
	long timeToRunFor;
	ArTime timeDone;
	ArTime lastDataRead;
	ArTime packetReceived;
	int i;

	if (myConn == NULL || myConn->getStatus()
			!= ArDeviceConnection::STATUS_OPEN) {
		return NULL;
	}

	timeDone.setToNow();
	if (!timeDone.addMSec(msWait)) {
		ArLog::log(ArLog::Terse, "%s::receivePacket() error adding msecs (%i)",
				myName, msWait);
	}
	//msWait = 100;

	do {
		timeToRunFor = timeDone.mSecTo();
		if (timeToRunFor < 0)
			timeToRunFor = 0;
		/*
		 ArLog::log(ArLog::Terse,
		 "%s::receivePacket() timeToRunFor = %d",
		 myName, timeToRunFor);
		 */

		myPacket.empty();
		myPacket.setLength(0);
		myReadCount = 0;

		unsigned char firstbytelen;
		unsigned char secondbytelen;
		unsigned char temp[4];

		unsigned char crcbuf[10000];
		memset(crcbuf, 0, 10000);
		memset(myReadBuf, 0, 100000);
		int n = 0;
#if 0
		bool nonzero = true;
		int zerocnt = 0;
		char prev_c = 0x30;
		while (nonzero)
		{
			if ((myConn->read((char *) &c, 1, msWait)) > 0)
			{
				if (((c == 0x00) && (zerocnt == 0)) || ((c == 0x00) && (prev_c == 0x00)))
				{
					zerocnt++;
					prev_c = c;
					if (zerocnt == 4)
			          nonzero = false;
				}
				else
				{
					zerocnt = 0;
					prev_c = 0x30;
				}
		    }
		} // endwhile

		//printf("WE FOUND A 4 ZERO's\n");
		packetReceived = myConn->getTimeRead(0);
		myPacket.setTimeReceived(packetReceived);
#endif
//#if 0
		// look for initial sequence 0x00 0x00 0x00 0x00
		for (i = 0; i < 4; i++) {
			if ((myConn->read((char *) &c, 1, msWait)) > 0) {
				if (c != 0x00) {
					//printf("char = %x\n",c);
					//ArLog::log(ArLog::Terse,
					 //                "ArSZSeries::receivePacket() error reading first 4 bytes of header");
					break;
				}
				if (i == 0) {
					packetReceived = myConn->getTimeRead(0);
					//ArTime previousTime = myPacket.getTimeReceived();
					myPacket.setTimeReceived(packetReceived);
					//ArLog::log(ArLog::Normal,
					//		"ms since = %d",
					//		packetReceived.mSecSince(previousTime));
				}
			} else {
				/* don't log this if we are in starting mode, means laser is not connecting */
				if (startMode)
					ArLog::log(ArLog::Terse,
							"%s::receivePacket() myConn->read error (header)",
							myName);
				return NULL;
			}
		} // end for

		if (c != 0x00)
			continue;

//#endif

		// next byte = 0x91  - command number
		if ((myConn->read((char *) &c, 1, msWait)) > 0) {
			if (c != 0x91) {
				//ArLog::log(ArLog::Terse,
				//                 "ArSZSeries::receivePacket() error data block number in header");
				break;
			}
		} else {
			ArLog::log(
					ArLog::Terse,
					"%s::receivePacket() myConn->read error (data block number)",
					myName);
			return NULL;
		}

		if (c != 0x91)
		{
			ArLog::log(ArLog::Terse,
			"%s::receivePacket() Invalid command ID (must be 0x91) = 0x%x",
			myName, c);
			continue;
		}

		for (n=0; n < 4; n++ )
		   crcbuf[n] = 0;
		crcbuf[n++] = 0x91;

		// next byte = 0x00  - Communication ID
		// note we are assuming this needs to be 0
		// as set in the Configurator
		if ((myConn->read((char *) &c, 1, msWait)) > 0) {
			if (c != 0x00) {
				//ArLog::log(ArLog::Terse,
				//                 "ArSZSeries::receivePacket() error data block number in header");
				break;
			}
		} else {
			ArLog::log(
					ArLog::Terse,
					"%s::receivePacket() myConn->read error (data block number)",
					myName);
			return NULL;
		}


		if (c != 0x00)
		{
			ArLog::log(ArLog::Terse,
			"%s::receivePacket() Communication ID error = %d",
			myName, c);
			continue;
		}

		crcbuf[n++] = 0;

		// next 2 bytes are length,

		for (i = 0; i < 2; i++) {
			if ((myConn->read((char *) &c, 1, msWait)) > 0) {
				temp[i] = c;
				crcbuf[n++] = c;
			} else {
				ArLog::log(ArLog::Terse,
						"%s::receivePacket() myConn->read error (length)",
						myName);
				return NULL;
			}
		} // end for

		firstbytelen = temp[0];
		secondbytelen = temp[1];
        //crcbuf[n++] = temp[0];
        //crcbuf[n++] = temp[1];

		// do we need to validate byte length = 1505
		int datalen = secondbytelen | (firstbytelen << 8);

		// data length is 1 less as it includes the scan freq byte
		myPacket.setDataLength(datalen - 1);

		/*
		ArLog::Terse(
		"%s::receivePacket() Data Length = %d", myName, myPacket.getDataLength());
		*/

		myPacket.setNumReadings(myPacket.getDataLength() / 2);

		/*
		ArLog::Terse(
		"%s::receivePacket() Number of readings = %d", myName, myPacket.getNumReadings());
		*/

		// next scan frequency

		if ((myConn->read((char *) &c, 1, msWait)) > 0) {
			myPacket.setScanFrequency(c);
		} else {
			ArLog::log(
					ArLog::Terse,
					"%s::receivePacket() myConn->read error (data block number)",
					myName);
			return NULL;
		}

	     crcbuf[n++] = c;

		// now read all the readings
		// PS 12/6/12 - change timeout from 5000 to 200
		int numRead = myConn->read((char *) &myReadBuf[0],
				myPacket.getDataLength(), 200);

		// trap if we failed the read
		if (numRead < 0) {
			ArLog::log(ArLog::Terse, "%s::receivePacket() Failed read (%d)",
					myName, numRead);
			return NULL;
		}

		/*
		ArLog::log(ArLog::Terse,
				"%s::receivePacket() Number of bytes read = %d, asked for = %d", myName,
				numRead, myPacket.getDataLength());
		*/

		// finally get the crc
		for (i = 0; i < 2; i++)
		{
			if ((myConn->read((char *) &c, 1, msWait)) > 0)
			{
				temp[i] = c;
			}
			else
			{
				ArLog::log(ArLog::Terse,
						"%s::receivePacket() myConn->read error (crc)", myName);
				return NULL;
			}
		} // end for
		myPacket.setCrcByte1(temp[0]);
		myPacket.setCrcByte2(temp[1]);

#if 0
		// Matt put this check in to look for 0's in the data - not sure why
		int numZeros = 0;
		for (i = 5; i < myPacket.getDataLength(); i++) {
			if (myReadBuf[i] == 0)
				numZeros++;
			else
				numZeros = 0;

			if (numZeros >= 4) {
				ArLog::log(
						ArLog::Terse,
						"%s::receivePacket() myConn->read error (got 4 zeros in data)",
						myName);
				return NULL;
			}
		}
#endif

		memcpy(&crcbuf[n], &myReadBuf[0], myPacket.getDataLength());

		/*code to trace
		char buf[100000];
		int x = 0;
		
		int t = myPacket.getDataLength() + n + 1;

		ArLog::log(ArLog::Normal,
		           "%s::receivePacket() DATA LEN  = %d  CRC byte1 = %x CRC byte2 = %x", myName, t, temp[0], temp[1]);
		int y = n;
		for (y = 0;y < t; y++) {
			sprintf(&buf[x++], "%02x", (char *)crcbuf[y] );
			
		}
			
		sprintf(&buf[x++], "%02x", (char *)temp[0]);
		
		sprintf(&buf[x], "%02x", (char *)temp[1]);
		
		
		ArLog::log(ArLog::Normal,
		           "%s::receivePacket() packet = %s", myName, buf);
			*/
		
		
		// now go validate the crc

		unsigned short crc = CRC16(crcbuf, myPacket.getDataLength() + n);

		unsigned short incrc = (temp[0] << 8) | temp[1];

		
		if (myPrevCrc == crc) {
	
			ArLog::log(ArLog::Verbose,
					"CRC MATCH, current scan freq = %d, prev scan freq = %d",
					myPacket.getScanFrequency(),
					myPacket.getPrevScanFrequency());
					
			myPrevCrc = crc;
			myPacket.setPrevScanFrequency(myPacket.getScanFrequency());
			return NULL;
		}	

		myPacket.setPrevScanFrequency(myPacket.getScanFrequency());
		myPrevCrc = crc;
		
		if (incrc != crc)
		{
			ArLog::log(ArLog::Terse,
					"%s::receivePacket() CRC error (in = 0x%02x calculated = 0x%02x) ",
					myName, incrc, crc);
			return NULL;
		}


		myPacket.dataToBuf(&myReadBuf[0], myPacket.getNumReadings() * 2);
		myPacket.resetRead();
		packet = new ArSZSeriesPacket;
		packet->duplicatePacket(&myPacket);

		/*
		ArLog::log(ArLog::Normal,
		           "%s::receivePacket() returning packet %d %d", myName, packet->getNumReadings(), myPacket.getNumReadings());
		*/

		myPacket.empty();

		return packet;

	} while (timeDone.mSecTo() >= 0); // || !myStarting)

	ArLog::log(ArLog::Terse, "%s::receivePacket() Timeout on read", myName);
	return NULL;
}

AREXPORT ArSZSeries::ArSZSeries(int laserNumber, const char *name) :
			ArLaser(laserNumber, name, 16382),
			mySensorInterpTask(this, &ArSZSeries::sensorInterp),
			myAriaExitCB(this, &ArSZSeries::disconnect) {

	//ArLog::log(ArLog::Normal, "%s: Sucessfully created", getName());

	clear();
	myRawReadings = new std::list<ArSensorReading *>;

	Aria::addExitCallback(&myAriaExitCB, -10);

	setInfoLogLevel(ArLog::Normal);
	//setInfoLogLevel(ArLog::Terse);

	laserSetName( getName());

	laserAllowSetPowerControlled(false);

	laserSetDefaultPortType("serial422");

	std::list < std::string > baudChoices;

	baudChoices.push_back("9600");
	baudChoices.push_back("19200");
	baudChoices.push_back("38400");
	baudChoices.push_back("57600");
	baudChoices.push_back("115200");
	//baudChoices.push_back("125000");
	baudChoices.push_back("230400");
	baudChoices.push_back("460800");

	//laserAllowStartingBaudChoices("9600", baudChoices);
	laserAllowStartingBaudChoices("38400", baudChoices);

    // PS 9/9/11 - don't allow auto baud for his laser
	//laserAllowAutoBaudChoices("57600", baudChoices);

  laserAllowSetDegrees(-135, -135, -135, 135, 135, 135);
  laserAllowSetIncrement(0.5, 0.5, 0.5);

	//myLogLevel = ArLog::Verbose;
	//myLogLevel = ArLog::Terse;
	myLogLevel = ArLog::Normal;

	setMinDistBetweenCurrent(0);
	setMaxDistToKeepCumulative(4000);
	setMinDistBetweenCumulative(200);
	setMaxSecondsToKeepCumulative(30);
	setMaxInsertDistCumulative(3000);

	setCumulativeCleanDist(75);
	setCumulativeCleanInterval(1000);
	setCumulativeCleanOffset(600);

	resetLastCumulativeCleanTime();

	setCurrentDrawingData(
			//new ArDrawingData("polyDots", ArColor(255, 204, 153), 75, // mm diameter of dots
					new ArDrawingData("polyDots", ArColor(255,102,0), 75, // mm diameter of dots
					76), // layer above sonar
					true);

	setCumulativeDrawingData(
			new ArDrawingData("polyDots", ArColor(255,153,0), 95, // mm diameter of dots
					61), // layer below current range devices
					true);

}

AREXPORT ArSZSeries::~ArSZSeries() {
	Aria::remExitCallback(&myAriaExitCB);
	if (myRobot != NULL) {
		myRobot->remRangeDevice(this);
		myRobot->remLaser(this);
		myRobot->remSensorInterpTask(&mySensorInterpTask);
	}
	if (myRawReadings != NULL) {
		ArUtil::deleteSet(myRawReadings->begin(), myRawReadings->end());
		myRawReadings->clear();
		delete myRawReadings;
		myRawReadings = NULL;
	}
	lockDevice();
	if (isConnected())
		disconnect();
	unlockDevice();
}

void ArSZSeries::clear(void) {
	myIsConnected = false;
	myTryingToConnect = false;
	myStartConnect = false;

	myNumChans = 0;
}

AREXPORT void ArSZSeries::laserSetName(const char *name) {
	myName = name;

	myConnMutex.setLogNameVar("%s::myConnMutex", getName());
	myPacketsMutex.setLogNameVar("%s::myPacketsMutex", getName());
	myDataMutex.setLogNameVar("%s::myDataMutex", getName());
	myAriaExitCB.setNameVar("%s::exitCallback", getName());

	ArLaser::laserSetName( getName());
}

AREXPORT void ArSZSeries::setRobot(ArRobot *robot) {
	myRobot = robot;

	if (myRobot != NULL) {
		myRobot->remSensorInterpTask(&mySensorInterpTask);
		myRobot->addSensorInterpTask("SZSeries", 90, &mySensorInterpTask);
	}
	ArLaser::setRobot(robot);
}

AREXPORT bool ArSZSeries::asyncConnect(void) {
	myStartConnect = true;
	if (!getRunning())
		runAsync();
	return true;
}

AREXPORT bool ArSZSeries::disconnect(void) {
	if (!isConnected())
		return true;

	ArLog::log(ArLog::Normal, "%s: Disconnecting", getName());

	laserDisconnectNormally();
	return true;
}

void ArSZSeries::failedToConnect(void) {
	lockDevice();
	myTryingToConnect = true;
	unlockDevice();
	laserFailedConnect();
}

void ArSZSeries::sensorInterp(void) {
	ArSZSeriesPacket *packet;

	while (1) {
		myPacketsMutex.lock();
		if (myPackets.empty()) {
			myPacketsMutex.unlock();
			return;
		}
		packet = myPackets.front();
		myPackets.pop_front();
		myPacketsMutex.unlock();

		//set up the times and poses

		ArTime time = packet->getTimeReceived();
		
		ArPose pose;
		int ret;
		int retEncoder;
		ArPose encoderPose;
		int dist;
		int j;

		unsigned char *buf = (unsigned char *) packet->getBuf();

		// this value should be found more empirically... but we used 1/75
		// hz for the lms2xx and it was fine, so here we'll use 1/50 hz for now
		if (!time.addMSec(-30)) {
			ArLog::log(ArLog::Normal,
					"%s::sensorInterp() error adding msecs (-30)", getName());
		}

		if (myRobot == NULL || !myRobot->isConnected())
		{
			pose.setPose(0, 0, 0);
			encoderPose.setPose(0, 0, 0);
		}
		else if ((ret = myRobot->getPoseInterpPosition(time, &pose)) < 0
				|| (retEncoder = myRobot->getEncoderPoseInterpPosition(time,
						&encoderPose)) < 0)
		{
			ArLog::log(ArLog::Normal,
					"%s::sensorInterp() reading too old to process", getName());
			delete packet;
			continue;
		}

		ArTransform transform;
		transform.setTransform(pose);

		unsigned int counter = 0;
		if (myRobot != NULL)
			counter = myRobot->getCounter();

		lockDevice();
		myDataMutex.lock();

		//std::list<ArSensorReading *>::reverse_iterator it;
		ArSensorReading *reading;

		myNumChans = packet->getNumReadings();

		double eachAngularStepWidth;
		int eachNumberData;

		// PS - test for SZ-16D, each reading is .36 degrees for 270 degrees

		if (packet->getNumReadings() == 751)
		{
			eachNumberData = packet->getNumReadings();
		}
		else
		{
			ArLog::log(ArLog::Normal,
					"%s::sensorInterp() The number of readings is not correct = %d",
					getName(), myNumChans);

			// PS 12/6/12 - unlock before continuing

			delete packet;
			myDataMutex.unlock();
			unlockDevice();
			continue;
		}

		// If we don't have any sensor readings created at all, make 'em all
		if (myRawReadings->size() == 0) {
			for (j = 0; j < eachNumberData; j++) {
				myRawReadings->push_back(new ArSensorReading);
			}
		}

		if (eachNumberData > myRawReadings->size())
		{
			ArLog::log(ArLog::Terse,
					"%s::sensorInterp() Bad data, in theory have %d readings but can only have 751... skipping this packet",
					getName(), eachNumberData);

			// PS 12/6/12 - unlock and delete before continuing

			delete packet;
			myDataMutex.unlock();
			unlockDevice();
			continue;
		}

		std::list<ArSensorReading *>::iterator it;
		double atDeg;
		int onReading;

		double start;
		double increment;

		eachAngularStepWidth = .36;

		if (myFlipped) {
			start = mySensorPose.getTh() + 135;
			increment = -eachAngularStepWidth;
		} else {
			start = -(mySensorPose.getTh() + 135);
			increment = eachAngularStepWidth;
		}

		int readingIndex;
		bool ignore = false;

		for (atDeg = start,
				it = myRawReadings->begin(),
				readingIndex = 0,
				onReading = 0;

				onReading < eachNumberData;

				atDeg += increment,
				it++,
				readingIndex++,
				onReading++)
		{


			reading = (*it);

			dist = (((buf[readingIndex * 2] & 0x3f)<< 8) | (buf[(readingIndex * 2) + 1]));

			// note max distance is 16383 mm, if the measurement
			// object is not there, distance will still be 16383
            /*
			ArLog::log(ArLog::Normal,
			"reading %d first half = 0x%x, second half = 0x%x dist =  %d",
			readingIndex, buf[(readingIndex *2)+1], buf[readingIndex], dist);
            */

			reading->resetSensorPosition(ArMath::roundInt(mySensorPose.getX()),
					ArMath::roundInt(mySensorPose.getY()), atDeg);
			reading->newData(dist, pose, encoderPose, transform, counter, time,
					ignore, 0); // no reflector yet

			//printf("dist = %d, pose = %d, encoderPose = %d, transform = %d, counter = %d, time = %d, igore = %d",
			//		dist, pose, encoderPose, transform, counter,
			//					 time, ignore);
		}
/*
		 ArLog::log(ArLog::Normal,
		 "Received: %s %s scan %d numReadings %d", 
		 packet->getCommandType(), packet->getCommandName(), 
		 myScanCounter, onReading);
*/

		myDataMutex.unlock();

		/*
		ArLog::log(
				ArLog::Terse,
				"%s::sensorInterp() Telegram number =  %d  ",
				getName(),  packet->getTelegramNumByte2());
		 */

		laserProcessReadings();
		unlockDevice();
		delete packet;
	}
}

AREXPORT bool ArSZSeries::blockingConnect(void) {

	if (!getRunning())
		runAsync();

	myConnMutex.lock();
	if (myConn == NULL) {
		ArLog::log(ArLog::Terse,
				"%s: Could not connect because there is no connection defined",
				getName());
		myConnMutex.unlock();
		failedToConnect();
		return false;
	}


	//				myPrevSensorIntTime = myConn->getTimeRead(0);

	// PS 9/9/11 - moved this here to fix issue with setting baud in mt400.p
	laserPullUnsetParamsFromRobot();
	laserCheckParams();

	// PS 9/9/11 - add setting baud
    ArSerialConnection *serConn = NULL;
	serConn = dynamic_cast<ArSerialConnection *>(myConn);
	if (serConn != NULL)
		serConn->setBaud(atoi(getStartingBaudChoice()));

	if (myConn->getStatus() != ArDeviceConnection::STATUS_OPEN
			&& !myConn->openSimple()) {
		ArLog::log(
				ArLog::Terse,
				"%s: Could not connect because the connection was not open and could not open it",
				getName());
		myConnMutex.unlock();
		failedToConnect();
		return false;
	}

	// PS - set logging level and laser type in packet receiver class
	myReceiver.setmyInfoLogLevel(myInfoLogLevel);
	myReceiver.setmyName(getName());

	myReceiver.setDeviceConnection(myConn);
	myConnMutex.unlock();

	lockDevice();
	myTryingToConnect = true;
	unlockDevice();

	// PS 9/9/11 - moved up top
	//laserPullUnsetParamsFromRobot();
	//laserCheckParams();

	int size = ArMath::roundInt((270/.3) + 1);
	ArLog::log(myInfoLogLevel,
			"%s::blockingConnect() Setting current buffer size to %d",
			getName(), size);
	setCurrentBufferSize(size);

	ArTime timeDone;
	if (myPowerControlled)
	{
		if (!timeDone.addMSec(60 * 1000))
		{
			ArLog::log(ArLog::Normal,
					"%s::blockingConnect() error adding msecs (60 * 1000)",
					getName());
		}
	}
	else
	{
		if (!timeDone.addMSec(30 * 1000))
		{
			ArLog::log(ArLog::Normal,
					"%s::blockingConnect() error adding msecs (30 * 1000)",
					getName());
		}
	}


	ArSZSeriesPacket *packet;

	ArSZSeriesPacket sendPacket;

#if 0
	sendPacket.empty();
	sendPacket.uByteToBuf(0xA0); // stop continous sending
	sendPacket.uByteToBuf(0x00);
	sendPacket.uByteToBuf(0x1D);
	sendPacket.uByteToBuf(0x7E);

	sendPacket.finalizePacket();

	if ((myConn->write(sendPacket.getBuf(), sendPacket.getLength())) == -1)
	{
		ArLog::log(ArLog::Terse,
				"%s::blockingConnect() Could not send Stop Continuous mode to laser", getName());
		failedToConnect();
		return false;
	}
#endif


	// Build the Start Continuous sending packet and set it
	// once we get a response, then we are connected, note
	// the response needs to be a real reading

	sendPacket.empty();
	// command id = 0x91
	//sendPacket.uByteToBuf(145);
	sendPacket.uByteToBuf(0x91);
	// note communication ID default is 0
	// this value is set via the SZ Configurator
	// ???? not sure what to do if it fails
	// and put in CRC - from manual, this is
	// specific to the communication ID = 0
	sendPacket.uByteToBuf(0);
	sendPacket.uByteToBuf(43);
	sendPacket.uByteToBuf(218);

	unsigned short crc = myReceiver.CRC16((unsigned char *)sendPacket.getBuf(), 2);



#if 0
	// other communications IDs and CRC
	// communication ID =1
	sendPacket.uByteToBuf(1);
	sendPacket.uByteToBuf(59);
	sendPacket.uByteToBuf(251);
	// communication ID =2
	sendPacket.uByteToBuf(2);
	sendPacket.uByteToBuf(11);
	sendPacket.uByteToBuf(152);
	// communication ID =3
	sendPacket.uByteToBuf(3);
	sendPacket.uByteToBuf(27);
	sendPacket.uByteToBuf(185);
#endif

	sendPacket.finalizePacket();

	IFDEBUG(

			int i;
	char x[100000];
	printf("buffer with len = %d: ",sendPacket.getLength());
	for (i = 0;i < sendPacket.getLength();i++)
	{
		printf("0x%x ",sendPacket.getBuf()[i] & 0xff);
		//sprintf(&x[i], "%2x", (char *)sendPacket.getBuf()[i]);

	}
	printf("\n");

	//ArLog::log(ArLog::Terse,
	//		"%s::blockingConnect() write Buffer = %s", getName(), x);

	); // end IFDEBUG

	if ((myConn->write(sendPacket.getBuf(), sendPacket.getLength())) == -1)
	{
		ArLog::log(ArLog::Terse,
				"%s::blockingConnect() Could not send Start Continuous mode to laser", getName());
		failedToConnect();
		return false;
	}
	while (timeDone.mSecTo() > 0)
	{
		// PS 9/7/11 - just go read 1 byte
		packet = myReceiver.receivePacket(1000);
		//char c;
		//if ((myConn->read((char *) &c, 1, 1000)) > 0)

		if (packet != NULL)
		{
			delete packet;
			packet = NULL;

			lockDevice();
			myIsConnected = true;
			myTryingToConnect = false;
			unlockDevice();
			ArLog::log(ArLog::Normal, "%s::blockingConnect() Connected to laser", getName());
			laserConnect();
			return true;
		}
		else
		{
			ArLog::log(ArLog::Normal, "%s::blockingConnect() Did not get response to Start Continuous request",
					getName());
			delete packet;
			packet = NULL;
		}

	}



	ArLog::log(ArLog::Terse,
			"%s::blockingConnect()  Did not get scan data back from laser",
			getName());
	failedToConnect();
	return false;

}

AREXPORT void * ArSZSeries::runThread(void *arg) {
	//char buf[1024];
	ArSZSeriesPacket *packet;

	while (getRunning())
	{
		lockDevice();
		if (myStartConnect)
		{
			myStartConnect = false;
			myTryingToConnect = true;
			unlockDevice();

			blockingConnect();

			lockDevice();
			myTryingToConnect = false;
			unlockDevice();
			continue;
		}
		unlockDevice();

		if (!myIsConnected)
		{
			ArUtil::sleep(100);
			continue;
		}


	// PS 10/20/11 - code to fix disconnect issues
	
	while (getRunning() && myIsConnected &&
	       (packet = myReceiver.receivePacket (500, false) ) != NULL) {
		myPacketsMutex.lock();
		myPackets.push_back (packet);
		myPacketsMutex.unlock();
		
		if (myRobot == NULL)
			sensorInterp();
	}
		
	// if we have a robot but it isn't running yet then don't have a
	// connection failure
	if (getRunning() && myIsConnected && laserCheckLostConnection() ) {
		ArLog::log (ArLog::Normal,
		            "%s::runThread()  Lost connection to the laser because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
		            myLastReading.mSecSince() / 1000.0,
		            getConnectionTimeoutSeconds() );
		myIsConnected = false;
		laserDisconnectOnError();
		continue;
	}

		//ArUtil::sleep(1);
		//ArUtil::sleep(2000);
		//ArUtil::sleep(500);

#if 0  // PS 10/20/11 - using code above to fix disconnect issues

		// PS 7/5/11 - change msWait from 50 to 5000

		// MPL 7/12/11 Changed mswait to 500 (which is bad enough,
		// especially since receive packet doesn't use it quite right at
		// this time)
		while (getRunning() && myIsConnected && (packet
				= myReceiver.receivePacket(500, false)) != NULL)
		{

			myPacketsMutex.lock();
			myPackets.push_back(packet);
			myPacketsMutex.unlock();

			//ArLog::log(ArLog::Terse, "myRobot = %s",myRobot);

			//if (myRobot == NULL)
			//sensorInterp();

			/// MPL TODO see if this gets called if the laser goes
			/// away... it looks like it may not (since the receivePacket may just return nothing)

			// if we have a robot but it isn't running yet then don't have a
			// connection failure
			if (laserCheckLostConnection())
			{
				ArLog::log(ArLog::Terse,
						"%s:  Lost connection to the laser because of error.  Nothing received for %g seconds (greater than the timeout of %g).",
						getName(), myLastReading.mSecSince() / 1000.0,
						getConnectionTimeoutSeconds());
				myIsConnected = false;

				laserDisconnectOnError();
				continue;
			}
		}

		ArUtil::sleep(1);
		//ArUtil::sleep(2000);
		//ArUtil::sleep(500);
		
#endif
		
	}
	return NULL;
}


static const unsigned short
crc_table[256] = { 0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5,
		0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad,
		0xe1ce, 0xf1ef, 0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294,
		0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c,
		0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7,
		0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf,
		0xc5ac, 0xd58d, 0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6,
		0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe,
		0xd79d, 0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861,
		0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969,
		0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50,
		0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58,
		0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03,
		0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b,
		0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32,
		0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,
		0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d,
		0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025,
		0x7046, 0x6067, 0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c,
		0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214,
		0x6277, 0x7256, 0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f,
		0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447,
		0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e,
		0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676,
		0x4615, 0x5634, 0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,
		0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1,
		0x3882, 0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8,
		0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0,
		0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b,
		0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83,
		0x1ce0, 0x0cc1, 0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba,
		0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2,
		0x0ed1, 0x1ef0 };

unsigned short ArSZSeriesPacketReceiver::CRC16(unsigned char *Data, int length) {
	unsigned short CRC_16 = 0x0000;
	int i;

	for (i = 0; i < length; i++)
	{
		CRC_16 = (CRC_16 << 8) ^ (crc_table[(CRC_16 >> 8) ^ (Data[i])] );
//printf("CRC_16=0x%x\n",CRC_16);
	}

	return CRC_16;
}

