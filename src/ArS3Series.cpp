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
#include "ArS3Series.h"
#include "ArRobot.h"
#include "ArSerialConnection.h"
#include "ariaInternal.h"
#include <time.h>


AREXPORT ArS3SeriesPacket::ArS3SeriesPacket() :
ArBasePacket(10000, 1, NULL, 1) {

}

AREXPORT ArS3SeriesPacket::~ArS3SeriesPacket() {

}

AREXPORT ArTime ArS3SeriesPacket::getTimeReceived(void) {
	return myTimeReceived;
}

AREXPORT void ArS3SeriesPacket::setTimeReceived(ArTime timeReceived) {
	myTimeReceived = timeReceived;
}

AREXPORT void ArS3SeriesPacket::duplicatePacket(ArS3SeriesPacket *packet) {
	myLength = packet->getLength();
	myReadLength = packet->getReadLength();
	myTimeReceived = packet->getTimeReceived();
	myDataLength = packet->myDataLength;
	myNumReadings = packet->myNumReadings;
	myStatusByte = packet->myStatusByte;
	myTimeStampByte1 = packet->myTimeStampByte1;
	myTimeStampByte2 = packet->myTimeStampByte2;
	myTimeStampByte3 = packet->myTimeStampByte3;
	myTimeStampByte4 = packet->myTimeStampByte4;
	myTelegramNumByte1 = packet->myTelegramNumByte1;
	myTelegramNumByte2 = packet->myTelegramNumByte2;
	myCrcByte1 = packet->myCrcByte1;
	myCrcByte2 = packet->myCrcByte2;
	myMonitoringDataByte1 = packet->myMonitoringDataByte1;
	myMonitoringDataByte2 = packet->myMonitoringDataByte2;
	myMonitoringDataAvailable = packet->myMonitoringDataAvailable;
	myProtocolVersionByte1 = packet->myProtocolVersionByte1;
	myProtocolVersionByte2 = packet->myProtocolVersionByte2;

	memcpy(myBuf, packet->getBuf(), myLength);
}

AREXPORT void ArS3SeriesPacket::empty(void) {
	myLength = 0;
	myReadLength = 0;
}

AREXPORT ArS3SeriesPacketReceiver::ArS3SeriesPacketReceiver() {

}

AREXPORT ArS3SeriesPacketReceiver::~ArS3SeriesPacketReceiver() {

}

AREXPORT void ArS3SeriesPacketReceiver::setDeviceConnection(
		ArDeviceConnection *conn) {
	myConn = conn;
}

AREXPORT ArDeviceConnection *ArS3SeriesPacketReceiver::getDeviceConnection(void) {
	return myConn;
}

