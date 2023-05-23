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


*/
#ifndef ARLMS1XX_H
#define ARLMS1XX_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ArRobotPacket.h"
#include "Aria/ArLaser.h"   
#include "Aria/ArFunctor.h"

#include <string>

#ifndef ARIA_WRAPPER
/** @internal 
  Constructs packets for LMS1xx ASCII protocol. 
  The various ...ToBuf() methods select argument types and how
they are written as ascii strings, the protocol is space delimited not fixed
width values (as in other Packet implementations), so they don't imply number of bytes used in packet output.
*/
class ArLMS1XXPacket final : public ArBasePacket
{
public:
  /// Constructor
  ArLMS1XXPacket();

  /// Gets the command type 
  const char *getCommandType();
  /// Gets the command name
  const char *getCommandName();
  
  // only call finalizePacket before a send
  virtual void finalizePacket() override;
  virtual void resetRead() override;
  
  /// Gets the time the packet was received at
  ArTime getTimeReceived();
  /// Sets the time the packet was received at
  void setTimeReceived(ArTime timeReceived);

  void duplicatePacket(ArLMS1XXPacket *packet);
  virtual void duplicatePacket(ArBasePacket *packet) override {
    duplicatePacket(dynamic_cast<ArLMS1XXPacket*>(packet));
  }

  virtual void empty() override;

  void intToBuf(int val);
  virtual void byteToBuf(int8_t val) override { intToBuf((int)val); }
  virtual void byte2ToBuf(int16_t val) override { intToBuf((int)val); }
  virtual void byte4ToBuf(int32_t val) override { intToBuf((int)val); }
  virtual void uByteToBuf(uint8_t val) override;
  virtual void uByte2ToBuf(uint16_t val) override;
  virtual void uByte4ToBuf(uint32_t val) override;
  virtual void strToBuf(const char *str) override;


private:
  // Not sure what the previous versions of these were trying to implement but they are not used
  virtual int8_t bufToByte() override { assert(false); return 0; }
  virtual int16_t bufToByte2() override { assert(false); return 0; }

public:
  /// Note unlike other accessors, this simply reads next 8 text bytes in which each pair is a digit of the number, combines them bitwise, and returns the number.
  virtual int32_t bufToByte4() override;


  virtual uint8_t bufToUByte() override
  {
    //assert(r >= 0 && r <= UINT8_MAX)
    return (uint8_t)getNumFromBufHexText();
  }

  virtual uint16_t bufToUByte2() override
  {
    //assert(r >= 0 && r <= UINT16_MAX)
    return (uint16_t)getNumFromBufHexText();
  }

  virtual uint32_t bufToUByte4() override
  {
    //assert(r >= 0 && r <= UINT32_MAX)
    return (uint32_t) getNumFromBufHexText();
  }


  virtual void bufToStr(char *buf, size_t len) override;

  // adds a raw char to the buf
  virtual void rawCharToBuf(unsigned char c);

private:
  // not implemented, change to private:
  using ArBasePacket::byte8ToBuf;
  using ArBasePacket::uByte8ToBuf;

protected:
  int deascii(char c);

  ArTime myTimeReceived;
  bool myFirstAdd;

  char myCommandType[1024]; 
  char myCommandName[1024]; 

  long getNumFromBufHexText();

};
#endif

#ifndef ARIA_WRAPPER
/// Given a device connection it receives packets from the sick through it
/// @internal
class ArLMS1XXPacketReceiver
{
public:
  /// Constructor with assignment of a device connection
  ArLMS1XXPacketReceiver();

  
  /// Receives a packet from the robot if there is one available
  ArLMS1XXPacket *receivePacket(unsigned int msWait = 0,
					 bool shortcut = false, 
					 bool ignoreRemainders = false);

  ArLMS1XXPacket *receiveTiMPacket(unsigned int msWait = 0,
					 bool shortcut = false, 
					 bool ignoreRemainders = false);

  /// Sets the device this instance receives packets from
  void setDeviceConnection(ArDeviceConnection *conn);
  /// Gets the device this instance receives packets from
  ArDeviceConnection *getDeviceConnection();

  // PS - added to pass info to this class
  void	setmyInfoLogLevel(ArLog::LogLevel infoLogLevel)
  { myInfoLogLevel = infoLogLevel; }
  void setLaserModel(int laserModel)
  { myLaserModel = laserModel; }
  void setmyName(const char *name )
  { 
    myName = name;
    //strncpy(myName, name, sizeof(myName)); 
    //myName[sizeof(myName)-1] = '\0';
  }
  void setReadTimeout(unsigned int timeout )
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
  std::string myName;
  //unsigned int myNameLength;
  char myReadBuf[100000];
  unsigned int myReadCount;
	unsigned int myReadTimeout;

	int myLaserModel;

  ArLog::LogLevel myInfoLogLevel;
};
#endif

/**
  @since Aria 2.7.2
  @see ArLaserConnector
  Use ArLaserConnector to connect to a laser, determining type based on robot and program configuration  parameters.

  This is the ArLaser implementation for SICK LMS1xx, LMS5xx, TiM310/510
  (aka TiM3xx), TiM551, TiM561, and TiM571  lasers. To use these lasers with ArLaserConnector, specify 
  the appropriate type in program configuration (lms1xx, lms5xx, tim3xx or
  tim510, tim551, tim561, tim571).
*/
class ArLMS1XX final : public ArLaser
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
  AREXPORT virtual bool blockingConnect();

	// specific init routine per laser
  AREXPORT virtual bool lms5xxConnect();
  AREXPORT virtual bool lms1xxConnect();
  AREXPORT virtual bool timConnect();

  AREXPORT virtual bool asyncConnect();
  AREXPORT virtual bool disconnect();
  AREXPORT virtual bool isConnected() { return myIsConnected; }
  AREXPORT virtual bool isTryingToConnect() 
    { 
      if (myStartConnect)
	return true;
      else if (myTryingToConnect)
	return true; 
      else
	return false;
    }  

  /// Logs the information about the sensor
  AREXPORT void log();


protected:
  AREXPORT virtual void laserSetName(const char *name);
  AREXPORT virtual void * runThread(void *arg);
  AREXPORT virtual void setRobot(ArRobot *robot);
  AREXPORT ArLMS1XXPacket *sendAndRecv(
	  ArTime timeout, ArLMS1XXPacket *sendPacket, const char *recvName);
  void sensorInterp();
  void failedToConnect();
  void clear();

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
  unsigned int mySerialNumber;
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
