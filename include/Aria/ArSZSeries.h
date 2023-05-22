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
#ifndef ARSZSERIES_H
#define ARSZSERIES_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ariaOSDef.h"
#include "Aria/ArBasePacket.h"
#include "Aria/ArLaser.h"   
#include "Aria/ArFunctor.h"

#ifndef ARIA_WRAPPER
/** @internal */
class ArSZSeriesPacket final : public ArBasePacket
{
public:
  /// Constructor
  ArSZSeriesPacket();
  
  /// Gets the time the packet was received at
  ArTime getTimeReceived();
  /// Sets the time the packet was received at
  void setTimeReceived(ArTime timeReceived);

  void duplicatePacket(ArSZSeriesPacket *packet);

  virtual void duplicatePacket(ArBasePacket *packet) override {
    duplicatePacket(dynamic_cast<ArSZSeriesPacket*>(packet));
  }

  virtual void empty() override;

  // not used AREXPORT virtual void byteToBuf(int8_t val) override;

  virtual int8_t bufToByte() override;
  
  void setDataLength(int x)
  { myDataLength = x; }

  virtual uint16_t getDataLength() const override
  { 
    assert(myDataLength <= USHRT_MAX);
    return (uint16_t) myDataLength; 
  }

  void setNumReadings(int x)
  { myNumReadings = x; }
  int getNumReadings()
  { return myNumReadings; }
  void setScanFrequency(int x)
  { myScanFrequency = x; }
  int getScanFrequency()
  { return myScanFrequency; }

  void setPrevScanFrequency(int x)
  { myPrevScanFrequency = x; }
  int getPrevScanFrequency()
  { return myPrevScanFrequency; }


  unsigned char getCrcByte1()
  { return myCrcByte1; }
  void setCrcByte1(unsigned char c)
  { myCrcByte1 = c; }
  unsigned char getCrcByte2()
  { return myCrcByte2; }
  void setCrcByte2(unsigned char c)
  { myCrcByte2 = c; }

private:
  int deascii(char c);

  ArTime myTimeReceived;

  // SZS specific
  int myDataLength;
  int myNumReadings;
  int myScanFrequency;
  unsigned char myCrcByte1;
  unsigned char myCrcByte2;
  
  int myPrevScanFrequency;


};


/// Given a device connection it receives packets from the sick through it
/// @internal
class ArSZSeriesPacketReceiver
{
public:
/*   // Constructor with assignment of a device connection
  AREXPORT ArSZSeriesPacketReceiver();
  // Destructor
  AREXPORT virtual ~ArSZSeriesPacketReceiver(); */
  
  /// Receives a packet from the robot if there is one available
  AREXPORT ArSZSeriesPacket *receivePacket(unsigned int msWait = 0,
					 bool shortcut = false);

  /// Sets the device this instance receives packets from
  AREXPORT void setDeviceConnection(ArDeviceConnection *conn);
  /// Gets the device this instance receives packets from
  AREXPORT ArDeviceConnection *getDeviceConnection();
  unsigned short CRC16(unsigned char *, int);

  // PS - added to pass info to this class
  AREXPORT void	setmyInfoLogLevel(ArLog::LogLevel infoLogLevel)
  { myInfoLogLevel = infoLogLevel; }
  AREXPORT void setmyIsSZ00(bool isSZ00)
  { myIsSZ00 = isSZ00; }
  AREXPORT void setmyName(const char *name )
  { myName.assign(name); }

private:
  ArDeviceConnection *myConn;
  ArSZSeriesPacket myPacket;
  
  std::string myName;
  unsigned int myNameLength;
  unsigned char myReadBuf[100000];
  int myReadCount;
  bool myIsSZ00;
  ArLog::LogLevel myInfoLogLevel;

  unsigned short myPrevCrc;


};

#endif

/**
  @since Aria 2.7.4
  @see ArLaserConnector
  Use ArLaserConnector to connect to a laser, determining type based on robot and program configuration  parameters.
*/
class ArSZSeries : public ArLaser
{
public:
  /// Constructor
  AREXPORT explicit ArSZSeries(int laserNumber,
		 const char *name = "SZSeries");
  /// Destructor
  AREXPORT virtual ~ArSZSeries();
  AREXPORT virtual bool blockingConnect() override;
  AREXPORT virtual bool asyncConnect() override;
  AREXPORT virtual bool disconnect() override;
  virtual bool isConnected() override { return myIsConnected; }
  virtual bool isTryingToConnect() override 
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
  AREXPORT virtual void laserSetName(const char *name) override;
  AREXPORT virtual void * runThread(void *arg) override;
  AREXPORT virtual void setRobot(ArRobot *robot) override;
  void sensorInterp();
  void failedToConnect();
  void clear();
  bool myIsConnected;
  bool myTryingToConnect;
  bool myStartConnect;

  int myNumChans;


  ArLog::LogLevel myLogLevel;

  ArSZSeriesPacketReceiver myReceiver;

  ArMutex myPacketsMutex;
  ArMutex myDataMutex;

  std::list<ArSZSeriesPacket *> myPackets;
  
  ArTime myPrevSensorIntTime;

  ArFunctorC<ArSZSeries> mySensorInterpTask;
  ArRetFunctorC<bool, ArSZSeries> myAriaExitCB;
};

#endif 