ArS3SeriesPacket *ArS3SeriesPacketReceiver::receivePacket(unsigned int msWait,
		bool startMode) {

	ArS3SeriesPacket *packet;
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
		ArLog::log(ArLog::Verbose, "%s::receivePacket() error adding msecs (%i)",
				myName, msWait);
	}
	msWait = 10;

	do {
		myConn->debugStartPacket();
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
		int n = 0;

		// look for initial sequence 0x00 0x00 0x00 0x00
		for (i = 0; i < 4; i++) {
			if ((myConn->read((char *) &c, 1, 200)) > 0) {
				if (c != 0x00) {
					//ArLog::log(ArLog::Terse,
					//                 "ArS3Series::receivePacket() error reading first 4 bytes of header");
					break;
				}
				if (i == 0) {

					packetReceived = myConn->getTimeRead(0);
					myPacket.setTimeReceived(packetReceived);
				}
				myConn->debugBytesRead(1);
			} else {
				/* don't log this if we are in starting mode, means laser is not connecting */
				if (startMode)
					ArLog::log(ArLog::Terse,
							"%s::receivePacket() myConn->read error (header)",
							myName);
				myConn->debugEndPacket(false, -10);
				return NULL;
			}
		} // end for

		if (c != 0x00)
		{
			myConn->debugEndPacket(false, -11);
			continue;
		}

		// next 2 bytes = 0x00 0x00 - data block number
		for (i = 0; i < 2; i++) {
			if ((myConn->read((char *) &c, 1, msWait)) > 0) {
			        myConn->debugBytesRead(1);
				if (c != 0x00) {
					//ArLog::log(ArLog::Terse,
					//                 "ArS3Series::receivePacket() error data block number in header");
					break;
				}
			} else {
				ArLog::log(
						ArLog::Terse,
						"%s::receivePacket() myConn->read error (data block number)",
						myName);
				myConn->debugEndPacket(false, -20);
				return NULL;
			}
		} // end for

		if (c != 0x00)
		{
			myConn->debugEndPacket(false, -21);
			continue;
		}

		crcbuf[n++] = 0;
		crcbuf[n++] = 0;

		// next 2 bytes are length, i think they are swapped so we need to mess with them

		for (i = 0; i < 2; i++) {
			if ((myConn->read((char *) &c, 1, msWait)) > 0) {
			        myConn->debugBytesRead(1);
				temp[i] = c;
				crcbuf[n++] = c;
			} else {
				ArLog::log(ArLog::Terse,
						"%s::receivePacket() myConn->read error (length)",
						myName);
				myConn->debugEndPacket(false, -30);
				return NULL;
			}
		} // end for

		firstbytelen = temp[0];
		secondbytelen = temp[1];

		// do we need to validate byte length
		int datalen = secondbytelen | (firstbytelen << 8);

		// double it as this is 2 byte pairs of readings
		// and take off the header of 17 bytes
		myPacket.setDataLength((datalen * 2) - 17);
		//printf("datalength = %d \n",myPacket.getDataLength());

		// the number of reading is going to be 4 bytes less as there's
		// a bb bb and 11 11 (which are ID for measurement data and ID
		// for measured values from angular range 1
		myPacket.setNumReadings(((myPacket.getDataLength() - 5) / 2) - 1);

		if (myPacket.getNumReadings() < 0) {
			ArLog::log(ArLog::Terse,
								"%s::receivePacket() myConn->read error (header - bad number of readings) %d %d",
								myName, firstbytelen, secondbytelen);
			myConn->debugEndPacket(false, -40);
			return NULL;
		}

		/*
		//ArLog::Terse,
			ArLog::log(ArLog::Normal,
		"%s::receivePacket() Number of readings = %d %d %d", myName, myPacket.getNumReadings(), firstbytelen, secondbytelen);
		*/

		// next 2 bytes need to be 0xff & 0x07
		for (i = 0; i < 2; i++) {
			if ((myConn->read((char *) &c, 1, msWait)) > 0) {
			        myConn->debugBytesRead(1);
				temp[i] = c;
				crcbuf[n++] = c;
			} else {
				ArLog::log(
						ArLog::Terse,
						"%s::receivePacket() myConn->read error (coordination flag and device code)",
						myName);
				myConn->debugEndPacket(false, -50);
				return NULL;
			}
		} // end for

		if (temp[0] != 0xff || temp[1] != 0x07)
		{
			/*
		  ArLog::Terse,
			ArLog::log(ArLog::Normal,
			     "ArS3Series::receivePacket() co-oridination flag and device code error");
			 */
			myConn->debugEndPacket(false, -51);
			continue;
		}

		// next 2 bytes are protocol version to be 0x02 & 0x01

		for (i = 0; i < 2; i++) {
			if ((myConn->read((char *) &c, 1, msWait)) > 0)
			{
				myConn->debugBytesRead(1);
				temp[i] = c;
				crcbuf[n++] = c;
			}
			else
			{
				ArLog::log(
						ArLog::Terse,
						"%s::receivePacket() myConn->read error (protocol version?)",
						myName);
				myConn->debugEndPacket(false, -55);
				return NULL;
			}
		} // end for

		// we have an old S3000 who's protocol is 00 01, later versions are 02 01
		// PS 6/11/13 - for Expert CMS protocol version is 03 01
		if ((temp[0] == 0x00 || temp[1] == 0x01) ||
				(temp[0] == 0x02 || temp[1] == 0x01) ||
				(temp[0] == 0x03 || temp[1] == 0x01))
		{
			myPacket.setProtocolVersionByte1(temp[0]);
			myPacket.setProtocolVersionByte2(temp[1]);
		}
		else
		{
			ArLog::log(ArLog::Terse,
					"%s::receivePacket() protocol version error (0x%02x 0x%02x)",
					myName, temp[0], temp[1]);
			myConn->debugEndPacket(false, -56);
			continue;
		}

		// next 1 byte is status flag

		for (i = 0; i < 1; i++)
		{
			if ((myConn->read((char *) &c, 1, msWait)) > 0)
			{
				myConn->debugBytesRead(1);
				temp[i] = c;
				crcbuf[n++] = c;
			}
			else
			{
				ArLog::log(ArLog::Terse,
						"%s::receivePacket() myConn->read error (status flag?)",
						myName);
				myConn->debugEndPacket(false, -60);
				return NULL;
			}
		} // end for

		myPacket.setStatusByte(temp[0]);

		// next 4 bytes are timestamp

		for (i = 0; i < 4; i++)
		{
			if ((myConn->read((char *) &c, 1, msWait)) > 0)
			{
				myConn->debugBytesRead(1);
				temp[i] = c;
				crcbuf[n++] = c;
			}
			else
			{
				ArLog::log(ArLog::Terse,
						"%s::receivePacket() myConn->read error (time stamp)",
						myName);
				myConn->debugEndPacket(false, -70);
				return NULL;
			}
		} // end for

		myPacket.setTimeStampByte1(temp[0]);
		myPacket.setTimeStampByte2(temp[1]);
		myPacket.setTimeStampByte3(temp[2]);
		myPacket.setTimeStampByte4(temp[3]);

		/*
		ArLog::log(
				ArLog::Terse,
				"%s::receivePacket() Time stamp = %x %x %x %x ",
				myName, temp[0],temp[1],temp[2],temp[3]);
		 */

		// next 2 bytes are telegram number

		for (i = 0; i < 2; i++)
		{
			if ((myConn->read((char *) &c, 1, msWait)) > 0)
			{
				myConn->debugBytesRead(1);
				temp[i] = c;
				crcbuf[n++] = c;
			}
			else
			{
				ArLog::log(ArLog::Terse,
						"%s::receivePacket() myConn->read error (telegram number)",
						myName);
				myConn->debugEndPacket(false, -80);
				return NULL;
			}
		} // end for

		myPacket.setTelegramNumByte1(temp[0]);
		myPacket.setTelegramNumByte2(temp[1]);

		/*
		ArLog::log(
				ArLog::Terse,
				"%s::receivePacket() Telegram number =  %d  ",
				myName,  myPacket.getTelegramNumByte2());
		 */
		/// MPL this timeout was 5000, but I've made it 200
		/// since the number of readings could be bogus and we
		/// don't want to go 5 seconds with no readings
		int numRead = myConn->read((char *) &myReadBuf[0],
						// PS 12/10/12 - change to 400
					   myPacket.getDataLength(), 400);
					   //myPacket.getDataLength(), 200);

		// trap if we failed the read
		if (numRead < 0) {
			ArLog::log(ArLog::Terse, "%s::receivePacket() Failed read (%d)",
					myName, numRead);
			myConn->debugEndPacket(false, -90);
			return NULL;
		}
		myConn->debugBytesRead(numRead);
		/*
		ArLog::log(ArLog::Terse,
				"%s::receivePacket() Number of bytes read = %d", myName,
				numRead);
		 */

#if 0 // for raw trace
				char obuf[10000];
				obuf[0] = '\0';
				int j = 0;
				for (int i = 0; i < myPacket.getDataLength() - 2; i++) {
					sprintf (&obuf[j], "_%02x", myReadBuf[i]);
					j= j+3;
				}
				ArLog::log (ArLog::Normal,
				            "%s::receivePacket() packet = %s ",myName, obuf);
#endif

		//printf("\nhere's the data from packetrecieve\n ");
		//for (i = 0; i < myPacket.getDataLength(); i++) {
		//strip out first 3 bytes
		//if ((i > 5) && ((i % 2 == 0)))
		//myReadBuf[i] = myReadBuf[i] && 0x1f;

		//printf("%x ", myReadBuf[i]);
		//}

		//printf("\n");
		// and finally the crc

		// start after the 00 bb bb 11 11 and put the
		// raw readings into myReadBuf
		//printf("\nhere's the raw readings\n ");
		//for (i=5; i<(myNumReadings * 2); i+2)
		//  {
		//	  // this my be backwards
		//  myReadBuf[i] = myReadBuf[i] && 0x1f;
		//  printf("%x %x",myReadBuf[i],myReadBuf[i+1]);
		// }

		for (i = 0; i < 2; i++)
		{
			if ((myConn->read((char *) &c, 1, msWait)) > 0)
			{
				myConn->debugBytesRead(1);
				temp[i] = c;
			}
			else
			{
				ArLog::log(ArLog::Terse,
						"%s::receivePacket() myConn->read error (crc)", myName);
				myConn->debugEndPacket(false, -100);
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
				myConn->debugEndPacket(false, -110);
				return NULL;
			}
		}
#endif

		memcpy(&crcbuf[n], &myReadBuf[0], myPacket.getDataLength());

#if 0 // for raw trace
				char obuf[10000];
				obuf[0] = '\0';
				int j = 0;
				for (int i = 0; i < myPacket.getDataLength() + n; i++) {
// to show header + data uncomment out below
//					sprintf (&obuf[j], "_%02x", myReadBuf[i]);
					sprintf (&obuf[j], "_%02x", crcbuf[i]);
					j= j+3;
				}
				ArLog::log (ArLog::Normal,
				            "%s::receivePacket() packet = %s ",myName, obuf);
#endif

		// now go validate the crc

		unsigned short crc = CRC16(crcbuf, myPacket.getDataLength() + n);

		unsigned short incrc = (temp[1] << 8) | temp[0];

		if (incrc != crc)
		{
			ArLog::log(ArLog::Terse,
					"%s::receivePacket() CRC error (in = 0x%02x calculated = 0x%02x) ",
					myName, incrc, crc);
			myConn->debugEndPacket(false, -120);
			return NULL;
		}

		// PS 7/5/11 - there are 5 bytes of 00 bb bb 11 11 at the start of
		// the buffer - which is the ID for measured data from measuring 
		// range 1, so skip over those so that we have the begining of
		// the readings - note if Range 2-5 are configured, then the id's 
		// for those would be 22 22, 33 33, etc - we only support range 1 (ie 11 11)

		myPacket.setMonitoringDataAvailable(false);
		if ((myReadBuf[1] == 0xaa) && (myReadBuf[2] == 0xaa)) {

/*
			ArLog::log(ArLog::Normal, "%s monitoring case of = %d seq = %d %d time = %d %d %d %d",
					myName, myReadBuf[3], myPacket.getTelegramNumByte1(), myPacket.getTelegramNumByte2(),
					myPacket.getTimeStampByte1(), myPacket.getTimeStampByte2(), myPacket.getTimeStampByte3(),
					myPacket.getTimeStampByte4());
*/
			myPacket.setDataLength((datalen * 2) - 21);
			myPacket.setNumReadings(((myPacket.getDataLength() - 5) / 2) - 1);

			myPacket.setMonitoringDataAvailable(true);
			myPacket.setMonitoringDataByte1(myReadBuf[3]);
			myPacket.setMonitoringDataByte2(myReadBuf[4]);

			myPacket.dataToBuf(&myReadBuf[9], myPacket.getNumReadings() * 2);
		}
		else
			myPacket.dataToBuf(&myReadBuf[5], myPacket.getNumReadings() * 2);

		myPacket.resetRead();
		packet = new ArS3SeriesPacket;
		packet->duplicatePacket(&myPacket);
		myConn->debugEndPacket(true, 1);
		/*
		ArLog::log(ArLog::Normal,
		           "%s::receivePacket() returning packet %d %d", myName, packet->getNumReadings(), myPacket.getNumReadings());
		*/

		myPacket.empty();

		return packet;

	} while (timeDone.mSecTo() >= 0); // || !myStarting)

	ArLog::log(ArLog::Terse, "%s::receivePacket() Timeout on read", myName);
	myConn->debugEndPacket(false, -130);
	return NULL;
}

