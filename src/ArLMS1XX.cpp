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
#include "ArLMS1XX.h"
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

AREXPORT ArLMS1XXPacket::ArLMS1XXPacket() : 
ArBasePacket(10000, 1, NULL, 1)
{
	myFirstAdd = true;
	myCommandType[0] = '\0';
	myCommandName[0] = '\0';
}

AREXPORT ArLMS1XXPacket::~ArLMS1XXPacket()
{

}

AREXPORT const char *ArLMS1XXPacket::getCommandType(void)
{
	return myCommandType;
}

AREXPORT const char *ArLMS1XXPacket::getCommandName(void)
{
	return myCommandName;
}


AREXPORT void ArLMS1XXPacket::finalizePacket(void)
{
	myBuf[0] = '\002';
	rawCharToBuf('\003');
	myBuf[myLength] = '\0';
}

AREXPORT void ArLMS1XXPacket::resetRead(void)
{
	myReadLength = 1;

	myCommandType[0] = '\0';
	myCommandName[0] = '\0';

	bufToStr(myCommandType, sizeof(myCommandType));
	bufToStr(myCommandName, sizeof(myCommandName));
}

AREXPORT ArTime ArLMS1XXPacket::getTimeReceived(void)
{
	return myTimeReceived;
}

AREXPORT void ArLMS1XXPacket::setTimeReceived(ArTime timeReceived)
{
	myTimeReceived = timeReceived;
}

AREXPORT void ArLMS1XXPacket::duplicatePacket(ArLMS1XXPacket *packet)
{
	myLength = packet->getLength();
	myReadLength = packet->getReadLength();
	myTimeReceived = packet->getTimeReceived();
	myFirstAdd = packet->myFirstAdd;
	strcpy(myCommandType, packet->myCommandType);
	strcpy(myCommandName, packet->myCommandName);
	memcpy(myBuf, packet->getBuf(), myLength);
}

AREXPORT void ArLMS1XXPacket::empty(void)
{
	myLength = 0;
	myReadLength = 0;
	myFirstAdd = false;
	myCommandType[0] = '\0';
	myCommandName[0] = '\0';
}


AREXPORT void ArLMS1XXPacket::byteToBuf(ArTypes::Byte val)
{
	char buf[1024];
	if (val > 0)
		sprintf(buf, "+%d", val);
	else
		sprintf(buf, "%d", val);
	strToBuf(buf);
}

AREXPORT void ArLMS1XXPacket::byte2ToBuf(ArTypes::Byte2 val)
{
	char buf[1024];
	if (val > 0)
		sprintf(buf, "+%d", val);
	else
		sprintf(buf, "%d", val);
	strToBuf(buf);
}

AREXPORT void ArLMS1XXPacket::byte4ToBuf(ArTypes::Byte4 val)
{
	char buf[1024];
	if (val > 0)
		sprintf(buf, "+%d", val);
	else
		sprintf(buf, "%d", val);
	strToBuf(buf);
}

AREXPORT void ArLMS1XXPacket::uByteToBuf(ArTypes::UByte val)
{
	char buf[1024];
	sprintf(buf, "%u", val);
	strToBuf(buf);
}

AREXPORT void ArLMS1XXPacket::uByte2ToBuf(ArTypes::UByte2 val)
{
	uByteToBuf(val & 0xff);
	uByteToBuf((val >> 8) & 0xff);
}

AREXPORT void ArLMS1XXPacket::uByte4ToBuf(ArTypes::UByte4 val)
{
	char buf[1024];
	sprintf(buf, "%u", val);
	strToBuf(buf);
}

AREXPORT void ArLMS1XXPacket::strToBuf(const char *str)
{
	if (str == NULL) {
		str = "";
	}

	if (!myFirstAdd && hasWriteCapacity(1))
	{
		myBuf[myLength] = ' ';
		myLength++;
	}

	myFirstAdd = false;

	ArTypes::UByte2 tempLen = strlen(str);

	if (!hasWriteCapacity(tempLen)) {
		return;
	}

	memcpy(myBuf+myLength, str, tempLen);
	myLength += tempLen;
}

AREXPORT ArTypes::Byte ArLMS1XXPacket::bufToByte(void)
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

AREXPORT ArTypes::Byte2 ArLMS1XXPacket::bufToByte2(void)
{
	ArTypes::Byte2 ret=0;

	if (!isNextGood(1))
		return 0;

	if (myBuf[myReadLength] == ' ')
		myReadLength++;

	if (!isNextGood(4))
		return 0;

	unsigned char n1, n2, n3, n4;
	n4 = deascii(myBuf[myReadLength+4]);
	n3 = deascii(myBuf[myReadLength+5]);
	n2 = deascii(myBuf[myReadLength+6]);
	n1 = deascii(myBuf[myReadLength+7]);
	ret = n4 << 12 | n3 << 8 | n2 << 4 | n1;

	myReadLength += 4;

	return ret;
}

AREXPORT ArTypes::Byte4 ArLMS1XXPacket::bufToByte4(void)
{
	ArTypes::Byte4 ret=0;

	if (!isNextGood(1))
		return 0;

	if (myBuf[myReadLength] == ' ')
		myReadLength++;

	if (!isNextGood(8))
		return 0;

	unsigned char n1, n2, n3, n4, n5, n6, n7, n8;
	n8 = deascii(myBuf[myReadLength]);
	n7 = deascii(myBuf[myReadLength+1]);
	n6 = deascii(myBuf[myReadLength+2]);
	n5 = deascii(myBuf[myReadLength+3]);
	n4 = deascii(myBuf[myReadLength+4]);
	n3 = deascii(myBuf[myReadLength+5]);
	n2 = deascii(myBuf[myReadLength+6]);
	n1 = deascii(myBuf[myReadLength+7]);
	ret = n8 << 28 | n7 << 24 | n6 << 20 | n5 << 16 | n4 << 12 | n3 << 8 | n2 << 4 | n1;

	myReadLength += 8;

	return ret;
}

AREXPORT ArTypes::UByte ArLMS1XXPacket::bufToUByte(void)
{
	ArTypes::UByte ret=0;
	if (!isNextGood(1))
		return 0;

	if (myBuf[myReadLength] == ' ')
		myReadLength++;

	std::string str;
	while (isNextGood(1) && myBuf[myReadLength] != ' ' &&
			myBuf[myReadLength] != '\003')
	{
		str += myBuf[myReadLength];
		myReadLength += 1;
	}

	ret = strtol(str.c_str(), NULL, 16);

	return ret;
}

AREXPORT ArTypes::UByte2 ArLMS1XXPacket::bufToUByte2(void)
{
	//printf("@ 1\n");

	ArTypes::UByte2 ret=0;

	if (!isNextGood(1))
		return 0;

	if (myBuf[myReadLength] == ' ')
		myReadLength++;

	//printf("@ 2 '%c' %d %d %d %d\n", myBuf[myReadLength], isNextGood(1),
	// myReadLength, myLength, myFooterLength);
	std::string str;
	while (isNextGood(1) && myBuf[myReadLength] != ' ' &&
			myBuf[myReadLength] != '\003')
	{
		//printf("%c\n", myBuf[myReadLength]);
		str += myBuf[myReadLength];
		myReadLength += 1;
	}


	ret = strtol(str.c_str(), NULL, 16);

	//printf("@ 3 %d\n", ret);
	return ret;
}

AREXPORT ArTypes::UByte4 ArLMS1XXPacket::bufToUByte4(void)
{
	ArTypes::Byte4 ret=0;

	if (!isNextGood(1))
		return 0;

	if (myBuf[myReadLength] == ' ')
		myReadLength++;

	std::string str;
	while (isNextGood(1) && myBuf[myReadLength] != ' ' &&
			myBuf[myReadLength] != '\003')
	{
		str += myBuf[myReadLength];
		myReadLength += 1;
	}


	ret = strtol(str.c_str(), NULL, 16);

	return ret;
}

/** 
Copy a string from the packet buffer into the given buffer, stopping when
the end of the packet buffer is reached, the given length is reached,
or a NUL character ('\\0') is reached.  If the given length is not large
enough, then the remainder of the string is flushed from the packet.
A NUL character ('\\0') is appended to @a buf if there is sufficient room
after copying the sting from the packet, otherwise no NUL is added (i.e.
if @a len bytes are copied).
@param buf Destination buffer
@param len Maximum number of characters to copy into the destination buffer
 */
AREXPORT void ArLMS1XXPacket::bufToStr(char *buf, int len)
{
	if (buf == NULL)
	{
		ArLog::log(ArLog::Normal, "ArLMS1XXPacket::bufToStr(NULL, %d) cannot write to null address",
				len);
		return;
	}
	int i;

	buf[0] = '\0';

	if (!isNextGood(1))
		return;

	if (myBuf[myReadLength] == ' ')
		myReadLength++;

	// see if we can read
	if (isNextGood(1))
	{
		// while we can read copy over those bytes
		for (i = 0;
				isNextGood(1) && i < (len - 1) && myBuf[myReadLength] != ' ' &&
						myBuf[myReadLength] != '\003';
				++myReadLength, ++i)
		{
			buf[i] = myBuf[myReadLength];
			buf[i+1] = '\0';
			//printf("%d %c %p\n", i);
		}
		if (i >= (len - 1))
		{
			// Otherwise, if we stopped reading because the output buffer was full,
			// then attempt to flush the rest of the string from the packet

			// This is a bit redundant with the code below, but wanted to log the
			// string for debugging
			myBuf[len - 1] = '\0';

			ArLog::log(ArLog::Normal, "ArLMS1XXPacket::bufToStr(buf, %d) output buf is not large enough for packet string %s",
					len, myBuf);

			while (isNextGood(1) && myBuf[myReadLength] != ' ' &&
					myBuf[myReadLength] != '\003') {
				myReadLength++;
			}
		} // end else if output buffer filled before null-terminator
	} // end if something to read

	// Make absolutely sure that the string is null-terminated...
	buf[len - 1] = '\0';
}

AREXPORT void ArLMS1XXPacket::rawCharToBuf(unsigned char c)
{
	if (!hasWriteCapacity(1)) {
		return;
	}
	myBuf[myLength] = c;
	//memcpy(myBuf+myLength, &c, 1);
	myLength += 1;
}

int ArLMS1XXPacket::deascii(char c)
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

AREXPORT ArLMS1XXPacketReceiver::ArLMS1XXPacketReceiver()
{
	myState = STARTING;
}

AREXPORT ArLMS1XXPacketReceiver::~ArLMS1XXPacketReceiver()
{

}

AREXPORT void ArLMS1XXPacketReceiver::setDeviceConnection(ArDeviceConnection *conn)
{
	myConn = conn;
}

AREXPORT ArDeviceConnection *ArLMS1XXPacketReceiver::getDeviceConnection(void)
{
	return myConn;
}


