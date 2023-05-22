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
#ifndef ARURG_2_0_H
#define ARURG_2_0_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ArLaser.h"
#include "Aria/ArDeviceConnection.h"

/** 
    Hokuyo URG laser range device (SCIP 2.0).

    Connects using the Urg's serial port connector or USB.  Note that
    if the max range is 4095 or less it'll use 2 bytes per range
    reading, and if the max range is more than that it'll use 3 bytes
    per range reading.

    Supports (probably) any URG using SCIP 2.0 protocol (use ArUrg aka
    'urg' for SCIP 1.1 instead). 
	
	See ArLaserConnector for instructions on configuring and using lasers.

	See http://robots.mobilerobots.com/wiki/Hokuyo_URG for more information and links to downloads from Hokuyo including the USB Windows driver.

    @sa ArUrg
    @sa ArLaserConnector
    @sa ArLaser
 */
class ArUrg_2_0 final : public ArLaser
{
public:
  /// Constructor
  explicit AREXPORT ArUrg_2_0(int laserNumber,
		 const char *name = "urg2.0");
  /// Destructor
  AREXPORT virtual ~ArUrg_2_0();
  AREXPORT virtual bool blockingConnect() override;
  AREXPORT virtual bool asyncConnect() override;
  AREXPORT virtual bool disconnect() override;
  AREXPORT virtual bool isConnected() override { return myIsConnected; }
  AREXPORT virtual bool isTryingToConnect() override 
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
private:
  /// Sets the parameters that control what data you get from the urg
  bool setParams(
	  double startingDegrees = -135, double endingDegrees = 135,
	  double incrementDegrees = 1, bool flipped = false);
  /// Sets the parameters that control what data you get from the urg
  bool setParamsBySteps(
	  int startingStep = 0, int endingStep = 768, int clusterCount = 3,
	  bool flipped = false);
  virtual void * runThread(void *arg) override;
  /// internal call to write a string to the urg
  bool writeLine(const char *str);
  /// internal call to read a string from the urg
  bool readLine(char *buf, unsigned int size, unsigned int msWait, 
		bool noChecksum, bool stripLastSemicolon, 
		ArTime *firstByte = NULL);

  /// internal call to write a command and get the response back into the buf
  bool sendCommandAndRecvStatus(
	  const char *command, const char *commandDesc, 
	  char *status, unsigned int size, unsigned int msWait);

  void sensorInterp();
  virtual void setRobot(ArRobot *robot) override;
  virtual bool laserCheckParams() override;
  virtual void laserSetName(const char *name) override;
  
  void failedToConnect();
  ArMutex myReadingMutex;
  ArMutex myDataMutex;

  ArTime myReadingRequested;
  std::string myReading;

  int myStartingStep;
  int myEndingStep;
  int myClusterCount;
  bool myFlipped;
  char myRequestString[1024];
  double myClusterMiddleAngle;

  bool internalConnect();

  bool internalGetReading();

  void clear();
  bool myIsConnected;
  bool myTryingToConnect;
  bool myStartConnect;
  
  std::string myVendor;
  std::string myProduct;
  std::string myFirmwareVersion;
  std::string myProtocolVersion;
  std::string mySerialNumber;
  std::string myStat;

  std::string myModel;
  unsigned int myDMin;
  unsigned int myDMax;
  int myARes;
  int myAMin;
  int myAMax;
  int myAFront;
  int myScan;

  double myStepSize;
  double myStepFirst;
  bool myUseThreeDataBytes;

  bool myLogMore;
  
  ArFunctorC<ArUrg_2_0> mySensorInterpTask;
  ArRetFunctorC<bool, ArUrg_2_0> myAriaExitCB;
};

#endif // ARURG_2_0_H