AREXPORT ArS3Series::ArS3Series(int laserNumber, const char *name) :
			ArLaser(laserNumber, name, 20000),
			mySensorInterpTask(this, &ArS3Series::sensorInterp),
			myAriaExitCB(this, &ArS3Series::disconnect),
			myPacketHandlerCB(this, &ArS3Series::packetHandler) {

	//ArLog::log(ArLog::Normal, "%s: Sucessfully created", getName());

  mySendFakeMonitoringData = false;

	clear();
	myRawReadings = new std::list<ArSensorReading *>;

	myIsMonitoringDataAvailable = false;

	Aria::addExitCallback(&myAriaExitCB, -10);

	setInfoLogLevel(ArLog::Normal);
	//setInfoLogLevel(ArLog::Terse);

	laserSetName( getName());

	laserAllowSetPowerControlled(false);

	/* taking this out for now - PS 6/29/11
	 laserAllowSetDegrees(-135, -135, 135, // start degrees
	 135, -135, 135); // end degrees

	 std::map<std::string, double> incrementChoices;
	 incrementChoices["half"] = .5;
	 incrementChoices["quarter"] = .25;
	 laserAllowIncrementChoices("half", incrementChoices);

	 laserSetDefaultTcpPort(2111);

	 */
  // rh adding back in ...
  laserAllowSetDegrees(-135, -135, -135, 135, 135, 135);
  std::map<std::string, double> incrementChoices;
  incrementChoices["half"] = 0.5;
  laserAllowIncrementChoices("half", incrementChoices);

	// PS 6/29/11 - changing to serial laserSetDefaultPortType("tcp");
	laserSetDefaultPortType("serial422");

	// PS 6/29/11 - added baud
	std::list < std::string > baudChoices;

	baudChoices.push_back("9600");
	baudChoices.push_back("19200");
	baudChoices.push_back("38400");
	baudChoices.push_back("57600");
	baudChoices.push_back("115200");
	//baudChoices.push_back("125000");
	baudChoices.push_back("230400");
	baudChoices.push_back("460800");

	laserAllowStartingBaudChoices("38400", baudChoices);

	// PS 7/1/11
	//laserAllowAutoBaudChoices("38400", baudChoices);

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

#if 0
	setCurrentDrawingData(
			new ArDrawingData("polyDots",
					ArColor(0, 0, 255),
					80, // mm diameter of dots
					75), // layer above sonar
					true);

	setCumulativeDrawingData(
			new ArDrawingData("polyDots",
					ArColor(125, 125, 125),
					100, // mm diameter of dots
					60), // layer below current range devices
					true);

#endif
	// PS make theses a different color, etc
	setCurrentDrawingData(
			new ArDrawingData("polyDots", ArColor(223, 223, 0), 75, // mm diameter of dots
					76), // layer above sonar
					true);

	setCumulativeDrawingData(
			new ArDrawingData("polyDots", ArColor(128, 128, 0), 95, // mm diameter of dots
					61), // layer below current range devices
					true);

}