ArLMS1XXPacket *ArLMS1XXPacketReceiver::receivePacket(unsigned int msWait,
						      bool scandataShortcut,
						      bool ignoreRemainders)
{
	ArLMS1XXPacket *packet;
	unsigned char c;
	long timeToRunFor;
	ArTime timeDone;
	ArTime lastDataRead;
	ArTime packetReceived;
	int numRead;
	int i;

	//if (myLaserModel == ArLMS1XX::TiM3XX)
	//	return receiveTiMPacket(msWait, scandataShortcut, ignoreRemainders);

	if (myConn == NULL ||
			myConn->getStatus() != ArDeviceConnection::STATUS_OPEN)
	{
		return NULL;
	}

	timeDone.setToNow();
	if (!timeDone.addMSec(msWait)) {
		ArLog::log(ArLog::Terse,
				"%s::receivePacket() error adding msecs (%i)",
				myName,msWait);
	}

	do
	{
		timeToRunFor = timeDone.mSecTo();
		if (timeToRunFor < 0)
			timeToRunFor = 0;

		//printf("%x\n", c);
		if (myState == STARTING)
		{
			if ((numRead = myConn->read((char *)&c, 1, timeToRunFor)) <= 0) {

				//ArLog::log(ArLog::Terse,
				//			"%s::receivePacket() Timeout on initial read - read timeout = (%d)",
				//					myName, timeToRunFor);
				return NULL;

			}//printf("%x\n", c);

			if (c == '\002')
			{
				myState = DATA;
				myPacket.empty();
				myPacket.setLength(0);
				myPacket.rawCharToBuf(c);
				myReadCount = 0;
				packetReceived = myConn->getTimeRead(0);
				myPacket.setTimeReceived(packetReceived);
			}
			else
			{
        			char l = ' ';
       				if(c >= ' ' && c <= '~')
          				l = c;
				ArLog::log(ArLog::Verbose,
							"%s::receivePacket() Warning: Received invalid char during STARTING, looking for 0x02 got 0x%02x %c. Skipping.",
									myName, c, l);
			}
		}
		else if (myState == DATA)
		{
			numRead = myConn->read(&myReadBuf[myReadCount],
					sizeof(myReadBuf) - myReadCount, myReadTimeout);

			// trap if we failed the read
			if (numRead < 0)
			{
				//printf("read failed \n");
				ArLog::log(ArLog::Normal,
						"%s::receivePacket() Failed read (%d)",
						myName,numRead);
				myState = STARTING;
				return NULL;
			}
			/*
			ArLog::log(ArLog::Normal, "%s::receivePacket() Read %d bytes",
					myName,numRead);
			*/

            IFDEBUG(

			if (numRead != 0)
			{

				// print out using logging
				int i;
				char x[100000];
				x[0] = '\0';
				int idx=0;
				strcat(&x[idx],"<STX>");
				idx = idx+5;
				for (i = 0;i < numRead;i++)
				{
					//
					if (myReadBuf[i] == '\002')
					{
						strcat(&x[idx], "<STX>");
						idx = idx+5;
					}
					else if (myReadBuf[i] == '\003')
					{
						strcat(&x[idx], "<ETX>");
						idx = idx+5;
					}
					else
						sprintf(&x[idx++], "%c",(char *)myReadBuf[i]);
				}

					ArLog::log(ArLog::Normal,
							"%s::receivePacket() Buffer with %d bytes = %s", myName, numRead, x);
			}
             ); // end IFDEBUG

			// see if we found the end of the packet
			for (i = myReadCount; i < myReadCount + numRead; i++)
			{
				if (myReadBuf[i] == '\002')
				{
//					ArLog::log(myInfoLogLevel, "%s::receivePacket() Data found start of new packet...",
					ArLog::log(myInfoLogLevel, "%s::receivePacket() Data found start of new packet...",
							myName);
					myPacket.empty();
					myPacket.setLength(0);
					memmove(myReadBuf, &myReadBuf[i], myReadCount + numRead - i);
					numRead -= (i - myReadCount);
					myReadCount -= i;
					i = 0;
					continue;
				}
				if (myReadBuf[i] == '\003')
				{

					myPacket.dataToBuf(myReadBuf, i + 1);
					myPacket.resetRead();
					packet = new ArLMS1XXPacket;
					packet->duplicatePacket(&myPacket);
					myPacket.empty();
					myPacket.setLength(0);

					// if it's the end of the data just go back to the beginning
					//printf("i=%d, myReadCount = %d, numRead = %d\n",i, myReadCount, numRead);

					if (i == myReadCount + numRead - 1)
					{
						//ArLog::log(myLogLevel1XXPacketReceiver: Starting again");
						myState = STARTING;
					}
					// if it isn't move the data up and start again
					else
					{
					  if (!ignoreRemainders)
					  {
					    memmove(myReadBuf, &myReadBuf[i+1], myReadCount + numRead - i - 1);
					    myReadCount = myReadCount + numRead - i - 1;
					    myState = REMAINDER;
					    ArLog::log(myInfoLogLevel, "%s::receivePacket() Got remainder, %d bytes beyond one packet ...",
						       myName,myReadCount);
					  }
					  else
					  {
					    myState = STARTING;
					    ArLog::log(myInfoLogLevel, "%s::receivePacket() Got remainder, %d bytes beyond one packet ... ignoring it",
						       myName,myReadCount);
					  }
					}
					return packet;
				}
			}
			//printf("didn't get enough bytes\n");
			myReadCount += numRead;
			if (numRead != 0)
				ArLog::log(myInfoLogLevel, "%s::receivePacket() Got %d bytes (but not end char), up to %d",
						myName, numRead, myReadCount);
		}
		else if (myState == REMAINDER)
		{
			//printf("In remainder ('%c' %d) \n", myReadBuf[0], myReadBuf[0]);
			if (myReadBuf[0] != '\002')
			{
				ArLog::log(myInfoLogLevel,
						"%s::receivePacket() Remainder didn't start with \\002, starting over...",myName);
				myState = STARTING;
				continue;
			}

			// PS - 9/1/11 not sure why, but for LMS500 this is off by 1 and it dosn't see the <EXT>
			// and thinks the last
			// packet does not have the last byte
			// so for the LMS500 - just use one less byte in the for loop

			int loopcount;

			switch (myLaserModel) {

				case ArLMS1XX::LMS1XX:
					loopcount = myReadCount - 1;
				break;

        default:
					loopcount = myReadCount;
				break;
			
			} // end switch laserModel


//			if (myIsLMS5XX)
//				loopcount = myReadCount;
//			else
//				loopcount = myReadCount - 1;

			for (i = 0; i < loopcount; i++)
			//for (i = 0; i < myReadCount - 1; i++)
			{
				//printf("%03d '%c' %d\n", i, myReadBuf[i], myReadBuf[i]);
				if (myReadBuf[i] == '\002' && i != 0)
				{
					ArLog::log(myInfoLogLevel, "%s::receivePacket() Remainder found start of new packet...",
							myName, myReadCount);
					myPacket.empty();
					myPacket.setLength(0);
					memmove(myReadBuf, &myReadBuf[i], myReadCount + i);
					myReadCount -= i;
					i = 0;
					continue;
				}
				if (myReadBuf[i] == '\003')
				{
					myPacket.dataToBuf(myReadBuf, i + 1);
					myPacket.resetRead();
					packet = new ArLMS1XXPacket;
					packet->duplicatePacket(&myPacket);
					myPacket.empty();
					myPacket.setLength(0);

					// if it's the end of the data just go back to the beginning
					if (i == myReadCount - 1)
					{
						myState = STARTING;
						ArLog::log(myInfoLogLevel,
								"%s::receivePacket() Remainder was one packet...",myName);
					}
					// if it isn't move the data up and start again
					else
					{
						// PS - took out this printf and replaced with a log
						if (myReadCount - i < 50)
							//printf("read buf (%d %d) %s\n", myReadCount, i, myReadBuf);
						   ArLog::log(ArLog::Terse, "%s::receivePacket() read buf (%d %d) %s",
								   myName, myReadCount, i, myReadBuf);

						memmove(myReadBuf, &myReadBuf[i+1], myReadCount - i);
						myReadCount = myReadCount - i - 1;
						myState = REMAINDER;
						ArLog::log(myInfoLogLevel,
								"%s::receivePacket() Remainder was more than one packet... (%d %d)", myName, myReadCount, i);
					}
					return packet;
				}
			}
			// if we didn't find the end of the packet, then get the rest of the data
			myState = DATA;
			ArLog::log(myInfoLogLevel,
					"%s::receivePacket() Remainder didn't contain a whole packet...",myName);

			continue;
		}
		else
		{
			ArLog::log(ArLog::Terse, "%s::receivePacket() Bad state (%d)",
					myName,myState);
			myState = STARTING;
		}
	} while (timeDone.mSecTo() >= 0); // || !myStarting)

	return NULL;
}


ArLMS1XXPacket *ArLMS1XXPacketReceiver::receiveTiMPacket(unsigned int msWait,
						      bool scandataShortcut,
						      bool ignoreRemainders)
{
	ArLMS1XXPacket *packet;
	unsigned char c;
	long timeToRunFor;
	ArTime timeDone;
	ArTime lastDataRead;
	ArTime packetReceived;
	int numRead;
	int i;

	if (myConn == NULL ||
			myConn->getStatus() != ArDeviceConnection::STATUS_OPEN)
	{
		return NULL;
	}

	timeDone.setToNow();
	if (!timeDone.addMSec(msWait)) {
		ArLog::log(ArLog::Terse,
				"%s::receiveTiMPacket() error adding msecs (%i)",
				myName,msWait);
	}

	do
	{
		timeToRunFor = timeDone.mSecTo();
		if (timeToRunFor < 0)
			timeToRunFor = 0;

		//printf("%x\n", c);
		if (myState == STARTING)
		{
			if ((numRead = myConn->read((char *)&c, 1, timeToRunFor)) <= 0) {

				return NULL;

				}//printf("%x\n", c);
			if (c == '\002')
			{
				myState = DATA;
				myPacket.empty();
				myPacket.setLength(0);
				myPacket.rawCharToBuf(c);
				myReadCount = 0;
				packetReceived = myConn->getTimeRead(0);
				myPacket.setTimeReceived(packetReceived);
			}
			else
			{
				ArLog::log(ArLog::Terse,
						"%s::receiveTiMPacket() Failed single char read (%d) %02x %c",
						myName,numRead, c, c);
			}
		}
		else if (myState == DATA)
		{
			numRead = myConn->read(&myReadBuf[myReadCount],
					sizeof(myReadBuf) - myReadCount, myReadTimeout);

			// trap if we failed the read
			if (numRead < 0)
			{
				//printf("read failed \n");
				ArLog::log(ArLog::Normal,
						"%s::receivePacket() Failed read (%d)",
						myName,numRead);
				myState = STARTING;
				return NULL;
			}
			/*
			ArLog::log(ArLog::Normal, "%s::receivePacket() Read %d bytes",
					myName,numRead);
			*/

            IFDEBUG(

			if (numRead != 0)
			{

				// print out using logging
				int i;
				char x[100000];
				x[0] = '\0';
				int idx=0;
				strcat(&x[idx],"<STX>");
				idx = idx+5;
				for (i = 0;i < numRead;i++)
				{
					//
					if (myReadBuf[i] == '\002')
					{
						strcat(&x[idx], "<STX>");
						idx = idx+5;
					}
					else if (myReadBuf[i] == '\003')
					{
						strcat(&x[idx], "<ETX>");
						idx = idx+5;
					}
					else
						sprintf(&x[idx++], "%c",(char *)myReadBuf[i]);
				}

					ArLog::log(ArLog::Normal,
							"%s::receivePacket() Buffer with %d bytes = %s", myName, numRead, x);
			}
             ); // end IFDEBUG

			// see if we found the end of the packet
			for (i = myReadCount; i < myReadCount + numRead; i++)
			{
				if (myReadBuf[i] == '\002')
				{
//					ArLog::log(myInfoLogLevel, "%s::receivePacket() Data found start of new packet...",
					ArLog::log(myInfoLogLevel, "%s::receivePacket() Data found start of new packet...",
							myName);
					myPacket.empty();
					myPacket.setLength(0);
					memmove(myReadBuf, &myReadBuf[i], myReadCount + numRead - i);
					numRead -= (i - myReadCount);
					myReadCount -= i;
					i = 0;
					continue;
				}
				if (myReadBuf[i] == '\003')
				{

					myPacket.dataToBuf(myReadBuf, i + 1);
					myPacket.resetRead();
					packet = new ArLMS1XXPacket;
					packet->duplicatePacket(&myPacket);
					myPacket.empty();
					myPacket.setLength(0);

					// if it's the end of the data just go back to the beginning
					//printf("i=%d, myReadCount = %d, numRead = %d\n",i, myReadCount, numRead);

					ArLog::log(myInfoLogLevel, "%s::receiveTiMPacket() i=%d, myReadCount = %d, numRead = %d",
									myName, i, myReadCount, numRead);

					if (i == myReadCount + numRead - 1)
					{
						ArLog::log(myInfoLogLevel, "%s::receiveTiMPacket() Starting again", myName);
						myState = STARTING;
					}
					// if it isn't move the data up and start again
					else
					{
					    myState = STARTING;
					    ArLog::log(myInfoLogLevel, "%s::receivePacket() Got remainder, %d bytes beyond one packet ... ignoring it",
						       myName,myReadCount);
					}
					return packet;
				}
			}
			//printf("didn't get enough bytes\n");
			myReadCount += numRead;
			if (numRead != 0)
				ArLog::log(myInfoLogLevel, "%s::receivePacket() Got %d bytes (but not end char), up to %d",
						myName, numRead, myReadCount);
		}
		else
		{
			ArLog::log(ArLog::Terse, "%s::receivePacket() Bad state (%d)",
					myName,myState);
			myState = STARTING;
		}
	} while (timeDone.mSecTo() >= 0); // || !myStarting)

	return NULL;
}


