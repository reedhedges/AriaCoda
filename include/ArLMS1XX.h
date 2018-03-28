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
#ifndef ARLMS1XX_H
#define ARLMS1XX_H

#include "ariaTypedefs.h"
#include "ArRobotPacket.h"
#include "ArLaser.h"   
#include "ArFunctor.h"

/** @internal 
  Constructs packets for LMS1xx ASCII protocol. 
  The various ...ToBuf() methods select argument types and how
they are written as ascii strings, the protocol is space delimited not fixed
width values (as in other Packet implementations), so they don't imply number of bytes used in packet output.
*/
class ArLMS1XXPacket : public ArBasePacket
{
public:
  /// Constructor
  AREXPORT ArLMS1XXPacket();
  /// Destructor
  AREXPORT virtual ~ArLMS1XXPacket();

  /// Gets the command type 
  AREXPORT const char *getCommandType(void);
  /// Gets the command name
  AREXPORT const char *getCommandName(void);
  
  // only call finalizePacket before a send
  AREXPORT virtual void finalizePacket(void);
  AREXPORT virtual void resetRead(void);
  
  /// Gets the time the packet was received at
  AREXPORT ArTime getTimeReceived(void);
  /// Sets the time the packet was received at
  AREXPORT void setTimeReceived(ArTime timeReceived);

  AREXPORT virtual void duplicatePacket(ArLMS1XXPacket *packet);
  AREXPORT virtual void empty(void);
  
  AREXPORT virtual void byteToBuf(ArTypes::Byte val);
  AREXPORT virtual void byte2ToBuf(ArTypes::Byte2 val);
  AREXPORT virtual void byte4ToBuf(ArTypes::Byte4 val);
  AREXPORT virtual void uByteToBuf(ArTypes::UByte val);
  AREXPORT virtual void uByte2ToBuf(ArTypes::UByte2 val);
  AREXPORT virtual void uByte4ToBuf(ArTypes::UByte4 val);
  AREXPORT virtual void strToBuf(const char *str);

  AREXPORT virtual ArTypes::Byte bufToByte(void);
  AREXPORT virtual ArTypes::Byte2 bufToByte2(void);
  AREXPORT virtual ArTypes::Byte4 bufToByte4(void);
  AREXPORT virtual ArTypes::UByte bufToUByte(void);
  AREXPORT virtual ArTypes::UByte2 bufToUByte2(void);
  AREXPORT virtual ArTypes::UByte4 bufToUByte4(void);
  AREXPORT virtual void bufToStr(char *buf, int len);

  // adds a raw char to the buf
  AREXPORT virtual void rawCharToBuf(unsigned char c);
protected:
  int deascii(char c);

  ArTime myTimeReceived;
  bool myFirstAdd;

  char myCommandType[1024]; 
  char myCommandName[1024]; 
};


/// Given a device connection it receives packets from the sick through it
/// @internal
class ArLMS1XXPacketReceiver
{
public:
  /// Constructor with assignment of a device connection
  AREXPORT ArLMS1XXPacketReceiver();
  /// Destructor
  AREXPORT virtual ~ArLMS1XXPacketReceiver();
  
  /// Receives a packet from the robot if there is one available
  AREXPORT ArLMS1XXPacket *receivePacket(unsigned int msWait = 0,
					 bool shortcut = false, 
					 bool ignoreRemainders = false);

  AREXPORT ArLMS1XXPacket *receiveTiMPacket(unsigned int msWait = 0,
					 bool shortcut = false, 
					 bool ignoreRemainders = false);

  /// Sets the device this instance receives packets from
  AREXPORT void setDeviceConnection(ArDeviceConnection *conn);
  /// Gets the device this instance receives packets from
  AREXPORT ArDeviceConnection *getDeviceConnection(void);