AREXPORT ArS3Series::~ArS3Series() {
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

void ArS3Series::clear(void) {
	myIsConnected = false;
	myTryingToConnect = false;
	myStartConnect = false;

	myNumChans = 0;
}

AREXPORT void ArS3Series::laserSetName(const char *name) {
	myName = name;

	myConnMutex.setLogNameVar("%s::myConnMutex", getName());
	myPacketsMutex.setLogNameVar("%s::myPacketsMutex", getName());
	mySafetyDebuggingTimeMutex.setLogNameVar("%s::mySafetyDebuggingTimeMutex", 
					     getName());
	myDataMutex.setLogNameVar("%s::myDataMutex", getName());
	myAriaExitCB.setNameVar("%s::exitCallback", getName());

	ArLaser::laserSetName( getName());
}

AREXPORT void ArS3Series::setRobot(ArRobot *robot) {
	myRobot = robot;

	if (myRobot != NULL) {
		myRobot->remSensorInterpTask(&mySensorInterpTask);
		myRobot->addSensorInterpTask("S3Series", 90, &mySensorInterpTask);
		myRobot->remPacketHandler(&myPacketHandlerCB);
		myRobot->addPacketHandler(&myPacketHandlerCB);
	}
	ArLaser::setRobot(robot);
}

AREXPORT bool ArS3Series::asyncConnect(void) {
	myStartConnect = true;
	if (!getRunning())
		runAsync();
	return true;
}

AREXPORT bool ArS3Series::disconnect(void) {
	if (!isConnected())
		return true;

	ArLog::log(ArLog::Normal, "%s: Disconnecting", getName());

	laserDisconnectNormally();
	return true;
}

void ArS3Series::failedToConnect(void) {
	lockDevice();
	myTryingToConnect = true;
	unlockDevice();
	laserFailedConnect();
}

void ArS3Series::sensorInterp(void) {
	ArS3SeriesPacket *packet;

	bool safetyDebugging = false;

	mySafetyDebuggingTimeMutex.lock();
	if (mySafetyDebuggingTime.mSecSince() < 1000)
	  safetyDebugging = true;
	mySafetyDebuggingTimeMutex.unlock();

	if (safetyDebugging)
	  ArLog::log(ArLog::Normal, 
		     "%s::SafetyDecommissionWarning: Robot speed %.0f mm/sec (in cycle)",
		     myName.c_str(), 
		     myRobot->getVel());

	/// MPL 2013_07_24 testing (added)
	lockDevice();

	/// MPL 2013_07_24 testing (added)
	adjustRawReadings(false);

	while (1) {
		myPacketsMutex.lock();
		if (myPackets.empty()) {
			myPacketsMutex.unlock();
			/// MPL 2013_07_24 testing (added)
			unlockDevice();
			return;
		}
		packet = myPackets.front();
		myPackets.pop_front();
		// MPL this was some code to only use the latest laser
		// packet, but that leaked memory because the
		// deleteSet wasn't there, just reverting to the old
		// way (the two lines above this comment)
		//packet = myPackets.back();
		//myPackets.clear();
		//ArUtil::deleteSet(myPackets.begin(), myPackets.end());
		myPacketsMutex.unlock();

		//set up the times and poses

		ArTime time = packet->getTimeReceived();
		ArTime timeEnd = packet->getTimeReceived();
		ArPose pose;
		ArPose poseEnd;
		int ret;
		int retEncoder;
		ArPose encoderPose;
		ArPose encoderPoseEnd;
		int dist;
		int j;

		// Packet will already be offset by 5 bytes to the start
		// of the readings - for S3000 there should be 381 readings (190 degrees)
		// for S300 541 readings (270 degrees)
		unsigned char *buf = (unsigned char *) packet->getBuf();

#if 0 // tracing code
				char obuf[10000];
				obuf[0] = '\0';
				int z = 0;
				for (int i = 0; i < packet->getDataLength(); i++) {
					sprintf (&obuf[z], "_%02x", buf[i]);
					z= z+3;
				}
				ArLog::log (ArLog::Normal,
				            "%s::sensorInterp() packet = %s ", getName(), obuf);
#endif

				/* MPL 2013_07_19 moving this into the part that gets a packet so that it's not subjected to the normal cycle time
		// if the monitoring data is available - send it down the firmware

		if (packet->getMonitoringDataAvailable()) {

			myIsMonitoringDataAvailable = true;
			myMonitoringData = packet->getMonitoringDataByte1();

			myRobot->comInt(217, packet->getMonitoringDataByte1());  

		}
		else {
			myIsMonitoringDataAvailable = false;
		}
				*/

		// this value should be found more empirically... but we used 1/75
		// hz for the lms2xx and it was fine, so here we'll use 1/50 hz for now
		// PS 7/9/11 - not sure what this is doing????

		// MPL 2013_06_03 - the S300 and the S3000 work
		// differently for timing...  on the S300 the data is
		// supposed to be real time (SICK seems to disagree
		// with themselves on this point), so assume no delay
		// (don't change the receive time)... but on the S3000
		// there's a delay of one scan (30ms) so add that if
		// the number of readings means it's an S3000

		bool interpolateReadings = false;
		if (packet->getNumReadings() == 381) /// S3000
		{
		  interpolateReadings = false;
		  if (!time.addMSec(-30)) {
		    ArLog::log(ArLog::Normal,
			       "%s::sensorInterp() error adding msecs (-30)", getName());
		  }
		}
		else if (packet->getNumReadings() == 540) // S300
		{
		  // for the S300 we're going to try and compensate
		  //for the robot's movement while it's turning
		  // just not right now
		  //interpolateReadings = true; 
		  // don't touch time
		  if (!timeEnd.addMSec(30))
		  {
		    ArLog::log(ArLog::Normal,
			       "%s::sensorInterp() error adding end msecs (30)", getName());
		  }
		}

		// MPL this was from debugging the intermittent lost
		// issue thatl ooked like a timing problem
		//ArLog::log(ArLog::Normal, "%s packet %lld mSec old", getName(), time.mSecSince());
		
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
					"%s::sensorInterp(): Warning: reading too old to process", getName());
			delete packet;
			continue;
		}

		if (interpolateReadings && 
		    ((ret = myRobot->getPoseInterpPosition(timeEnd, 
							   &poseEnd)) < 0
		     || (retEncoder = myRobot->getEncoderPoseInterpPosition(
				 timeEnd, &encoderPoseEnd)) < 0))
		{
		  ArLog::log(ArLog::Normal,
			     "%s::sensorInterp(): Warning: reading too old to process end", getName());
		  delete packet;
		  continue;
		}

		ArTransform transform;
		transform.setTransform(pose);

		unsigned int counter = 0;
		if (myRobot != NULL)
			counter = myRobot->getCounter();

		/// MPL 2013_07_24 testing (commented out)
		//lockDevice();
		myDataMutex.lock();

		//std::list<ArSensorReading *>::reverse_iterator it;
		ArSensorReading *reading;

		myNumChans = packet->getNumReadings();

		double eachAngularStepWidth;
		int eachNumberData;

		// PS - test for both S3000 (190 degrees) and S300 (270 degrees)
		if ((packet->getNumReadings() == 381) || 
					(packet->getNumReadings() == 540))
		{
			// PS 7/5/11 - grab the number of raw readings from the receive packet
			eachNumberData = packet->getNumReadings();
		}
		else
		{
			ArLog::log(ArLog::Normal,
					"%s::sensorInterp(): Warning: The number of readings is not correct = %d",
					getName(), myNumChans);

			// PS 12/6/12 - need to unlock
			myDataMutex.unlock();
			/// MPL 2013_07_24 testing (commented out)
			//unlockDevice();

			delete packet;
			continue;
		}

		// If we don't have any sensor readings created at all, make 'em all
		if (myRawReadings->size() == 0)
			for (j = 0; j < eachNumberData; j++)
				myRawReadings->push_back(new ArSensorReading);

		if (eachNumberData > myRawReadings->size())
		{
			ArLog::log(ArLog::Terse,
					"%s::sensorInterp() Bad data, in theory have %d readings but can only have 541... skipping this packet",
					getName(), eachNumberData);

			// PS 12/6/12 - need to unlock and delete packet
			myDataMutex.unlock();
			/// MPL 2013_07_24 testing (commented out)
			//unlockDevice();
			delete packet;

			continue;
		}

		std::list<ArSensorReading *>::iterator it;
		double atDeg;
		int onReading;

		double start;
		double increment;

		//eachStartingAngle = -5;
		eachAngularStepWidth = .5;

		// from the number of readings, calculate the start

		if (myFlipped) {
			//start = mySensorPose.getTh() + eachStartingAngle - 90.0 + eachAngularStepWidth * eachNumberData;
			start = mySensorPose.getTh() + (packet->getNumReadings() / 4);
			increment = -eachAngularStepWidth;
		} else {
			//start = mySensorPose.getTh() + eachStartingAngle - 90.0;
			start = -(mySensorPose.getTh() + (packet->getNumReadings() / 4));
			increment = eachAngularStepWidth;
		}

		int readingIndex;
		bool ignore = false;


		// The MonitoringData - 2 bytes

		// bits 0-3 Active Monitoring Case
		// bits 4-7 - not used
		// bits 8-10 Monitoring Area
		// bits 11-15 - not used

		// ???? - so we need to grab bits 0-3, but i'm not sure if they are in 
		// byte 1 or byte 2 and then pass them to something

		// On page 41 of the S300 telegram manual it defines the scan data block.
		// There is a 4-bit field within RI_SCAN_STATUS called “monitoring case”.  
		// We will need to relay this 4-bit value back to the C2K periodically (say with the normal motion commands every XXms).  
		// This will be used by the C2K as part of the diagnostic checks of the safety system hardware.  
		// The C2K will need to check that the SICK reported state matches the safety HW state. 
		// 
		
		if (packet->getMonitoringDataAvailable()) {

			// I'm not sure if theses are swapped ie lsb first, but i think they are
			// as the distances are that way

		  unsigned char activeMonitoringCase = packet->getMonitoringDataByte2() & 0x0f;
			//unsigned char activeMonitoringCase = packet->getMonitoringDataByte1() & 0x0f;

			//myRobot->processActiveMonitoringCase(activeMonitoringCase);
		}

		double incrX = 0;
		double incrY = 0;
		double incrTh = 0;
		ArPose interpolateDelta(0, 0, 0);
		if (interpolateReadings)
		{
		  incrX = ((encoderPoseEnd.getX() - encoderPose.getX()) / 
			   packet->getNumReadings());
		  incrY = ((encoderPoseEnd.getY() - encoderPose.getY()) / 
			   packet->getNumReadings());
		  incrTh = (ArMath::subAngle(encoderPoseEnd.getTh(), 
					     encoderPose.getTh()) / 
			    packet->getNumReadings());
		  
		  /*
		  incrX = ((encoderPose.getX() - encoderPoseEnd.getX()) / 
			   packet->getNumReadings());
		  incrY = ((encoderPose.getY() - encoderPoseEnd.getY()) / 
			   packet->getNumReadings());
		  incrTh = (ArMath::subAngle(encoderPose.getTh(), 
					     encoderPoseEnd.getTh()) / 
			    packet->getNumReadings());
		  */

		  ArLog::log(ArLog::Normal, 
   "%s:InterpolateReadings: diffAll of %d mSec, x %g y %g th %g (incr x %g y %g th %g) start x %g y %g th %g end x %g y %g th %g",
			     getName(), 
			     timeEnd.mSecSince(time), 
			     encoderPoseEnd.getX() - encoderPose.getX(), 
			     encoderPoseEnd.getY() - encoderPose.getY(), 
			     ArMath::subAngle(encoderPoseEnd.getTh(),
					      encoderPose.getTh()),
			     incrX, incrY, incrTh,
			     encoderPose.getX(), encoderPose.getY(), 
			     encoderPose.getTh(),
			     encoderPoseEnd.getX(), encoderPoseEnd.getY(), 
			     encoderPoseEnd.getTh());

		  
		  ArLog::log(ArLog::Normal, 
   "%s:InterpolateReadings: diffTh of %d mSec, th %g (%g) start th %g end th %g",
			     getName(), 
			     timeEnd.mSecSince(time), 
			     ArMath::subAngle(encoderPoseEnd.getTh(),
					      encoderPose.getTh()),
			     incrTh,
			     encoderPose.getTh(),
			     encoderPoseEnd.getTh());

		}

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

			dist = ((buf[(readingIndex * 2) + 1] & 0x8f) << 8)
							| buf[readingIndex * 2];
			dist = dist * 10; // convert to mm

			if (interpolateReadings)
			{
			  interpolateDelta.setX(
				  interpolateDelta.getX() + incrX);
			  interpolateDelta.setY(
				  interpolateDelta.getY() + incrY);
			  interpolateDelta.setTh(
				  ArMath::addAngle(interpolateDelta.getTh(),
						   incrTh));

			  /*
			  ArLog::log(ArLog::Normal, "%d %g %g %g",
				     onReading, 
				     interpolateDelta.getX(), 
				     interpolateDelta.getY(), 
				     interpolateDelta.getTh());
			  */

			  reading->resetSensorPosition(
				  ArMath::roundInt(mySensorPose.getX() + 
						   interpolateDelta.getX()),
				  ArMath::roundInt(mySensorPose.getY() + 
						   interpolateDelta.getY()),
				  ArMath::addAngle(atDeg,
						   interpolateDelta.getTh()));
			}
			else
			{
			  reading->resetSensorPosition(
				  ArMath::roundInt(mySensorPose.getX()),
				  ArMath::roundInt(mySensorPose.getY()), 
				  atDeg);
			}
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
		/// MPL 2013_07_24 testing (commented out)
		//unlockDevice();
		delete packet;
	}
	/// MPL 2013_07_24 testing (added)
	unlockDevice();
}