AREXPORT ArLMS1XX::ArLMS1XX(int laserNumber,
		const char *name, LaserModel laserModel) :
		ArLaser(laserNumber, name, 20000),
		mySensorInterpTask(this, &ArLMS1XX::sensorInterp),
		myAriaExitCB(this, &ArLMS1XX::disconnect)
{

	myLaserModel = laserModel;

  if(myLaserModel == LMS1XX || myLaserModel == LMS5XX)
    myLaserModelFamily = LMS;
  else
    myLaserModelFamily = TiM;

	clear();
	myRawReadings = new std::list<ArSensorReading *>;

	Aria::addExitCallback(&myAriaExitCB, -10);

	setInfoLogLevel(ArLog::Normal);

	laserSetName(getName());

	laserAllowSetPowerControlled(false);

  switch (myLaserModel) {

    case ArLMS1XX::LMS1XX:
    case ArLMS1XX::TiM3XX:
    case TiM551:
    case TiM561:
    case TiM571:
      // LMS1xx and all TiMs have 270 deg max FoV
      laserAllowSetDegrees(-135, -135, 135, 	// default, min, max for start degrees
         135, -135, 135); 		// default, min, max for end degrees

      break;

    case ArLMS1XX::LMS5XX:
      // LMS500 has 190 deg max FOV
      laserAllowSetDegrees(-95, -95, 95, 95, -95, 95);
      break;


  } // end switch laserModel



	std::map<std::string, double> incrementChoices;

	// According to documentation, LMS1xx allows half or quarter degree resolution, and
	// LMS5xx allows half, quarter and also one degree resolution. One was the default for the 
	// LMS2xx, so let's keep that for LMS5xx. Applications can still set it to 
  // half or quarter instead, same as they used to with the 2xx.

  switch (myLaserModel) {

	// not sure if we need to do this for TiM??
  case ArLMS1XX::LMS1XX:
    incrementChoices["half"] = .5;
    incrementChoices["quarter"] = .25;
		laserAllowIncrementChoices("half", incrementChoices);
    break;

  case ArLMS1XX::LMS5XX:
    incrementChoices["half"] = .5;
    incrementChoices["quarter"] = .25;
		incrementChoices["one"] = 1;
		laserAllowIncrementChoices("one", incrementChoices);
		break;

  case ArLMS1XX::TiM3XX:
		incrementChoices["three"] = 3;
		laserAllowIncrementChoices("three", incrementChoices);
		break;

  case TiM551:
		incrementChoices["one"] = 1;
		//incrementChoices["three"] = 3;
		laserAllowIncrementChoices("one", incrementChoices);
		break;

  case TiM561:
  case TiM571:
		//incrementChoices["one"] = 1;
		//incrementChoices["three"] = 3;
		incrementChoices["third"] = 0.333333;
		laserAllowIncrementChoices("third", incrementChoices);
		break;
  } // end switch laser model


	//if (myIsLMS5XX)
	//{
	//	incrementChoices["one"] = 1;
	//	laserAllowIncrementChoices("one", incrementChoices);
	//}
	//else
	//{
	//	laserAllowIncrementChoices("half", incrementChoices);
	//}

	std::list<std::string> reflectorBitsChoices;
	reflectorBitsChoices.push_back("none");
	reflectorBitsChoices.push_back("8bits");
	// 16 bits isn't implemented yet
	//reflectorBitsChoices.push_back("16bits");
	laserAllowReflectorBitsChoices("none", reflectorBitsChoices);


  // max ranges
  switch (myLaserModel) 
  {
    case LMS1XX:
      laserSetAbsoluteMaxRange(50000);
      break;

    case LMS5XX:
      laserSetAbsoluteMaxRange(80000);
      break;

    case TiM3XX:
      laserSetAbsoluteMaxRange(4000);
      break;

    case TiM551:
    case TiM561:
      laserSetAbsoluteMaxRange(10000);
      break;

    case TiM571:
      laserSetAbsoluteMaxRange(25000);
      break;
  }

	std::list<std::string> baudChoices;

  switch (myLaserModel) {

	// not sure if we need to do this for TiM??
  case ArLMS1XX::TiM3XX:
		laserSetDefaultPortType("serial");
		baudChoices.push_back("115200");
		laserAllowStartingBaudChoices("115200", baudChoices);
    break;

  case ArLMS1XX::LMS1XX:
  case ArLMS1XX::LMS5XX:
  case TiM551:
  case TiM561:
  case TiM571:
    // default to tcp only
		laserSetDefaultPortType("tcp");
		break;

  } // end switch laser model

  // default TCP port for all types, though tim310/510 default connection type is
  // serial, only used for tim if manually set to tcp.
  laserSetDefaultTcpPort(2111);

	// PS = add new field for scan freq
//	myScanFreq = 5;

  // Log level for debugging and other detailed informational messages. Warnings
  // and key events are still logged at Normal/Terse as appropriate.
	myLogLevel = ArLog::Verbose;
	//myLogLevel = ArLog::Normal;

	setMinDistBetweenCurrent(0);
	setMaxDistToKeepCumulative(4000);
	setMinDistBetweenCumulative(200);
	setMaxSecondsToKeepCumulative(30);
	setMaxInsertDistCumulative(3000);

	setCumulativeCleanDist(75);
	setCumulativeCleanInterval(1000);
	setCumulativeCleanOffset(600);

	resetLastCumulativeCleanTime();

  switch (myLaserModel) {

    break;

  case ArLMS1XX::LMS5XX:
		setCurrentDrawingData(
				new ArDrawingData("polyDots",
						ArColor(255, 20, 147),
						80,  // mm diameter of dots
						75), // layer above sonar
						true);

		setCumulativeDrawingData(
				new ArDrawingData("polyDots",
						ArColor(128, 128, 0),
						100, // mm diameter of dots
						60), // layer below current range devices
						true);

		break;

  default:
		// need to make different default dots for TiMs, 1xx, etc.
		setCurrentDrawingData(
				new ArDrawingData("polyDots",
						ArColor(0, 0, 255),
						80,  // mm diameter of dots
						75), // layer above sonar
						true);

		setCumulativeDrawingData(
				new ArDrawingData("polyDots",
						ArColor(125, 125, 125),
						100, // mm diameter of dots
						60), // layer below current range devices
						true);

  } // end switch laserModel

}