  // PS - added to pass info to this class
  AREXPORT void	setmyInfoLogLevel(ArLog::LogLevel infoLogLevel)
  { myInfoLogLevel = infoLogLevel; }
  AREXPORT void setLaserModel(int laserModel)
  { myLaserModel = laserModel; }
  AREXPORT void setmyName(const char *name )
  { 
    strncpy(myName, name, sizeof(myName)); 
    myName[sizeof(myName)-1] = '\0';
  }
  AREXPORT void setReadTimeout(int timeout )
  { myReadTimeout = timeout; }


protected:
  ArDeviceConnection *myConn;
  ArLMS1XXPacket myPacket;
  
  enum State 
  {
    STARTING, ///< Looking for beginning
    DATA, ///< Read the data in a big whack
    REMAINDER ///< Have extra data from reading in data
  };
  State myState;
  char myName[1024];
  unsigned int myNameLength;
  char myReadBuf[100000];
  int myReadCount;
	int myReadTimeout;

	int myLaserModel;

  ArLog::LogLevel myInfoLogLevel;
};

/**
  @since Aria 2.7.2
  @see ArLaserConnector
  Use ArLaserConnector to connect to a laser, determining type based on robot and program configuration  parameters.

  This is the ArLaser implementation for SICK LMS1xx, LMS5xx, TiM310/510
  (aka TiM3xx), TiM551, TiM561, and TiM571  lasers. To use these lasers with ArLaserConnector, specify 
  the appropriate type in program configuration (lms1xx, lms5xx, tim3xx or
  tim510, tim551, tim561, tim571).
*/
class ArLMS1XX : public ArLaser
{
public:

	enum LaserModel
	{
		LMS1XX, 
		LMS5XX,
		TiM3XX,
    TiM551,
    TiM561,
    TiM571
	};

  /// Constructor
	  AREXPORT ArLMS1XX(int laserNumber,
			    const char *name,
					LaserModel laserModel);

  /// Destructor
  AREXPORT ~ArLMS1XX();
  AREXPORT virtual bool blockingConnect(void);

	// specific init routine per laser
  AREXPORT virtual bool lms5xxConnect(void);
  AREXPORT virtual bool lms1xxConnect(void);
  AREXPORT virtual bool timConnect(void);

  AREXPORT virtual bool asyncConnect(void);
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

  /// Logs the information about the sensor
  AREXPORT void log(void);


protected:
  AREXPORT virtual void laserSetName(const char *name);
  AREXPORT virtual void * runThread(void *arg);
  AREXPORT virtual void setRobot(ArRobot *robot);
  AREXPORT ArLMS1XXPacket *sendAndRecv(
	  ArTime timeout, ArLMS1XXPacket *sendPacket, const char *recvName);
  void sensorInterp(void);
  void failedToConnect(void);
  void clear(void);

  /// @return true if message contents matches checksum, false otherwise.
  bool validateCheckSum(ArLMS1XXPacket *packet);

  LaserModel myLaserModel;

  enum LaserModelFamily
  {
    LMS,
    TiM,
  };

  LaserModelFamily myLaserModelFamily;

  bool myIsConnected;
  bool myTryingToConnect;
  bool myStartConnect;
  int myScanFreq;

  int myVersionNumber;
  int myDeviceNumber;
  int mySerialNumber;
  int myDeviceStatus1;
  int myDeviceStatus2;
  int myMessageCounter;
  int myScanCounter;
  int myPowerUpDuration;
  int myTransmissionDuration;
  int myInputStatus1;
  int myInputStatus2;
  int myOutputStatus1;
  int myOutputStatus2;
  int myReserved;
  int myScanningFreq;
  int myMeasurementFreq;
  int myNumberEncoders;
  int myNumChans16Bit;
  int myNumChans8Bit;
  int myFirstReadings;
  int myYear;
  int myMonth;
  int myMonthDay;
  int myHour;
  int myMinute;
  int mySecond;
  int myUSec;

  ArLog::LogLevel myLogLevel;

  ArLMS1XXPacketReceiver myReceiver;

  ArMutex myPacketsMutex;
  ArMutex myDataMutex;

  std::list<ArLMS1XXPacket *> myPackets;

  ArFunctorC<ArLMS1XX> mySensorInterpTask;
  ArRetFunctorC<bool, ArLMS1XX> myAriaExitCB;
};

#endif 