AREXPORT bool ArS3Series::blockingConnect(void) {
	long timeToRunFor;

	if (!getRunning())
		runAsync();

	myConnMutex.lock();
	if (myConn == NULL) {
		ArLog::log(ArLog::Terse,
				"%s: Error: Could not connect because there is no connection defined",
				getName());
		myConnMutex.unlock();
		failedToConnect();
		return false;
	}

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
	myReceiver.setInfoLogLevel(myInfoLogLevel);
	myReceiver.setName(getName());

	myReceiver.setDeviceConnection(myConn);
	myConnMutex.unlock();

	lockDevice();
	myTryingToConnect = true;
	unlockDevice();

	// PS 9/9/11 - moved up top
	//laserPullUnsetParamsFromRobot();
	//laserCheckParams();

	int size = ArMath::roundInt((270 / .25) + 1);
	ArLog::log(myInfoLogLevel,
			"%s::blockingConnect() Setting current buffer size to %d",
			getName(), size);
	setCurrentBufferSize(size);

	ArTime timeDone;
	if (myPowerControlled)
	{
	        // MPL 11/28/2012 making this timeout shorter
	        //if (!timeDone.addMSec(60 * 1000))
		if (!timeDone.addMSec(5 * 1000))
		{
			ArLog::log(ArLog::Verbose,
					"%s::blockingConnect() error adding msecs (60 * 1000)",
					getName());
		}
	}
	else
	{
	        // MPL 11/28/2012 making this timeout shorter 
	        //if (!timeDone.addMSec(30 * 1000))
		if (!timeDone.addMSec(5 * 1000))
		{
			ArLog::log(ArLog::Verbose,
					"%s::blockingConnect() error adding msecs (30 * 1000)",
					getName());
		}
	}

	ArS3SeriesPacket *packet;

	bool startMode = true;
	do
	{
		timeToRunFor = timeDone.mSecTo();
		if (timeToRunFor < 0)
			timeToRunFor = 0;

		if ((packet = myReceiver.receivePacket(1000, startMode)) != NULL)
		{
			ArLog::log(ArLog::Verbose, "%s: got packet", getName());
			// PS 10/17/12 - verify number of readings
			// PS - test for both S3000 (190 degrees) and S300 (270 degrees)
			if (packet->getNumReadings() != 381) {
				if (packet->getNumReadings() != 540) {
					ArLog::log(ArLog::Normal, "%s:blockingConnect - number of readings is invalid %d", getName(), packet->getNumReadings());
					delete packet;
					packet = NULL;
					continue;
				}
			}

			if (packet->getMonitoringDataAvailable()) {
				ArLog::log(ArLog::Normal, "%s: Monitoring data is available (0x%02x 0x%02x)", getName(), 
										packet->getMonitoringDataByte1(), packet->getMonitoringDataByte2());
			}
			else {
				ArLog::log(ArLog::Normal, "%s: Monitoring data is not available", getName());

			}

			ArLog::log(ArLog::Normal, "%s: Protocol version (0x%02x 0x%02x)", getName(), 
										packet->getProtocolVersionByte1(), packet->getProtocolVersionByte2());


			delete packet;
			packet = NULL;

			lockDevice();
			myIsConnected = true;
			myTryingToConnect = false;
			unlockDevice();
			ArLog::log(ArLog::Normal, "%s: Connected to laser", getName());
			laserConnect();
			return true;
		}
		else
		{
			packet = NULL;
			ArLog::log(ArLog::Verbose, "%s: MS left = %d", getName(), timeDone.mSecTo());
		}
		// this is only used for logging in receivePacket
		startMode = false;
	} while (timeDone.mSecTo() >= 0); // || !myStarting)

	ArLog::log(ArLog::Terse,
			"%s::blockingConnect()  Did not get scan data back from laser",
			getName());
	failedToConnect();
	return false;

}