AREXPORT ArLMS1XX::~ArLMS1XX()
{
	Aria::remExitCallback(&myAriaExitCB);
	if (myRobot != NULL)
	{
		myRobot->remRangeDevice(this);
		myRobot->remLaser(this);
		myRobot->remSensorInterpTask(&mySensorInterpTask);
	}
	if (myRawReadings != NULL)
	{
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

void ArLMS1XX::clear(void)
{
	myIsConnected = false;
	myTryingToConnect = false;
	myStartConnect = false;

	myVersionNumber = 0;
	myDeviceNumber = 0;
	mySerialNumber = 0;
	myDeviceStatus1 = 0;
	myDeviceStatus2 = 0;
	myMessageCounter = 0;
	myScanCounter = 0;
	myPowerUpDuration = 0;
	myTransmissionDuration = 0;
	myInputStatus1 = 0;
	myInputStatus2 = 0;
	myOutputStatus1 = 0;
	myOutputStatus2 = 0;
	myReserved = 0;
	myScanningFreq = 0;
	myMeasurementFreq = 0;
	myNumberEncoders = 0;
	myNumChans16Bit = 0;
	myNumChans8Bit = 0;
	myFirstReadings = true;
}

AREXPORT void ArLMS1XX::laserSetName(const char *name)
{
	myName = name;

	myConnMutex.setLogNameVar("%s::myConnMutex", getName());
	myPacketsMutex.setLogNameVar("%s::myPacketsMutex", getName());
	myDataMutex.setLogNameVar("%s::myDataMutex", getName());
	myAriaExitCB.setNameVar("%s::exitCallback", getName());

	ArLaser::laserSetName(getName());
}

AREXPORT void ArLMS1XX::setRobot(ArRobot *robot)
{
	myRobot = robot;

	if (myRobot != NULL)
	{
		myRobot->remSensorInterpTask(&mySensorInterpTask);
    myRobot->addSensorInterpTask(getName(), 90, &mySensorInterpTask);

		//if (myIsLMS5XX)
		//	myRobot->addSensorInterpTask("lms5XX", 90, &mySensorInterpTask);
		//else
		//	myRobot->addSensorInterpTask("lms1XX", 90, &mySensorInterpTask);
	}
	ArLaser::setRobot(robot);
}



AREXPORT bool ArLMS1XX::asyncConnect(void)
{
	myStartConnect = true;
	if (!getRunning())
		runAsync();
	return true;
}

AREXPORT bool ArLMS1XX::disconnect(void)
{
	if (!isConnected())
		return true;

	ArLog::log(ArLog::Normal, "%s: Disconnecting", getName());

	laserDisconnectNormally();
	return true;
}

void ArLMS1XX::failedToConnect(void)
{
	lockDevice();
	myTryingToConnect = true;
	unlockDevice();
	laserFailedConnect();
}

void ArLMS1XX::sensorInterp (void)
{
	ArLMS1XXPacket *packet;
	ArTime startTime;
	bool printing = false;
	//bool printing = true;

	while (1) {
		//ArTime packetMutexTime;
		myPacketsMutex.lock();
		//ArLog::log(ArLog::Normal, "%s: lock took = %ld",
		//getName(), packetMutexTime.mSecSince());

		if (myPackets.empty()) {
			myPacketsMutex.unlock();
			/*
			if (printing)
			ArLog::log(ArLog::Normal, "%s: Function took = %ld",
			 getName(), startTime.mSecSince());
			*/
			return;
		}

		// save some time by only processing the most recent packet
		//packet = myPackets.front();
		//myPackets.pop_front();
		// this'll still process two packets if there's another one while
		// the first is processing, but that's fine
		packet = myPackets.back();
		myPackets.pop_back();
		ArUtil::deleteSet (myPackets.begin(), myPackets.end());
		myPackets.clear();
		myPacketsMutex.unlock();
		// if its not a reading packet just skip it


		if (strcasecmp (packet->getCommandName(), "LMDscandata") != 0) {
			delete packet;
			continue;
		}

		//set up the times and poses
		ArTime packetRecvTime;
		/*
		// PS 9/1/11 - put this down below and use received scan freq to caculate
		// this value should be found more empirically... but we used 1/75
		//hz for the lms2xx and it was fine, so here we'll use 1/50 hz for now
		// MPL 9/26/11 moved the rest of it below since we must correct the time before finding the pose
		if (!time.addMSec(-20)) {
		ArLog::log(ArLog::Normal,
		"%s::sensorInterp() error adding msecs (-20)",getName());
		}
		if (myRobot == NULL || !myRobot->isConnected())
		{
		pose.setPose(0, 0, 0);
		encoderPose.setPose(0, 0, 0);
		}
		else if ((ret = myRobot->getPoseInterpPosition(time, &pose)) < 0 ||
		(retEncoder =
		myRobot->getEncoderPoseInterpPosition(time, &encoderPose)) < 0)
		{
		ArLog::log(ArLog::Normal, "%s::sensorInterp() reading too old to process", getName());
		delete packet;
		continue;
		}
		ArTransform transform;
		transform.setTransform(pose);
		*/
		unsigned int counter = 0;
		if (myRobot != NULL)
			counter = myRobot->getCounter();
		lockDevice();
		myDataMutex.lock();
		int i;
		int dist;
		int refl;
		//int onStep;
		ArSensorReading *reading;
		// read the extra stuff
		myVersionNumber = packet->bufToUByte2(); // first value after LMDscandata
		myDeviceNumber = packet->bufToUByte2();
		mySerialNumber = packet->bufToUByte4();
		myDeviceStatus1 = packet->bufToUByte();
		myDeviceStatus2 = packet->bufToUByte();
		myMessageCounter = packet->bufToUByte2();
		myScanCounter = packet->bufToUByte2();
		myPowerUpDuration = packet->bufToUByte4();
		myTransmissionDuration = packet->bufToUByte4();
		//printf("time values = %d %d\n",myPowerUpDuration, myTransmissionDuration);
		//printf("scan values = %d %d\n",myMessageCounter, myScanCounter);
		myInputStatus1 = packet->bufToUByte();
		myInputStatus2 = packet->bufToUByte();
		myOutputStatus1 = packet->bufToUByte();
		myOutputStatus2 = packet->bufToUByte();

		// myReserved is checksum on TiM
		myReserved = packet->bufToUByte2(); // 14th value after LMDscandata

		myScanningFreq = packet->bufToUByte4();
		myMeasurementFreq = packet->bufToUByte4();


		if (myDeviceStatus1 != 0 || myDeviceStatus2 != 0)
			ArLog::log (myLogLevel, "%s::sensorInterp() DeviceStatus %d %d",
			            getName(), myDeviceStatus1, myDeviceStatus2);


    // Do only TiMs have checksums?
/*
		if (myLaserModelFamily == TiM) {
			if (!validateCheckSum(packet)) {
				ArLog::log (ArLog::Normal, "%s: Warning: Bad checksum... skipping this packet",
				            getName());

				delete packet;
				unlockDevice();
				myDataMutex.unlock();
				continue;
			}
		}
*/

		ArTime time = packet->getTimeReceived();
		ArPose pose;
		int ret;
		int retEncoder;
		ArPose encoderPose;

		// PS 9/1/11 - cacl freq from input

		// TiM scan freq is 0 in TiM packet - use measurementfreq

    // XXX TODO check that this is correct
    int freq = 0;
		if (myLaserModelFamily == TiM)
    {
			myScanningFreq = myMeasurementFreq;
      freq = 0;
    }
    else
    {
      freq = 1000.0/ ((double)myScanningFreq/100.0);
    }

		//ArLog::log(ArLog::Normal,
		//	"%s::sensorInterp() freq = %d inputted freq = %d",getName(),freq,myScanningFreq);

		if (!time.addMSec (-freq)) {
			ArLog::log (ArLog::Normal,
			            "%s::sensorInterp() error adding msecs (-%d)",getName(), freq);
		}

		if (myRobot == NULL || !myRobot->isConnected()) {
			pose.setPose (0, 0, 0);
			encoderPose.setPose (0, 0, 0);
		} else if ( (ret = myRobot->getPoseInterpPosition (time, &pose)) < 0 ||
		            (retEncoder =
		               myRobot->getEncoderPoseInterpPosition (time, &encoderPose)) < 0) {
			ArLog::log (ArLog::Normal, "%s::sensorInterp() reading too old to process", getName());
			delete packet;
			unlockDevice();
			myDataMutex.unlock();
			continue;
		}

		ArTransform transform;
		transform.setTransform (pose);
		/*
		  printf("Received: %s %s ver %d devNum %d serNum %d scan %d sf %d mf %d\n",
		  packet->getCommandType(), packet->getCommandName(),
		  myVersionNumber, myDeviceNumber,
		  mySerialNumber, myScanCounter, myScanningFreq, myMeasurementFreq);
		*/
		myNumberEncoders = packet->bufToUByte2(); // 17th value after LMDscandata
		//printf("\tencoders %d\n", myNumberEncoders);

		if (myNumberEncoders > 0)
			ArLog::log (myLogLevel, "%s::sensorInterp() Encoders %d", getName(), myNumberEncoders);

		for (i = 0; i < myNumberEncoders; i++) {
			packet->bufToUByte4();
			packet->bufToUByte2();
			//printf("\t\t%d\t%d %d\n", i, eachEncoderPosition, eachEncoderSpeed);
		}


		myNumChans16Bit = packet->bufToUByte2(); // 18th value after LMDscanadat if NumberEncoders=0

		// for TiM310/510 for what ever reason 2nd byte is also num 16 bit chans
		// despite COLA protocol documentation:
		if (myLaserModel == ArLMS1XX::TiM3XX)
			myNumChans16Bit = packet->bufToUByte2(); // 19th value after LMDscandata if NumberEncoders=0 and its a TiM


		if (myNumChans16Bit > 1)
			ArLog::log (myLogLevel, "%s::sensorInterp() NumChans16Bit %d", getName(), myNumChans16Bit);

		char eachChanMeasured[1024];
		int eachScalingFactor;
		int eachScalingOffset;
		double eachStartingAngle;
		double eachAngularStepWidth;
		int eachNumberData;
		std::list<ArSensorReading *>::iterator it;
		double atDeg; // angle of reading transformed according to sensorPoseTh parameter
    double atDegLocal = 0; // angle of reading local to laser
		int onReading;
		double start = 0;
    double startLocal = 0;
		double increment = 0;
		bool startedProcessing = false;

		for (i = 0; i < myNumChans16Bit; i++) {
			bool measuringDistance = false;
			bool measuringReflectance = false;
			eachChanMeasured[0] = '\0';
			packet->bufToStr (eachChanMeasured, sizeof (eachChanMeasured));
			if (strcasecmp (eachChanMeasured, "DIST1") == 0)
				measuringDistance = true;
			else if (strcasecmp (eachChanMeasured, "RSSI1") == 0)
				measuringReflectance = true;
			// if this isn't the data we want then skip it
			if (!measuringDistance && !measuringReflectance) {
				delete packet;
				unlockDevice();
				myDataMutex.unlock();
				ArLog::log (ArLog::Normal,
				            "%s::sensorInterp(): Could not process channel %s",
				            getName(), eachChanMeasured);
				continue;
			}
			if (printing)
				ArLog::log (ArLog::Normal, "%s: Processing 16bit %s (%d %d)",
				            getName(), eachChanMeasured,
				            measuringDistance, measuringReflectance);

			// for LMS5XX Scaling Factor is a real number
      if(myLaserModel == LMS5XX)
				eachScalingFactor = packet->bufToByte4(); // FIX should be real
      else
				eachScalingFactor = packet->bufToUByte4(); // FIX should be real

			eachScalingOffset = packet->bufToUByte4(); // FIX should be real
			eachStartingAngle = packet->bufToByte4() / 10000.0;
			eachAngularStepWidth = packet->bufToUByte2() / 10000.0; // angular distance between each reading
			eachNumberData = packet->bufToUByte2(); // number of readings in this set
			/*
			ArLog::log(ArLog::Terse, "%s: %s start %.1f step %.2f numReadings %d",
			getName(), eachChanMeasured,
			eachStartingAngle, eachAngularStepWidth, eachNumberData);
			*/
			/*
			printf("\t\t%s\tscl %d %d ang %g %g num %d\n",
			eachChanMeasured,
			eachScalingFactor, eachScalingOffset,
			eachStartingAngle, eachAngularStepWidth,
			eachNumberData);
			*/
			// If we don't have any sensor readings created at all, make 'em all
			if (myRawReadings->size() == 0)
				for (i = 0; i < eachNumberData; i++)
					myRawReadings->push_back (new ArSensorReading);

			if (eachNumberData > myRawReadings->size()) {
				ArLog::log (ArLog::Terse, "%s::sensorInterp() Bad data, in theory have %d readings but can only have %d... skipping this packet\n",
				            getName(), myRawReadings->size(), eachNumberData);
				//printf("%s\n", packet->getBuf());
				delete packet;
				unlockDevice();
				myDataMutex.unlock();
				continue;
			}

      // TODO? Move some of the logic below regarding projecting from rays to
      // cartesian points based on sensor position, flipped, etc. to ArLaser,
      // ArRangeDevice or ArSensorReading so other laser and sensor classes can
      // use it?

      // first iteration:
			if (!startedProcessing) {
        startLocal = -1 * ((eachNumberData - 1) * eachAngularStepWidth) / 2.0;
				if (getFlipped()) {
					// original from LMS100, but this seems to have some problems
					//start = mySensorPose.getTh() + eachStartingAngle - 90.0 + eachAngularStepWidth * eachNumberData;
					// so we're trying this new algorithm which should be less dependent on SICK's protocol
					//start = mySensorPose.getTh() + ( (eachNumberData - 1) * eachAngularStepWidth) / 2.0;
					// but this might have had problems pointing backwards (found on 3/16/2015)				
          start = ArMath::addAngle(mySensorPose.getTh(), ( (eachNumberData - 1) * eachAngularStepWidth) / 2.0);
					increment = -eachAngularStepWidth;
				} else {
					// original from LMS100, but this seems to have some problems
					//start = mySensorPose.getTh() + eachStartingAngle - 90.0;
					// so we're trying this new algorithm which should be less dependent on SICK's protocol...
				  // but this might have had problems pointing backwards (found on 3/16/2015)
				  //start = mySensorPose.getTh() - ( (eachNumberData - 1) * eachAngularStepWidth) / 2.0;
				  // so we're trying this one
				  start = ArMath::subAngle(mySensorPose.getTh(), ( (eachNumberData - 1) * eachAngularStepWidth) / 2.0);
					increment = eachAngularStepWidth;
					/*
						ArLog::log(ArLog::Normal,
						"! start %g sensorPose %g eachstartingangle %g eachangularstepwidth %g eachNumberData %d",
						start, mySensorPose.getTh(), eachStartingAngle, eachAngularStepWidth, eachNumberData);
					*/
				}
			}
			startedProcessing = true;
			bool ignore;

			for (atDeg = start,
           atDegLocal = startLocal,
			     it = myRawReadings->begin(),
			     onReading = 0;
			     onReading < eachNumberData;
			     // MPL trying to fix bug with negative readings
			     //atDeg += increment,
			     atDeg = ArMath::addAngle(atDeg, increment),
           atDegLocal = ArMath::addAngle(atDegLocal, eachAngularStepWidth),
			     it++,
			     onReading++) {

				ignore = false;

        reading = (*it);

				if (myFirstReadings)
					reading->resetSensorPosition (ArMath::roundInt (mySensorPose.getX()),
					                              ArMath::roundInt (mySensorPose.getY()),
					                              atDeg);

        // was configured to have restricted fov, set ignore flag. (Move to ArLaser or other shared class?)
        if (    (canSetDegrees()    && (atDegLocal < getStartDegrees()             || atDegLocal > getEndDegrees()))
             || (canChooseDegrees() && (atDegLocal < -getDegreesChoiceDouble()/2.0 || atDegLocal > getDegreesChoiceDouble()/2.0))
        )
        {
          ignore = true;
        }
        //fprintf(stderr, "onReading=%d (n=%d), atDeg=%f atDegLocal=%f, (canSetDegrees=%d, startDeg=%f, endDeg=%f, increment=%f, eachAngularStepWidth=%f) => ignore=%d\n", onReading, eachNumberData, atDeg, atDegLocal, canSetDegrees(), getStartDegrees(), getEndDegrees(), increment, eachAngularStepWidth, ignore); 

        // calculate either obstacle position data or reflectance value data
        // and update the ArSensorReading reading with the new data
				if (measuringDistance)  
        {
					dist = packet->bufToUByte2();
					// this was the original code, that just ignored 0s as a
					// reading... however sometimes the sensor reports very close
					// distances for rays it gets no return on... Sick wasn't very
					// helpful figuring out what values it will report for
					// those... so this is changing to a check that is basically
					// if it reports as well within the sensor itself it gets
					// ignored (the sensor head is 90mm across, but part of that
					// slopes in, so this check should be those readings well
					// within the sensor).  Further note that shiny/black things
					// within the minimum range will sometimes report closer than
					// they are... 9/21/2010 MPL
					//if (dist == 0)
					// try not doing this for 500?????
					int mindist;

          if(myLaserModel == LMS5XX)
            mindist = 15;
          else
            mindist = 50;

					if (dist < mindist) {
            // too close to be valid, ignore
						ignore = true;
						// set this to greater than max range, so that some ARAM
						// features work
						dist = getMaxRange() + 1;
					}

					// on LMS5XX scalling factor = 1 is 65m =2 is 80m
          if(myLaserModel == LMS5XX)
            dist = dist * 2;

					/*
					  else if (!ignore && dist < 150)
					  {
					  //ignore = true;
					  ArLog::log(ArLog::Normal, "%s: Reading at %.1f %s is %d (not ignoring, just warning)",
					  getName(), atDeg,
					  eachChanMeasured, dist);
					  }
					*/
					reading->newData (dist, pose, encoderPose, transform, counter,
					                  time, ignore, 0); // no reflector yet
				} else if (measuringReflectance) {
					refl = packet->bufToUByte2();
					if (refl > 254 * 255) {
						reading->setExtraInt (refl/255);
						//ArLog::log (ArLog::Normal, "%s: refl at %g of %d (raw %d)", getName(), atDeg, refl/255, refl);
					}
					// if the bit is dazzled we could set it to be ignored, but
					// that'd be some later day
					else if (refl == 255 * 255) {
					}
				}
			}
			/*
			ArLog::log(ArLog::Normal,
			"Received: %s %s scan %d numReadings %d",
			packet->getCommandType(), packet->getCommandName(),
			myScanCounter, onReading);
			*/
		} // end for 16bit

		// if we processed the readings and they were our first ones set
		// it so it's not our first ones anymore so that we only do the
		// sin/cos once
		if (startedProcessing && myFirstReadings)
			myFirstReadings = false;
		// read the 8 bit channels, that's just reflectance for now
		myNumChans8Bit = packet->bufToUByte2();
		//myLogLevel,
		//ArLog::log(ArLog::Normal, "%s::sensorInterp() NumChans8Bit %d", getName(), myNumChans8Bit);
		char eachChanMeasured8Bit[1024];

		for (i = 0; i < myNumChans8Bit; i++) {
			eachChanMeasured8Bit[0] = '\0';
			packet->bufToStr (eachChanMeasured8Bit, sizeof (eachChanMeasured));
			/*
			// for LMS5XX Scaling Factor is a real number
			if (myIsLMS5XX)
			eachScalingFactor = packet->bufToByte4(); // FIX should be real
			else
			eachScalingFactor = packet->bufToUByte4(); // FIX should be real
			eachScalingOffset = packet->bufToUByte4(); // FIX should be real
			eachStartingAngle = packet->bufToByte4() / 10000.0;
			eachAngularStepWidth = packet->bufToUByte2() / 10000.0;
			eachNumberData = packet->bufToUByte2();
			*/
			// scaling factor
			packet->bufToByte4(); // FIX should be real
			// scaling offset
			packet->bufToUByte4(); // FIX should be real
			// starting angle
			packet->bufToByte4();
			// step width
			packet->bufToUByte2();
			// number of data (use the one from the first set to make sure
			// we stay in bounds, and since it should be the same for both)
			packet->bufToUByte2();
			// if this isn't the data we want then skip it

// PS - need to understand this more
			if (myLaserModelFamily != TiM) {

				if (strcasecmp (eachChanMeasured8Bit, "RSSI1") != 0) {
					ArLog::log (ArLog::Normal, "%s: Got unprocessable 8bit of %s", getName(),
											eachChanMeasured8Bit);
					continue;
				}
			}

			if (printing)
				ArLog::log (ArLog::Normal, "%s: Processing 8bit %s", getName(),
				            eachChanMeasured8Bit);

			for (atDeg = start,
			     it = myRawReadings->begin(),
			     onReading = 0;
			     onReading < eachNumberData;
			     atDeg += increment,
			     it++,
			     onReading++) {
				reading = (*it);
				refl = packet->bufToUByte();
				if (refl == 254) {
					reading->setExtraInt (32);
					// ArLog::log(ArLog::Normal, "%s: refl at %g of %d", getName(), atDeg, refl);
				}
				// if the bit is dazzled we could set it to be ignored, but
				// that'd be some later day
				else if (refl == 255) {
				}
			}
		} // end for 8bit channel

		// 10/17/11 - PS read the last bytes, the first
		// 4 bytes need to be zero, if timestamp is there
		// just grap it
		int positionData = packet->bufToUByte2();
		if (positionData == 0) {
			int deviceName = packet->bufToUByte2();
			if (deviceName == 0) {
				int comment = packet->bufToUByte2();
				if (comment == 0) {
					int timeStamp = packet->bufToUByte2();
					if (timeStamp != 0) {
						/*
						myYear = packet->bufToUByte2();
						myMonth = packet->bufToUByte();
						myMonthDay = packet->bufToUByte();
						myHour = packet->bufToUByte();
						myMinute = packet->bufToUByte();
						mySecond = packet->bufToUByte();
						myUSec = packet->bufToUByte4();
						*/
						/*
						  ArLog::log(ArLog::Terse, "%s::sensorInterp() Date= %d/%d/%d %d:%d:%d:%d TranDuration = %d\n",
						  getName(), myMonth, myMonthDay, myYear, myHour, myMinute, mySecond, myUSec, myTransmissionDuration);
						*/
					}
				}
			}
		}
		myDataMutex.unlock();
		//ArTime test;
		laserProcessReadings();
		unlockDevice();
		if (printing)
			ArLog::log (ArLog::Normal, "%s: Packet took = %ld",
			            getName(), packetRecvTime.mSecSince());
		delete packet;
	}
	/*
	if (printing)
	  ArLog::log(ArLog::Normal, "%s: Function took = %ld",
	       getName(), startTime.mSecSince());
	*/
}

AREXPORT ArLMS1XXPacket *ArLMS1XX::sendAndRecv(
		ArTime timeout, ArLMS1XXPacket *sendPacket, const char *recvName)
{
	ArLMS1XXPacket *packet;




    IFDEBUG(

	int i;
	char x[100000];
	int idx=0;
	for (i = 0;i < sendPacket->getLength();i++)
	{
		if (sendPacket->getBuf()[i] == '\002')
		{
			strcat(&x[idx], "<STX>");
			idx = idx+5;
		}
		else if (sendPacket->getBuf()[i] == '\003')
		{
			strcat(&x[idx], "<ETX>");
			idx = idx+5;
		}
		else
			sprintf(&x[idx++], "%c",(char *)sendPacket->getBuf()[i]);
	}

	ArLog::log(ArLog::Terse,
			"%s::sendAndRecv() Buffer = %s", getName(), x);

    ); // end IFDEBUG


	while (timeout.mSecTo() > 0)
	{
		// just ask for data
		if ((myConn->write(sendPacket->getBuf(), sendPacket->getLength())) == -1)
		{
			ArLog::log(ArLog::Terse,
					"%s::sendAndRecv() Could not send %s to laser", getName(), recvName);
			return NULL;
		}

		while (timeout.mSecTo() > 0 &&
				(packet = myReceiver.receivePacket(1000)) != NULL)
		{
			if (strcasecmp(packet->getCommandName(), recvName) == 0)
				return packet;
			// PS 9/1/11 - need to make sure we have a packet
			else if (packet != NULL)
			{
				ArLog::log(ArLog::Normal, "%s::sendAndRecv() received %s %s, was expecting %s",
						getName(),
						packet->getCommandType(),
						packet->getCommandName(),
						recvName);

				delete packet;
				packet = NULL;
			}
			else
			{
				ArLog::log(ArLog::Normal, "%s::sendAndRecv() call to receivePacket() failed  was expecting %s",
						getName(),
						recvName);

			}

		}
	}

	ArLog::log(ArLog::Normal,
			"%s::sendAndRecv() Did not get %s ack",
			getName(), recvName);

	return NULL;
}

AREXPORT bool ArLMS1XX::blockingConnect(void)
{
//	char buf[1024];
  ArSerialConnection *conn;

	if (!getRunning())
		runAsync();

	myConnMutex.lock();
	if (myConn == NULL)
	{
		ArLog::log(ArLog::Terse,
				"%s::blockingConnect() Could not connect because there is no connection defined",
				getName());
		myConnMutex.unlock();
		failedToConnect();
		return false;
	}

  if (myConn->getStatus() != ArDeviceConnection::STATUS_OPEN)
	{
		if ((conn = dynamic_cast<ArSerialConnection *>(myConn)) != NULL)
		{
			conn->setBaud(atoi(getStartingBaudChoice()));
      ArLog::log(ArLog::Normal, "%s: Will use serial connection %s at %d baud.", getName(), conn->getPort(), conn->getBaud());
		}
	}

  ArLog::log(ArLog::Normal, "%s: Opening connection...", getName());

	if (myConn->getStatus() != ArDeviceConnection::STATUS_OPEN &&
			!myConn->openSimple())
	{
		ArLog::log(ArLog::Terse,
				"%s::blockingConnect() Could not connect because the connection was not open and could not open it as %s %s", getName(), myConn->getPortType(), myConn->getPortName());
		myConnMutex.unlock();
		failedToConnect();
		return false;
	}
	// PS - set logging level and laser type in packetreceiver class
	myReceiver.setmyInfoLogLevel(myInfoLogLevel);
	myReceiver.setLaserModel(myLaserModel);
	myReceiver.setmyName(getName());

	switch (myLaserModel) {

		case ArLMS1XX::LMS1XX:
		case ArLMS1XX::LMS5XX:
			myReceiver.setReadTimeout(5);
		break;

		// for the TiM, we need to put
		// in a delay so that if it's 
		// powered off, we give it time
		// to come up
		case ArLMS1XX::TiM3XX:
			//ArUtil::sleep(10000);
			myReceiver.setReadTimeout(50);
		break;
			
    default:
      myReceiver.setReadTimeout(5);
    break;

	} // end switch laserModel

	myReceiver.setDeviceConnection(myConn);
	myConnMutex.unlock();

	lockDevice();
	myTryingToConnect = true;
	unlockDevice();

	laserPullUnsetParamsFromRobot();
	laserCheckParams();

	int size = (270 / .25 + 1);
	ArLog::log(myInfoLogLevel, "%s::blockingConnect() Setting current buffer size to %d",
			getName(), size);
	setCurrentBufferSize(size);


  ArLog::log(ArLog::Normal, "%s: Connecting to laser...", getName());
	switch (myLaserModel) {

		case ArLMS1XX::LMS1XX:
			if (lms1xxConnect())
				return true;
		break;

		case ArLMS1XX::TiM3XX:
    case TiM551:
    case TiM561:
    case TiM571:
			if (timConnect())
				return true;
		break;

		case ArLMS1XX::LMS5XX:
			if (lms5xxConnect())
				return true;
		break;
			

	} // end switch laserModel


	ArLog::log(ArLog::Terse,
			"%s::blockingConnect() Could not connect to laser.", getName());
	failedToConnect();
	return false;

}

// scanfreq is scannnin frequency in hz, resolution is angle increment in
// degrees. these values will be converted as appropriate.

AREXPORT bool ArLMS1XX::lms5xxConnect(void)
{

		ArTime timeDone;
		if (myPowerControlled) {
			if (!timeDone.addMSec(60 * 1000)) {
				ArLog::log(ArLog::Normal,
						"%s::lms5xxConnect() error adding msecs (60 * 1000)");
			}
		}
		else {
			if (!timeDone.addMSec(30 * 1000)) {
				ArLog::log(ArLog::Normal,
						"%s::lms5xxConnect() error adding msecs (30 * 1000)");
			}
		}

		ArLMS1XXPacket *packet;

		ArLMS1XXPacket sendPacket;


		// 1. Log in
		sendPacket.empty();
		sendPacket.strToBuf("sMN");
		sendPacket.strToBuf("SetAccessMode");
		sendPacket.uByteToBuf(0x3); // level
		sendPacket.strToBuf("F4724744"); // hashed password
    sendPacket.finalizePacket();

    ArLog::log(myLogLevel, "%s::lms5xxConnect() sending SetAccessMode: %s", getName(),
        sendPacket.getBuf());

    if ((packet = sendAndRecv(timeDone, &sendPacket, "SetAccessMode")) != NULL)
    {

      ArLog::log(myLogLevel, "%s::lms5xxConnect() received SetAccessMode answer",getName());
      delete packet;
      packet = NULL;

    }
    else
    {
      ArLog::log(ArLog::Normal,
          "%s::lms5xxConnect() ::sendAndRecvfor send SetAccessMode failed", getName());
      failedToConnect();
      return false;
    }


    // 2. Set Frequency and Resolution
    sendPacket.empty();
    sendPacket.strToBuf("sMN");
    sendPacket.strToBuf("mLMPsetscancfg");



    // PS the scanning freq needs to be 5000 for .5 angle resolution and 2500 for angle resolution .25
    // PS 9/1/11 - use strcmp instead of double stuff
    // TODO set for other increment choices or use getIncrementDouble()*10000.
    if (strcmp(getIncrementChoice(),"quarter") == 0)
      //if (getIncrementChoiceDouble() == .25)
      sendPacket.byte4ToBuf(2500); // scanning freq
    else if (strcmp(getIncrementChoice(),"half") == 0)
			sendPacket.byte4ToBuf(5000); // scanning freq
		else
			sendPacket.byte4ToBuf(7500); // scanning freq


		sendPacket.byte2ToBuf(1); // number segments

		sendPacket.byte4ToBuf(getIncrementChoiceDouble() * 10000); // angle resolution
		//sendPacket.byte4ToBuf(.25 * 10000); // angle resolution
		//ArLog::log(ArLog::Normal,
		//		"%s::lms5xxConnect() increment = %d", getName(), getIncrementChoiceDouble());


    // we send the stand and end angle but according to protocol will not be
    // used (but they are included in the response packet)
		sendPacket.byte4ToBuf(-5 * 10000); // can't change starting angle
		sendPacket.byte4ToBuf(185 * 10000); // can't change ending angle

		sendPacket.finalizePacket();

		ArLog::log(myLogLevel, "%s::lms5xxConnect() sending setscancfg: %s", getName(),
				sendPacket.getBuf());


		if ((packet = sendAndRecv(timeDone, &sendPacket, "mLMPsetscancfg")) != NULL)
		{
			int val;
			val = packet->bufToUByte();

			delete packet;
			packet = NULL;

			if (val == 0)
			{
				ArLog::log(myLogLevel, "%s::lms5xxConnect() received setscancfg answer (%d)",
						getName(), val);
			}
			else
			{
				ArLog::log(ArLog::Normal,
						"%s::lms5xxConnect() received wrong status from setscancfg (%d), needs to be 0", getName(), val);
				failedToConnect();
				return false;
			}

		}
		else
		{
			ArLog::log(ArLog::Normal,
					"%s::lms5xxConnect() ::sendAndRecv for send setscancfg failed", getName());
			failedToConnect();
			return false;
		}

		// get frequency and angular resolution

		sendPacket.empty();
		sendPacket.strToBuf("sRN");
		sendPacket.strToBuf("LMPscancfg");
		sendPacket.finalizePacket();

		ArLog::log(myLogLevel, "%s::lms5xxConnect() sending scancfg: %s", getName(), sendPacket.getBuf());

		if ((packet = sendAndRecv(timeDone, &sendPacket, "LMPscancfg")) != NULL)
		{
			ArLog::log(myLogLevel, "%s::lms5xxConnect() received scancfg answer", getName());

			delete packet;
			packet = NULL;
		}
		else
		{
			ArLog::log(ArLog::Normal,
					"%s::lms5xxConnect() ::sendAndRecv for send scancfg failed", getName());
			failedToConnect();
			return false;
		}


		// set time

		time_t ltime;
		struct tm *Tm;
		ltime = time(NULL);
		Tm = localtime(&ltime);


		sendPacket.empty();
		sendPacket.strToBuf("sMN");
		sendPacket.strToBuf("LSPsetdatetime");
		sendPacket.byte2ToBuf(Tm->tm_year+1900);
		sendPacket.byteToBuf(Tm->tm_mon+1);
		sendPacket.byteToBuf(Tm->tm_mday);
		sendPacket.byteToBuf(Tm->tm_hour);
		sendPacket.byteToBuf(Tm->tm_min);
		sendPacket.byteToBuf(Tm->tm_sec);
		sendPacket.byteToBuf(0x0);

		sendPacket.finalizePacket();

		ArLog::log(myLogLevel, "%s::lms5xxConnect() sending setdatetime: %s", getName(), sendPacket.getBuf());

		if ((packet = sendAndRecv(timeDone, &sendPacket, "LSPsetdatetime")) != NULL)
		{
			ArLog::log(myLogLevel, "%s::lms5xxConnect() received setdatetime answer", getName());

			delete packet;
			packet = NULL;
		}
		else
		{
			ArLog::log(ArLog::Normal,
					"%s::lms5xxConnect() ::sendAndRecv for send setdatetime failed", getName());
			failedToConnect();
			return false;
		}


        // Configure scandata content
		sendPacket.empty();
		sendPacket.strToBuf("sWN");
		sendPacket.strToBuf("LMDscandatacfg");
		sendPacket.uByte2ToBuf(0x1); // output channel
		// if there's no reflectors do 0 here, otherwise put 1
		if (strcasecmp(getReflectorBitsChoice(), "none") == 0)
		  sendPacket.uByteToBuf(0x0); // remission (reflectance)
		else
		  sendPacket.uByteToBuf(0x1); // remission (reflectance)
		// if we're doing 16 bits set it that way
		if (strcasecmp(getReflectorBitsChoice(), "16bits") == 0)
		  sendPacket.uByteToBuf(0x1); // remission resolution
		else
		  sendPacket.uByteToBuf(0x0); // remission resolution
		sendPacket.uByteToBuf(0x0); // unit
		sendPacket.uByte2ToBuf(0x0); // encoder
		sendPacket.uByteToBuf(0x0); // position
		sendPacket.uByteToBuf(0x0); // device name
		sendPacket.uByteToBuf(0x0); // comment

		// PS 10/17/11 - per Matt L. request - turn on timeinfo
		sendPacket.uByteToBuf(0x1); // time
		//sendPacket.uByteToBuf(0x0); // time

		// PS 9/1/11 - based on increment choice set which scan
		if (strcmp(getIncrementChoice(),"quarter") == 0)
			sendPacket.byteToBuf(2); // which scan
		else if (strcmp(getIncrementChoice(),"half") == 0)
			sendPacket.byteToBuf(4); // which scan
		else
			sendPacket.byteToBuf(8); // which scan


   		//sendPacket.byteToBuf(myScanFreq); // which scan

		sendPacket.finalizePacket();

		ArLog::log(myLogLevel, "%s::lms5xxConnect() sending scandatacfg: %s", getName(), sendPacket.getBuf());

		if ((packet = sendAndRecv(timeDone, &sendPacket, "LMDscandatacfg")) != NULL)
		{
			ArLog::log(myLogLevel, "%s::lms5xxConnect() received scandatacfg answer", getName());

			delete packet;
			packet = NULL;
		}
		else
		{
			ArLog::log(ArLog::Normal,
					"%s::lms5xxConnect() ::sendAndRecv for send scandatacfg failed", getName());
			failedToConnect();
			return false;
		}



        // Configure scandata output
		sendPacket.empty();
		sendPacket.strToBuf("sWN");
		sendPacket.strToBuf("LMPoutputRange");
		sendPacket.byte2ToBuf(1); // number segments

		sendPacket.byte4ToBuf(getIncrementChoiceDouble() * 10000); // angle resolution
		//sendPacket.byte4ToBuf(.25 * 10000); // angle resolution

		 sendPacket.byte4ToBuf(-5 * 10000); // can't change starting angle
		sendPacket.byte4ToBuf(185 * 10000); // can't change ending angle

		sendPacket.finalizePacket();

		ArLog::log(myLogLevel, "%s::lms5xxConnect() sending outputRange: %s", getName(),
				sendPacket.getBuf());


		if ((packet = sendAndRecv(timeDone, &sendPacket, "LMPoutputRange")) != NULL)
		{

			ArLog::log(myLogLevel, "%s::lms5xxConnect() received outputRange answer", getName());
			delete packet;
			packet = NULL;

		}
		else
		{
			ArLog::log(ArLog::Normal,
					"%s::lms5xxConnect() ::sendAndRecv for send outputRange failed", getName());
			failedToConnect();
			return false;
		}


	       // Configure echo filter to echo first
			sendPacket.empty();
			sendPacket.strToBuf("sWN");
			sendPacket.strToBuf("FREchoFilter");
			sendPacket.uByteToBuf(0x0); // 0 = First Echo 1=All Echos 2=Last Echo

			sendPacket.finalizePacket();

			ArLog::log(myLogLevel, "%s::lms5xxConnect() sending Echo Filter: %s", getName(),
					sendPacket.getBuf());


			if ((packet = sendAndRecv(timeDone, &sendPacket, "FREchoFilter")) != NULL)
			{

				ArLog::log(myLogLevel, "%s::lms5xxConnect() received Echo Filter answer", getName());
				delete packet;
				packet = NULL;

			}
			else
			{
				ArLog::log(ArLog::Normal,
						"%s::lms5xxConnect() ::sendAndRecv for send Echo Filter failed", getName());
				failedToConnect();
				return false;
			}

		// Store Paramters
		sendPacket.empty();
		sendPacket.strToBuf("sMN");
		sendPacket.strToBuf("mEEwriteall");
		sendPacket.finalizePacket();

		ArLog::log(myLogLevel, "%s::lms5xxConnect() sending writeall: %s", getName(),
				sendPacket.getBuf());

		if ((packet = sendAndRecv(timeDone, &sendPacket, "mEEwriteall")) != NULL)
		{
			int val;
			val = packet->bufToUByte();
			delete packet;
			packet = NULL;
			if (val == 1)
			{
				ArLog::log(myLogLevel, "%s::lms5xxConnect() received writeall answer (%d)",
						getName(), val);
			}
			else
			{
				ArLog::log(ArLog::Normal,
						"%s::lms5xxConnect() received wrong status from writeall (%d), needs to be 1", getName(), val);
				failedToConnect();
				return false;
			}
		}
		else
		{
			ArLog::log(ArLog::Normal,
					"%s::lms5xxConnect() ::sendAndRecv for send writeall failed", getName());
			failedToConnect();
			return false;
		}

        // PS 8/22/11 - try a sleep per SICK because of issues with extra responses
		ArUtil::sleep(3000);

		// Log out
		sendPacket.empty();
		sendPacket.strToBuf("sMN");
		sendPacket.strToBuf("Run");
		sendPacket.finalizePacket();

		ArLog::log(myLogLevel, "%s::lms5xxConnect() sending Run: %s", getName(),
				sendPacket.getBuf());

		if ((packet = sendAndRecv(timeDone, &sendPacket, "Run")) != NULL)
		{
			int val;
			val = packet->bufToUByte();
			delete packet;
			packet = NULL;
			if (val == 1)
			{
				ArLog::log(myLogLevel, "%s::lms5xxConnect() received Run answer (%d)",
						getName(), val);
			}
			else
			{
				ArLog::log(ArLog::Normal,
						"%s::lms5xxConnect() received wrong status from Run (%d), needs to be 1", getName(), val);
				failedToConnect();
				return false;
			}
		}
		else
		{
			ArLog::log(ArLog::Normal,
					"%s::lms5xxConnect() ::sendAndRecv for send Run failed", getName());
			failedToConnect();
			return false;
		}





		// Check status
		// PS 9/27/11 - loop thru for a 30 sec, then exit
		//while (1)
		
		ArTime loopTimeDone;
		if (!loopTimeDone.addMSec(30 * 1000)) {
			ArLog::log(ArLog::Normal,
					"%s::lms5xxConnect() error adding msecs (30 * 1000)");
		}
		
		while (loopTimeDone.mSecTo() > 0)
		{
			sendPacket.empty();
			sendPacket.strToBuf("sRN");
			sendPacket.strToBuf("STlms");
			sendPacket.finalizePacket();

			ArLog::log(myLogLevel, "%s::lms5xxConnect() sending STlms: %s", getName(),
					sendPacket.getBuf());

			if ((packet = sendAndRecv(timeDone, &sendPacket, "STlms")) != NULL)
			{
				int val;
				val = packet->bufToUByte();
				delete packet;
				packet = NULL;
				if (val == 7)
				{
					ArLog::log(myLogLevel, "%s::lms5xxConnect() received STlms answer (%d)",
							getName(), val);
					break;
				}
				else
				{
					ArLog::log(ArLog::Normal,
							"%s::lms5xxConnect() wrong state received from STlms answer = (%d), needs to be 7", getName(), val);
					// PS 9/27/11 - took out call to failedToConnect() as this is not necessary as the laser is 
					// coming up during this loop and we actually are getting messages from the LMS500
					//failedToConnect();
					//return false;
				}
			}
			else
			{
				ArLog::log(ArLog::Normal,
						"%s::lms5xxConnect() ::sendAndRecv for send STlms failed", getName());
				failedToConnect();
				return false;
			}
		}

        // PS 9/27/11 - fail the connect if we don't get the correct status back in 30 seconds
		if (loopTimeDone.mSecTo() <= 0) {
			ArLog::log(ArLog::Normal,
						"%s::lms5xxConnect() timeout on STlms state changed never received a 7 status", getName());
			
			failedToConnect();
			return false;			
		}

		sendPacket.empty();
		sendPacket.strToBuf("sEN");
		sendPacket.strToBuf("LMDscandata");
		sendPacket.uByteToBuf(1);
		sendPacket.finalizePacket();

	    IFDEBUG(

		int i;
		char x[100000];
		int idx=0;
		for (i = 0;i < sendPacket.getLength();i++)
		{
			if (sendPacket.getBuf()[i] == '\002')
			{
				strcat(&x[idx], "<STX>");
				idx = idx+5;
			}
			else if (sendPacket.getBuf()[i] == '\003')
			{
				strcat(&x[idx], "<ETX>");
				idx = idx+5;
			}
			else
				sprintf(&x[idx++], "%c",(char *)sendPacket.getBuf()[i]);
		}

		ArLog::log(ArLog::Terse,
				"%s::lms5xxConnect() write Buffer = %s", getName(), x);

		); // end IFDEBUG


		//printf("(%s)\n", sendPacket.getBuf());
		// just ask for continuous data
		if ((myConn->write(sendPacket.getBuf(), sendPacket.getLength())) == -1)
		{
			ArLog::log(ArLog::Normal,
					"%s::lms5xxConnect() Could not send %s to laser", getName(), "LMDscandata");
			failedToConnect();
			return false;
		}

    		//ArLog::log(ArLog::Normal, "Waiting for response...");
		while (timeDone.mSecTo() > 0)
		{
			packet = myReceiver.receivePacket(1000);

      			//ArLog::log(ArLog::Normal, "Got packet %s %s", packet->getCommandType(), packet->getCommandName());

			// PS 9/1/11 - fix a bug where we were just looking for the sSN LMDscandata
			// but actually the LMS sends back a sEA LMDscandata first, so we need to
			// put up another read to the the sSN LMDscandata

			if (packet != NULL &&
					strcasecmp(packet->getCommandType(), "sEA") == 0 &&
					strcasecmp(packet->getCommandName(), "LMDscandata") == 0)
			{
				delete packet;
				packet = NULL;
				packet = myReceiver.receivePacket(1000);

				if (packet != NULL &&
						strcasecmp(packet->getCommandType(), "sSN") == 0 &&
						strcasecmp(packet->getCommandName(), "LMDscandata") == 0)
				{
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

					if (packet != NULL)
					{
						ArLog::log(ArLog::Normal, "%s::lms5xxConnect() received %s %s (%d long) was expecting %s", getName(),
								packet->getCommandType(), packet->getCommandName(),
								packet->getLength(),
								"sSN LMDscandata");
						delete packet;
						packet = NULL;
					}
					else
					{
						ArLog::log(ArLog::Normal, "%s::lms5xxConnect() call to receivePacket() failed was expecting %s",
								getName(),
								"sSN LMDscandata");
					}
				}
			}
			else
			{
				if (packet != NULL)
				{
					ArLog::log(ArLog::Normal, "%s::lms5xxConnect() received %s %s (%d long) was expecting %s", getName(),
							packet->getCommandType(), packet->getCommandName(),
							packet->getLength(),
							"sEA LMDscandata");
					delete packet;
					packet = NULL;
				}
				else
				{
					ArLog::log(ArLog::Normal, "%s::lms5xxConnect() call to receivePacket() failed was expecting %s",
							getName(),
							"sEA LMDscandata");
				}
			}
		}

	return false;

}

AREXPORT bool ArLMS1XX::lms1xxConnect(void)
{

		ArTime timeDone;
		if (myPowerControlled) {
			if (!timeDone.addMSec(60 * 1000)) {
				ArLog::log(ArLog::Normal,
						"%s::lms1xxConnect() error adding msecs (60 * 1000)");
			}
		}
		else {
			if (!timeDone.addMSec(30 * 1000)) {
				ArLog::log(ArLog::Normal,
						"%s::lms1xxConnect() error adding msecs (30 * 1000)");
			}
		}

		ArLMS1XXPacket *packet;

		ArLMS1XXPacket sendPacket;

		sendPacket.empty();
		sendPacket.strToBuf("sMN");
		sendPacket.strToBuf("SetAccessMode");
		sendPacket.uByteToBuf(0x3); // level
		sendPacket.strToBuf("F4724744"); // hashed password
		sendPacket.finalizePacket();

		if ((packet = sendAndRecv(timeDone, &sendPacket, "SetAccessMode")) != NULL)
		{
			int val;
			val = packet->bufToUByte();

			delete packet;
			packet = NULL;

			if (val == 1)
			{
				ArLog::log(myLogLevel, "%s::lms1xxConnect() Changed access mode (%d)",
						getName(), val);
			}
			else
			{
				ArLog::log(ArLog::Terse,
						"%s::lms1xxConnect() Could not change access mode (%d)", getName(), val);
				failedToConnect();
				return false;
			}
		}
		else
		{
			failedToConnect();
			return false;
		}


		sendPacket.empty();
		sendPacket.strToBuf("sMN");
		sendPacket.strToBuf("mLMPsetscancfg");
		sendPacket.byte4ToBuf(5000); // scanning freq
		sendPacket.byte2ToBuf(1); // number segments
		sendPacket.byte4ToBuf(getIncrementChoiceDouble() * 10000); // angle resolution
		sendPacket.byte4ToBuf((getStartDegrees() + 90) * 10000); // starting angle
		sendPacket.byte4ToBuf((getEndDegrees() + 90) * 10000); // ending angle

		sendPacket.finalizePacket();

		ArLog::log(ArLog::Verbose, "%s::lms1xxConnect() mLMPsetscancfg: %s", getName(),
				sendPacket.getBuf());

		if ((packet = sendAndRecv(timeDone, &sendPacket, "mLMPsetscancfg")) != NULL)
		{
			int val;
			val = packet->bufToUByte();

			delete packet;
			packet = NULL;

			if (val == 0)
			{
				ArLog::log(myLogLevel, "%s::lms1xxConnect() mLMPsetscancfg succeeded (%d)",
						getName(), val);
			}
			else
			{
        // See LMS100 operating instruction manual section 10.2.9 for
        // mLMPsetscancfg answer packet documentation
				if(val == 1)
					ArLog::log(ArLog::Terse, 
            "%s::lms1xxConnect() mLMPsetscancfg failed with error %d: Invalid scanning frequency (%d).", 
            getName(), val, 5000);
				else if(val == 2)
					ArLog::log(ArLog::Terse, 
          "%s::lms1xxConnect() mLMPsetscancfg failed with error %d: Invalid resolution (increment) (%.2f degrees, sent value %d to laser).", 
          getName(), val, getIncrementChoiceDouble(), (int)(getIncrementChoiceDouble()*10000));
				else if(val == 3)
					ArLog::log(ArLog::Terse, 
            "%s::lms1xxConnect() mLMPsetscancfg failed with error %d: Invalid scanning frequency (%d) AND invalid resolution (increment) (%.2f degrees, sent value %d to laser).", 
            getName(), val, 5000, getIncrementChoiceDouble(), (int)(getIncrementChoiceDouble()*10000));
				else if(val == 4)
					ArLog::log(ArLog::Terse, 
            "%s:lms1xxConnect() mLMPsetscancfg failed with error %d: Invalid scan area (Sent %d %d to laser)", 
            getName(), val, -45*10000, 225*10000);
				else
					ArLog::log(ArLog::Terse, 
            "%s:lms1xxConnect() mLMPsetscancfg failed with error %d: Unknown error", 
            getName(), val);

				failedToConnect();
				return false;
			}

		}
		else
		{
			failedToConnect();
			return false;
		}


		sendPacket.empty();
		sendPacket.strToBuf("sWN");
		sendPacket.strToBuf("LMDscandatacfg");
		sendPacket.uByte2ToBuf(0x1); // output channel 
		sendPacket.uByteToBuf(0x0); // remission
		sendPacket.uByteToBuf(0x0); // remission resolution
		sendPacket.uByteToBuf(0x0); // unit
		sendPacket.uByte2ToBuf(0x0); // encoder
		sendPacket.uByteToBuf(0x0); // position
		sendPacket.uByteToBuf(0x0); // device name
		sendPacket.uByteToBuf(0x0); // comment
		sendPacket.uByteToBuf(0x0); // time
		//sendPacket.byteToBuf(5); // every 5th scan only???
		//sendPacket.byteToBuf(1); // which scan ?
    sendPacket.uByteToBuf(1); // send all scans
		sendPacket.finalizePacket();

		ArLog::log(myLogLevel, "%s::lms1xxConnect() scandatacfg: %s", getName(), sendPacket.getBuf());

		if ((packet = sendAndRecv(timeDone, &sendPacket, "LMDscandatacfg")) != NULL)
		{
			ArLog::log(myLogLevel, "%s::lms1xxConnect() scandatacfg succeeded", getName());

			delete packet;
			packet = NULL;
		}
		else
		{
			failedToConnect();
			return false;
		}

		sendPacket.empty();
		sendPacket.strToBuf("sMN");
		sendPacket.strToBuf("Run");
		sendPacket.finalizePacket();

		if ((packet = sendAndRecv(timeDone, &sendPacket, "Run")) != NULL)
		{
			int val;
			val = packet->bufToUByte();
			delete packet;
			packet = NULL;
			if (val == 1)
			{
				ArLog::log(myLogLevel, "%s::lms1xxConnect() Run succeeded (%d)",
						getName(), val);
			}
			else
			{
				ArLog::log(ArLog::Terse,
						"%s::lms1xxConnect() Could not run (%d)", getName(), val);
				failedToConnect();
				return false;
			}
		}
		else
		{
			failedToConnect();
			return false;
		}

		/* when asking one at a time
  sendPacket.empty();
  sendPacket.strToBuf("sRN");
  sendPacket.strToBuf("LMDscandata");
  sendPacket.finalizePacket();

  if ((packet = sendAndRecv(timeDone, &sendPacket, "LMDscandata")) != NULL)
  {
    ArLog::log(myLogLevel, "%s: Got %s scan data %d", getName(), 
	       packet->getCommandType(), packet->getLength());
    myPacketsMutex.lock();
    myPackets.push_back(packet);
    myPacketsMutex.unlock();	
    sensorInterp();

    ArLog::log(myLogLevel, "%s: Processed scan data", getName());

  }
  else
  {
    failedToConnect();
    return false;
  }
		 */

		sendPacket.empty();
		sendPacket.strToBuf("sEN");
		sendPacket.strToBuf("LMDscandata");
		sendPacket.uByteToBuf(1);
		sendPacket.finalizePacket();

		//printf("(%s)\n", sendPacket.getBuf());
		// just ask for continuous data
		if ((myConn->write(sendPacket.getBuf(), sendPacket.getLength())) == -1)
		{
			ArLog::log(ArLog::Terse,
					"%s::lms1xxConnect() Could not send %s to laser", getName(), "LMDscandata");
			failedToConnect();
			return false;
		}
		ArTime t;
		while (timeDone.mSecTo() > 0)
		{
			packet = myReceiver.receivePacket(1000);
			if (packet != NULL &&
					strcasecmp(packet->getCommandType(), "sSN") == 0 &&
					strcasecmp(packet->getCommandName(), "LMDscandata") == 0)
			{
				ArLog::log(ArLog::Normal, "%s::lms1xxConnect() Took %f sec to get sSN LMDscandata packet from laser.\n", getName(), (double)(t.mSecSince())/1000.0);
				delete packet;
				packet = NULL;

				lockDevice();
				myIsConnected = true;
				myTryingToConnect = false;
				unlockDevice();
				ArLog::log(ArLog::Normal, "%s::lms1xxConnect() Connected to laser", getName());
				laserConnect();
				return true;
			}
			else if (packet != NULL)
			{
				ArLog::log(ArLog::Normal, "%s::lms1xxConnect() Got %s %s (%d long) while waiting for sSN LMDscandata", getName(),
						packet->getCommandType(), packet->getCommandName(),
						packet->getLength());
				delete packet;
				packet = NULL;
			}
		}
	return false;

}

AREXPORT bool ArLMS1XX::timConnect(void)
{

		ArTime timeDone;
		if (myPowerControlled) {
			if (!timeDone.addMSec(60 * 1000)) {
				ArLog::log(ArLog::Normal,
						"%s::timConnect() error adding msecs (60 * 1000)");
			}
		}
		else {
			if (!timeDone.addMSec(30 * 1000)) {
				ArLog::log(ArLog::Normal,
						"%s::timConnect() error adding msecs (30 * 1000)");
			}
		}

		ArLMS1XXPacket *packet;

		ArLMS1XXPacket sendPacket;

		// first send a stop

		sendPacket.empty();
		sendPacket.strToBuf("sEN");
		sendPacket.strToBuf("LMDscandata");
		sendPacket.uByteToBuf(0);
		sendPacket.finalizePacket();

		//printf("(%s)\n", sendPacket.getBuf());
		// just ask for continuous data

		ArLog::log(ArLog::Terse,
					"%s::timConnect() Sending (STOP) sEN LMDscandata ", getName());

		if ((myConn->write(sendPacket.getBuf(), sendPacket.getLength())) == -1)
		{
			ArLog::log(ArLog::Terse,
					"%s::timConnect() Could not send sEN LMDscandata to laser", getName());
			failedToConnect();
			return false;
		}

		ArUtil::sleep(1000);

		// now look for 1 packet

		sendPacket.empty();
		sendPacket.strToBuf("sRN");
		sendPacket.strToBuf("LMDscandata");
		sendPacket.finalizePacket();

		//printf("(%s)\n", sendPacket.getBuf());
		// just ask for continuous data

		ArLog::log(ArLog::Terse,
					"%s::timConnect() Sending (Only one Telegram) sRN LMDscandata ", getName());

		if ((myConn->write(sendPacket.getBuf(), sendPacket.getLength())) == -1)
		{
			ArLog::log(ArLog::Terse,
					"%s::timConnect() Could not send sRN LMDscandata to laser", getName());
			failedToConnect();
			return false;
		}


    ArLog::log(ArLog::Normal, "%s::timConnect: Waiting for response...", getName());

		// now get the response to the one telegram
		ArTime t;
		while (timeDone.mSecTo() > 0)
		{

			packet = myReceiver.receivePacket(1000);

      // debug:
      if(packet) ArLog::log(ArLog::Normal, "Got response packet %s %s", packet->getCommandType(), packet->getCommandName());

			if (packet != NULL &&
//					strcasecmp(packet->getCommandType(), "sSN") == 0 &&
					strcasecmp(packet->getCommandType(), "sRA") == 0 &&
					strcasecmp(packet->getCommandName(), "LMDscandata") == 0)
			{
				ArLog::log(ArLog::Normal, "%s::timConnect() Took %f sec to get sRA LMDscandata packet from laser.", getName(), (double)(t.mSecSince())/1000.0);
				delete packet;
				packet = NULL;

				// now send data permanent

				sendPacket.empty();
				sendPacket.strToBuf("sEN");
				sendPacket.strToBuf("LMDscandata");
				sendPacket.uByteToBuf(1);
				sendPacket.finalizePacket();

				ArLog::log(ArLog::Terse,
							"%s::timConnect() Sending (send permanent) sEN LMDscandata ", getName());

				if ((myConn->write(sendPacket.getBuf(), sendPacket.getLength())) == -1)
				{
					ArLog::log(ArLog::Terse,
							"%s::timConnect() Could not send sEN LMDscandata to laser", getName());
					failedToConnect();
					return false;
				}
				
				// grab the response
				packet = myReceiver.receivePacket(1000);
				if (packet != NULL &&
					strcasecmp(packet->getCommandType(), "sEA") == 0 &&
					strcasecmp(packet->getCommandName(), "LMDscandata") == 0)
				{
					ArLog::log(ArLog::Normal, "%s::timConnect() Took %f sec to get sEA LMDscandata packet from laser.", getName(), (double)(t.mSecSince())/1000.0);
					delete packet;
					packet = NULL;

					lockDevice();
					myIsConnected = true;
					myTryingToConnect = false;
					unlockDevice();
					ArLog::log(ArLog::Normal, "%s::timConnect() Connected to laser", getName());
					laserConnect();
					return true;
				}
				else
				{
					ArLog::log(ArLog::Terse,
							"%s::timConnect() Could not get sEA LMDscandata from laser", getName());
					failedToConnect();
					return false;
				}
			}
			else if (packet != NULL)
			{
				ArLog::log(ArLog::Normal, "%s::timConnect() Got unexpected %s %s (%d long) while waiting for sRA LMDscandata", getName(),
						packet->getCommandType(), packet->getCommandName(),
						packet->getLength());
				delete packet;
				packet = NULL;

			}
			else if (packet == NULL) {
				if ((myConn->write(sendPacket.getBuf(), sendPacket.getLength())) == -1) {
					ArLog::log(ArLog::Terse,
							"%s::timConnect() Could not send sRN LMDscandata to laser", getName());
					failedToConnect();
					return false;
				}
			}
		}
	ArLog::log(ArLog::Normal,
						"%s::timConnect() Timout waiting for initial packet from TiM - failing connect", getName());
	failedToConnect();
	return false;

}

AREXPORT void * ArLMS1XX::runThread(void *arg)
{
  //char buf[1024];
  ArLMS1XXPacket *packet;
  
  /*
    ArTime dataRequested;
    
    ArLMS1XXPacket requestPacket;
    requestPacket.strToBuf("sRN");
    requestPacket.strToBuf("LMDscandata");
    requestPacket.finalizePacket();
  */
  
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
    
    
    /*
      dataRequested.setToNow();
      
      if (myConn == NULL || !myConn->write(requestPacket.getBuf(), 
      requestPacket.getLength()))
      {
      ArLog::log(ArLog::Terse, "Could not send packets request to lms1XX");
      continue;
      }
    */
    
    /// MPL made this 500 since otherwise it will not get a packet
    /// sometimes, then go out and have to sleep and come back in and
    /// wind up with a remainder that caused the timing problems
    while (getRunning() && myIsConnected &&
	   (packet = myReceiver.receivePacket(500, true, true)) != NULL)
    {
      myPacketsMutex.lock();
      myPackets.push_back(packet);
      myPacketsMutex.unlock();
      
      if (myRobot == NULL)
	sensorInterp();
    }

    // if we have a robot but it isn't running yet then don't have a
    // connection failure
    if (getRunning() && myIsConnected && laserCheckLostConnection())
    {
      ArLog::log(ArLog::Terse,
		 "%s::runThread()  Lost connection to the laser because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(),
		 myLastReading.mSecSince()/1000.0,
		 getConnectionTimeoutSeconds());
      myIsConnected = false;
      laserDisconnectOnError();
      continue;
    }
    
    /// MPL no sleep here so it'll get back into that while as soon as it can

    //ArUtil::sleep(1);
    //ArUtil::sleep(2000);
    //ArUtil::sleep(500);
  }
  return NULL;
}


bool ArLMS1XX::validateCheckSum(ArLMS1XXPacket *packet)

{

  // XOR all values in the message including the checksum, which should result
  // in 0.

  unsigned short checksum = 0;
  char str[1024];
  char *pch;
  unsigned char val;
  unsigned char val1;

  const int maxcopylen = ArUtil::findMin(1024, packet->getLength());
	strncpy(str, packet->getBuf(), maxcopylen);
  str[maxcopylen-1] = '\0';

  IFDEBUG(
		ArLog::log(ArLog::Normal,
		 "%s::validateCheckSum() packet contains %s", getName(), str);
  )
  
	pch = strtok(&str[17], " ");
	while (pch != NULL) {

		if (pch[0] != '0') {
			if (strcmp(pch, "DIST1") == 0) {
				checksum ^= 0x44;
				checksum ^= 0x49;
				checksum ^= 0x53;
				checksum ^= 0x54;
				checksum ^= 0x31;
			}
			else {
				// if it's an odd number - do the 1st byte
				if (strlen(pch) & 1) {

					if ((pch[0] >= '0') && (pch[0] <= '9')) {
						val = pch[0] - '0';
					} 
					else if ((pch[0] >= 'A') && (pch[0] <= 'F')) {
						val = 10 + pch[0] - 'A';
					}

				checksum ^= val;
			
				pch = &pch[1];

				}

				while (strlen(pch) != 0) {

					if ((pch[0] >= '0') && (pch[0] <= '9')) {
						val = pch[0] - '0';
					} 
					else if ((pch[0] >= 'A') && (pch[0] <= 'F')) {
						val = 10 + pch[0] - 'A';
					}

					if ((pch[1] >= '0') && (pch[1] <= '9')) {
						val1 = pch[1] - '0';
					} 
					else if ((pch[1] >= 'A') && (pch[1] <= 'F')) {
						val1 = 10 + pch[1] - 'A';
					}
					checksum ^= (val1 | (val << 4));       // Calculate the checksum... (XOR)

					pch = &pch[2];
				}
			}
		}
		pch = strtok(NULL, " ");
	} // end while


  IFDEBUG(
    if (checksum == 0)
      ArLog::log(ArLog::Normal,
       "%s::validateCheckSum() checksum = 0x%x", getName(), checksum);
    else
      ArLog::log(ArLog::Normal,
        "%s::validateCheckSum() INVALID checksum = 0x%x", getName(), checksum);
  )

	return (checksum == 0);

}