AREXPORT void * ArS3Series::runThread(void *arg) {
	//char buf[1024];
	ArS3SeriesPacket *packet;

	bool safetyDebugging = false;
	
	ArTimeChecker checker;
	std::string checkerName = myName + "::safetyZone";
	checker.setName(checkerName.c_str());
	checker.setDefaultMSecs(200);

	checker.start();

while (getRunning() )
{
	lockDevice();
	
	if (myStartConnect) {
		myStartConnect = false;
		myTryingToConnect = true;
		unlockDevice();
		
		blockingConnect();
		
		lockDevice();
		myTryingToConnect = false;
		unlockDevice();
		checker.start();
		continue;
	}
	
	unlockDevice();
	
	if (!myIsConnected) {
		ArUtil::sleep (100);
		continue;
	}
	
	
	while (getRunning() && myIsConnected &&
	       (packet = myReceiver.receivePacket (500, false) ) != NULL) {

	        // MPL 2013_07_09 moved this from the process packet
	        // so that we don't trigger a safety warning if the
	        // cycle takes too long...  it's possible there should
	        // be some mutex around this monitoring data but it's
	        // chars/bools so hopefully it'll be OK TODO verify

	        // if the monitoring data is available - send it down to
	        // the firmware

	        if (packet->getMonitoringDataAvailable()) {

		  mySafetyDebuggingTimeMutex.lock();
		  if (mySafetyDebuggingTime.mSecSince() < 1000)
		    safetyDebugging = true;
		  else
		    safetyDebugging = false;
		  mySafetyDebuggingTimeMutex.unlock();


		  
		  myIsMonitoringDataAvailable = true;
		  if (!mySendFakeMonitoringData)
		    myMonitoringData = packet->getMonitoringDataByte1();
		  else
		    myMonitoringData = 0;
		  // MPL taking out the locking since the
		  // ArRobotPacketSender has it's own mutex to make
		  // sure we don't munge the packets

		  //myRobot->lock();
		  if (!mySendFakeMonitoringData)
		    myRobot->comInt(217, packet->getMonitoringDataByte1());  
		  else
		    myRobot->comInt(217, 0);

		  if (safetyDebugging)
		    ArLog::log(ArLog::Normal, 
			       "%s::SafetyDecommissionWarning: Laser reports zone %d, last sent %lld mSecAgo",
			       myName.c_str(), 
			       packet->getMonitoringDataByte1(),
			       checker.getLastCheckTime().mSecSinceLL());
		  
		  checker.finish();
		  checker.start();
		  /*
			ArLog::log(ArLog::Normal, "%s monitoring case of = %d seq = %d %d time = %d %d %d %d",
					getName(), packet->getMonitoringDataByte1(), packet->getTelegramNumByte1(), packet->getTelegramNumByte2(),
					packet->getTimeStampByte1(), packet->getTimeStampByte2(), packet->getTimeStampByte3(),
					packet->getTimeStampByte4());
		  ArLog::log(ArLog::Normal, "%s: Sent monitoring case of %d", 
			     getName(), packet->getMonitoringDataByte1());
		  */
		  //myRobot->unlock();
		}
		else {
		  myIsMonitoringDataAvailable = false;
		}

		myPacketsMutex.lock();
		myPackets.push_back (packet);
		myPacketsMutex.unlock();
		

		if (myRobot == NULL)
			sensorInterp();
	}
	
	// if we have a robot but it isn't running yet then don't have a
	// connection failure
	if (getRunning() && myIsConnected && laserCheckLostConnection() ) {
		ArLog::log (ArLog::Terse,
		            "%s::runThread()  Lost connection to the laser because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
		            myLastReading.mSecSince() / 1000.0,
		            getConnectionTimeoutSeconds() );
		myIsConnected = false;
		laserDisconnectOnError();
		continue;
	}
	
	/// MPL no sleep here so it'll get back into that while as soon as it can
	
	//ArUtil::sleep(1);
	//ArUtil::sleep(2000);
	//ArUtil::sleep(500);
	
#if 0 // PS 10/12/11 - fixing disconnects
	
	
	// PS 7/5/11 - change msWait from 50 to 5000
	
	// MPL 7/12/11 Changed mswait to 500 (which is bad enough,
	// especially since receive packet doesn't use it quite right at
	// this time)
	while (getRunning() && myIsConnected && (packet
	        = myReceiver.receivePacket (500, false) ) != NULL) {
	        
		myPacketsMutex.lock();
		myPackets.push_back (packet);
		myPacketsMutex.unlock();
		
		//ArLog::log(ArLog::Terse, "myRobot = %s",myRobot);
		
		//if (myRobot == NULL)
		//sensorInterp();
		
		/// MPL TODO see if this gets called if the laser goes
		/// away... it looks like it may not (since the receivePacket may just return nothing)
		
		// if we have a robot but it isn't running yet then don't have a
		// connection failure
		if (laserCheckLostConnection() ) {

			ArLog::log (ArLog::Terse,
			            "%s:  Lost connection to the laser because of error.  Nothing received for %g seconds (greater than the timeout of %g).",
			            getName(), myLastReading.mSecSince() / 1000.0,
			            getConnectionTimeoutSeconds() );
			myIsConnected = false;
			
			laserDisconnectOnError();
			continue;
		}
	}
	
	ArUtil::sleep (1);
	//ArUtil::sleep(2000);
	//ArUtil::sleep(500);
	
#endif // end PS 10/12/11
	
}
	return NULL;
}

AREXPORT bool ArS3Series::packetHandler(ArRobotPacket *packet)
{
  if (packet->getID() != 0xd9)
    return false;

  mySafetyDebuggingTimeMutex.lock();
  mySafetyDebuggingTime.setToNow();
  mySafetyDebuggingTimeMutex.unlock();

  // we return false anyways, so that we get more zones if we have
  // more than one laser
  return false;
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

unsigned short ArS3SeriesPacketReceiver::CRC16(unsigned char *Data, int length) {
	unsigned short CRC_16 = 0xFFFF;
	int i;
	for (i = 0; i < length; i++)
	{
		CRC_16 = (CRC_16 << 8) ^ (crc_table[(CRC_16 >> 8) ^ (Data[i])]);
	}
	return CRC_16;
}

