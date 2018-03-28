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
#include "ArUrg_2_0.h"
#include "ArRobot.h"
#include "ArSerialConnection.h"
#include "ariaInternal.h"

AREXPORT ArUrg_2_0::ArUrg_2_0(int laserNumber, const char *name) :
  ArLaser(laserNumber, name, 262144),
  mySensorInterpTask(this, &ArUrg_2_0::sensorInterp),
  myAriaExitCB(this, &ArUrg_2_0::disconnect)
{
  clear();
  myRawReadings = NULL;

  Aria::addExitCallback(&myAriaExitCB, -10);

  laserSetName(getName());

  setSensorPosition(0, 0, 0);     

  laserAllowSetDegrees(-180, -180, 180, // start degrees
		  180, -180, 180); // end degrees

  laserAllowSetIncrement(1, 0, 180);
  
  std::list<std::string> baudChoices;
  baudChoices.push_back("0");
  baudChoices.push_back("019200");
  baudChoices.push_back("057600");
  baudChoices.push_back("115200");
  baudChoices.push_back("250000");
  baudChoices.push_back("500000");
  baudChoices.push_back("750000");
  laserAllowStartingBaudChoices("0", baudChoices);

  laserAllowAutoBaudChoices("0", baudChoices);


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
	  new ArDrawingData("polyDots", 
			    ArColor(0, 0xaa, 0xaa), 
			    70,  // mm diameter of dots
			    77), true);
  setCumulativeDrawingData(
	  new ArDrawingData("polyDots", 
			    ArColor(0, 0x55, 0x55), 
			    100,  // mm diameter of dots
			    62), true);

  myLogMore = false;
  //myLogMore = true;
}

AREXPORT ArUrg_2_0::~ArUrg_2_0()
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

AREXPORT void ArUrg_2_0::laserSetName(const char *name)
{
  myName = name;
  
  myConnMutex.setLogNameVar("%s::myConnMutex", getName());
  myReadingMutex.setLogNameVar("%s::myReadingMutex", getName());
  myDataMutex.setLogNameVar("%s::myDataMutex", getName());
  myAriaExitCB.setNameVar("%s::exitCallback", getName());
  
  ArLaser::laserSetName(getName());
}

AREXPORT bool ArUrg_2_0::setParams(
	double startingDegrees, double endingDegrees,
	double incrementDegrees, bool flipped)
{
  if (startingDegrees < -180.0 || startingDegrees > 180.0 || 
      endingDegrees < -180.0 || startingDegrees > 180.0 || 
      incrementDegrees < 0.0)
  {
    ArLog::log(ArLog::Normal, 
	       "%s::setParams: Bad params (starting %g ending %g incrementDegrees %g)",
	       getName(), startingDegrees, endingDegrees, incrementDegrees);
    return false;
  }
  double startingStepRaw;
  double endingStepRaw;
  int startingStep;
  int endingStep;
  int clusterCount;
  
  //startingStepRaw = -(startingDegrees - 135.0) / .3515625;
  startingStepRaw = -(startingDegrees - myStepFirst) / myStepSize;
  //endingStepRaw = -(endingDegrees - 135.0) / .3515625;
  endingStepRaw = -(endingDegrees - myStepFirst) / myStepSize;
  startingStep = (int)ceil(ArUtil::findMin(startingStepRaw, endingStepRaw));
  endingStep = (int)floor(ArUtil::findMax(startingStepRaw, endingStepRaw));
  //clusterCount = ArMath::roundInt(incrementDegrees / .3515625);
  clusterCount = ArMath::roundInt(incrementDegrees / myStepSize);
  if (clusterCount < 1)
    clusterCount = 1;

  ArLog::LogLevel logLevel;
  if (myLogMore)
    logLevel = ArLog::Normal;
  else
    logLevel = ArLog::Verbose;

  ArLog::log(logLevel, "%s: starting deg %.1f raw %.1f step %d ending deg %.1f raw %.1f step %d, cluster deg %f count %d flipped %d",
	     getName(), startingDegrees, startingStepRaw, startingStep, endingDegrees, endingStepRaw, endingStep, incrementDegrees, clusterCount, flipped);
  
  return setParamsBySteps(startingStep, endingStep, clusterCount, flipped);
}

AREXPORT bool ArUrg_2_0::setParamsBySteps(int startingStep, int endingStep, 
				      int clusterCount, bool flipped)
{
  if (startingStep > endingStep || clusterCount < 1)
  {
    ArLog::log(ArLog::Normal, 
	       "%s::setParamsBySteps: Bad params (starting %d ending %d clusterCount %d)",
	       getName(), startingStep, endingStep, clusterCount);
    return false;
  }

  if (startingStep < myAMin)
    startingStep = myAMin;
  if (endingStep > myAMax)
    endingStep = myAMax;

  myDataMutex.lock();
  myStartingStep = startingStep;
  myEndingStep = endingStep;
  myClusterCount = clusterCount;
  myFlipped = flipped;
  //sprintf(myRequestString, "G%03d%03d%02d", myStartingStep, endingStep,
  //clusterCount);

  int baudRate = 0;
  ArSerialConnection *serConn = NULL;
  serConn = dynamic_cast<ArSerialConnection *>(myConn);
  if (serConn != NULL)
    baudRate = serConn->getBaud();

  // only use the three data bytes if our range needs it, and if the baud rate can support it
  if (myMaxRange > 4095 && (baudRate == 0 || baudRate > 57600))
  {
    myUseThreeDataBytes = true;
    sprintf(myRequestString, "MD%04d%04d%02d%01d%02d", 
	    myStartingStep, myEndingStep, myClusterCount, 
	    0, // scan interval
	    0 // number of scans to send (forever)
	);
  }
  else
  {
    myUseThreeDataBytes = false;
    if (myMaxRange > 4094) 
      myMaxRange = 4094;
    sprintf(myRequestString, "MS%04d%04d%02d%01d%02d", 
	    myStartingStep, myEndingStep, myClusterCount, 
	    0, // scan interval
	    0 // number of scans to send (forever)
	);
  }

  myClusterMiddleAngle = 0;
  if (myClusterCount > 1)
    //myClusterMiddleAngle = myClusterCount * 0.3515625 / 2.0;
    myClusterMiddleAngle = myClusterCount * myStepSize / 2.0;

  if (myRawReadings != NULL)
  {
    ArUtil::deleteSet(myRawReadings->begin(), myRawReadings->end());
    myRawReadings->clear();
    delete myRawReadings;
    myRawReadings = NULL;
  }

  myRawReadings = new std::list<ArSensorReading *>;
  

  ArSensorReading *reading;
  int onStep;
  double angle;

  for (onStep = myStartingStep; 
       onStep < myEndingStep; 
       onStep += myClusterCount)
  {
    /// FLIPPED
    if (!myFlipped)
      //angle = ArMath::subAngle(ArMath::subAngle(135, 
      //                                          onStep * 0.3515625),
      angle = ArMath::subAngle(ArMath::subAngle(myStepFirst, 
						onStep * myStepSize),
			       myClusterMiddleAngle);
    else
      //angle = ArMath::addAngle(ArMath::addAngle(-135, 
      //                                          onStep * 0.3515625), 
      angle = ArMath::addAngle(ArMath::addAngle(-myStepFirst, 
						onStep * myStepSize), 
			       myClusterMiddleAngle);
			       
    reading = new ArSensorReading;
    reading->resetSensorPosition(ArMath::roundInt(mySensorPose.getX()),
				 ArMath::roundInt(mySensorPose.getY()),
				 ArMath::addAngle(angle, 
						  mySensorPose.getTh()));
    myRawReadings->push_back(reading);
    //printf("%.1f ", reading->getSensorTh());
  }


  myDataMutex.unlock();
  return true;
}

void ArUrg_2_0::clear(void)
{
  myIsConnected = false;
  myTryingToConnect = false;
  myStartConnect = false;

  myVendor = "";
  myProduct = "";
  myFirmwareVersion = "";
  myProtocolVersion = "";
  mySerialNumber = "";
  myStat = "";

  myModel = "";
  myDMin = 0;
  myDMax = 0;
  myARes = 0;
  myAMin = 0;
  myAMax = 0;
  myAFront = 0;
  myScan = 0;
}

AREXPORT void ArUrg_2_0::log(void)
{
  ArLog::log(ArLog::Normal, "URG %s:", getName());
  ArLog::log(ArLog::Normal, "Vendor information: %s", myVendor.c_str());
  ArLog::log(ArLog::Normal, "Product information: %s", myProduct.c_str());
  ArLog::log(ArLog::Normal, "Firmware version: %s", myFirmwareVersion.c_str());
  ArLog::log(ArLog::Normal, "Protocol Version: %s", myProtocolVersion.c_str());
  ArLog::log(ArLog::Normal, "Serial number: %s", mySerialNumber.c_str());
  if (!myStat.empty())
    ArLog::log(ArLog::Normal, "Stat: %s", myStat.c_str());
  ArLog::log(ArLog::Normal, "Model: %s:", myModel.c_str());
  ArLog::log(ArLog::Normal, "Distance min: %d", myDMin);
  ArLog::log(ArLog::Normal, "Distance max: %d", myDMax);
  ArLog::log(ArLog::Normal, "Angular resolution: %d", myARes);
  ArLog::log(ArLog::Normal, "Angle min step: %d", myAMin);
  ArLog::log(ArLog::Normal, "Angle max step: %d", myAMax);
  ArLog::log(ArLog::Normal, "Angle front step: %d", myAFront);
  ArLog::log(ArLog::Normal, "Scanning speed: %d", myScan);

  ArLog::log(ArLog::Normal, "Calculated first step: %g", myStepFirst);
  ArLog::log(ArLog::Normal, "Calculated step size: %g", myStepSize);
  
}

AREXPORT void ArUrg_2_0::setRobot(ArRobot *robot)
{
  myRobot = robot;
  if (myRobot != NULL)
    myRobot->addSensorInterpTask("urg2.0", 90, &mySensorInterpTask);
  ArRangeDevice::setRobot(robot);
}

bool ArUrg_2_0::writeLine(const char *str)
{
  if (myConn == NULL)
  {
    ArLog::log(ArLog::Terse, 
	       "%s: Attempt to write string to null connection", getName());
    return false;
  }

  bool ret = true;
  size_t len = strlen(str);

  myConnMutex.lock();
  if (myConn->write(str, len) < len || 
      myConn->write("\n", 1) < 1)
    ret = false;
  myConnMutex.unlock();
  return ret;
}

/**
   @param buf the buffer to put the data into
   @param size the size of the buffer
   @param msWait how long to wait for the data
   @param noChecksum whether there is a checksum on this data or not
   (there isn't on command echos)
   @param stripLastSemicolon Some responses (to VV and PP) have a
   semicolon to separate the string from the checksum... but that
   semicolon is NOT included in the checksum, and shouldn't be
   included in the string...   
   @param firstByte if given record time at which first byte was received to
this ArTime objects.
**/
bool ArUrg_2_0::readLine(char *buf, unsigned int size, 
			 unsigned int msWait, bool noChecksum, 
			 bool stripLastSemicolon, ArTime *firstByte)
{
  if (myConn == NULL)
  {
    ArLog::log(ArLog::Terse, 
	       "%s: Attempt to read line from null connection", getName());
    return false;
  }

  ArTime started;
  started.setToNow();

  buf[0] = '\0';
  unsigned int onChar = 0;
  int ret;

  //long int rawCheckSum = 0;
  unsigned char rawCheckSum = 0;
  char checkSum;
  unsigned int i;
  unsigned int iMax;

  myConnMutex.lock();
  while ((msWait == 0 || started.mSecSince() < (int)msWait) && 
	 onChar < size)
  {
    if ((ret = myConn->read(&buf[onChar], 1, 0)) > 0)
    {
      if (onChar == 0 && firstByte != NULL)
	firstByte->setToNow();
      if (buf[onChar] == '\n' || 
	  buf[onChar] == '\r')
      {
	//buf[onChar+1] = '\0';
	buf[onChar] = '\0';
	if (!noChecksum && onChar >= 1)
	{
	  if (stripLastSemicolon &&
	      onChar > 2 && buf[onChar - 2] == ';')
	    iMax = onChar - 2;
          else
	    iMax = onChar - 1;

	  // find the checksum 
	  for (i = 0; i < iMax; i++)
	    rawCheckSum += buf[i];

	  // see if it matches onChar - 1, then NULL out onchar -1
	  checkSum = (rawCheckSum & 0x3f) + 0x30;
	  
	  if ((checkSum) != buf[onChar - 1])
	  {
	    ArLog::log(ArLog::Normal, 
		       "%s: Bad checksum on '%s' it should be %c", 
		       getName(), buf, checkSum);
	    myConnMutex.unlock();
	    return false;
	  }
	  // null out the checksum so it doesn't mess up other parsing
	  buf[onChar - 1] = '\0';

	  if (stripLastSemicolon &&
	      onChar >= 2 && buf[onChar - 2] == ';')
	  {
	    buf[onChar - 2] = '\0';
	  }
	}
	if (myLogMore)
	  ArLog::log(ArLog::Normal, "%s: '%s'", getName(), buf);
	myConnMutex.unlock();
	return true;
      }
      onChar++;
    }
    if (ret < 0)
    {
      ArLog::log(ArLog::Normal, "%s: bad ret", getName());
      myConnMutex.unlock();
      return false;
    }
    if (ret == 0)
      ArUtil::sleep(1);
  }
  myConnMutex.unlock();
  return false;
}

bool ArUrg_2_0::sendCommandAndRecvStatus(
	const char *command, const char *commandDesc, 
	char *buf, unsigned int size, unsigned int msWait)
{
  ArTime start;

  // send the command
  if (!writeLine(command))
  {
    ArLog::log(ArLog::Normal, "%s: Could not send %s", getName(), 
	       commandDesc);
    return false;
  }

  // try and read the command back (no checksum on the echo)
  if (!readLine(buf, size, msWait, true, false))
  {
    ArLog::log(ArLog::Normal, "%s: Received no response to %s (in %d but really %d)", 
	       getName(), commandDesc, msWait, start.mSecSince());
    return false;
  }

  // make sure the command back matches
  if (strcasecmp(buf, command) != 0)
  {
    ArLog::log(ArLog::Normal, "%s: Received bad echo to %s (command %s got back %s)", 
	       getName(), commandDesc, command, buf);
    return false;
  }

  // get the status from that command back
  if (!readLine(buf, size, msWait, false, false))
  {
    ArLog::log(ArLog::Normal, 
	       "%s: Did not receive status back from %s",
	       getName(), commandDesc);
    return false;
  }

  return true;
}

AREXPORT bool ArUrg_2_0::blockingConnect(void)
{
  if (!getRunning())
    runAsync();

  myConnMutex.lock();
  if (myConn == NULL)
  {
    ArLog::log(ArLog::Terse, 
	       "%s: Could not connect because there is no connection defined", 
	       getName());
    myConnMutex.unlock();
    failedToConnect();
    return false;
  }

  ArSerialConnection *serConn = NULL;
  serConn = dynamic_cast<ArSerialConnection *>(myConn);

  // if we have a starting baud and are a serial port, then change the
  // baud rate... not by default this will set it to 0 baud which'll
  // cause the serial stuff not to touch it
  if (serConn != NULL)
    serConn->setBaud(atoi(getStartingBaudChoice()));

  if (myConn->getStatus() != ArDeviceConnection::STATUS_OPEN && 
      !myConn->openSimple())
  {
    ArLog::log(ArLog::Terse, 
	       "%s: Could not connect because the connection was not open and could not open it", 
	       getName());
    myConnMutex.unlock();
    failedToConnect();
    return false;
  }
  myConnMutex.unlock();

  lockDevice();
  myTryingToConnect = true;
  unlockDevice();

  laserPullUnsetParamsFromRobot();
  laserCheckParams();
  
  ArUtil::sleep(100);

  bool connected = false;

  if (internalConnect())
    connected = true;

  if (connected)
  {
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
    failedToConnect();
    return false;
  }
}


bool ArUrg_2_0::internalConnect(void)

{
  bool ret = true;
  char buf[1024];
  

  ArSerialConnection *serConn = NULL;
  serConn = dynamic_cast<ArSerialConnection *>(myConn);

  bool alreadyAtAutobaud = false;



  // empty the buffer...
  /*
  sendCommandAndRecvStatus(
	  "RS", "reset", 
	  buf, sizeof(buf), 1000);
  readLine(buf, sizeof(buf), 1, true, false);

  sendCommandAndRecvStatus(
	  "SCIP2.0", "SCIP 2.0 request", 
	  buf, sizeof(buf), 1000);
  */

  writeLine("RS");
  ArUtil::sleep(100);

  writeLine("SCIP2.0");
  ArUtil::sleep(100);

  ArTime startedFlushing;

  while (readLine(buf, sizeof(buf), 1, true, false) ||
	 startedFlushing.mSecSince() < 1000);

  buf[0] = '\0';

  if (!(ret = sendCommandAndRecvStatus(
		"VV", "version request", 
		buf, sizeof(buf), 10000)) || 
      strcasecmp(buf, "00") != 0)      
  {
    // if we didn't get it and have an autobaud, try it at what the autobaud rate is
    if (serConn != NULL && atoi(getAutoBaudChoice()) > 0)
    {
      alreadyAtAutobaud = true;
      serConn->setBaud(atoi(getAutoBaudChoice()));
      ArUtil::sleep(100);

      writeLine("RS");
      ArUtil::sleep(100);
      
      writeLine("SCIP2.0");
      ArUtil::sleep(100);
      
      startedFlushing.setToNow();
      while (readLine(buf, sizeof(buf), 1, true, false) ||
	     startedFlushing.mSecSince() < 1000);
      
      if (!(ret = sendCommandAndRecvStatus(
		"VV", "version request after falling back to autobaudchoice", 
		buf, sizeof(buf), 10000)) || 
	  strcasecmp(buf, "00") != 0)      
      {
	if (ret && strcasecmp(buf, "00") != 0)      
	  ArLog::log(ArLog::Normal, 
		     "%s::blockingConnect: Bad status on version response after falling back to autobaudchoice", 
		     getName());
	return false;
      }
    }
    // if we don't have a serial port or no autobaud then we can't
    // change the baud, so just fail
    else
    {
      if (ret && strcasecmp(buf, "00") != 0)      
	ArLog::log(ArLog::Normal, 
		   "%s::blockingConnect: Bad status on version response (%s)",
		   getName(), buf);
      return false;
    }
  }

  // if we want to autobaud, then give it a whirl
  if (!alreadyAtAutobaud && serConn != NULL && atoi(getAutoBaudChoice()) > 0)
  {

    // empty the buffer from the last version request
    while (readLine(buf, sizeof(buf), 100, true, false));

    // now change the baud...
    sprintf(buf, "SS%06d", atoi(getAutoBaudChoice()));
    if (!writeLine(buf))
      return false;

    ArUtil::sleep(100);

    //serConn->setBaud(115200);
    serConn->setBaud(atoi(getAutoBaudChoice()));
    // wait a second for the baud to change...
    ArUtil::sleep(100);

    // empty the buffer from the baud change
    while (readLine(buf, sizeof(buf), 100, true, false));
    
    if (!(ret = sendCommandAndRecvStatus(
		  "VV", "version request after switching to autobaudchoice", 
		  buf, sizeof(buf), 10000)) || 
	strcasecmp(buf, "00") != 0)      
    {
      if (ret && strcasecmp(buf, "00") != 0)      
	ArLog::log(ArLog::Normal, 
		   "%s::blockingConnect: Bad status on version response after switching to autobaudchoice", 
		   getName());
      return false;
    }

    ArLog::log(ArLog::Verbose, "%s: Switched to %s baud rate",
	       getName(), getAutoBaudChoice());
  }

  while (readLine(buf, sizeof(buf), 10000, false, true))
  {
    if (strlen(buf) == 0)
      break;

    if (strncasecmp(buf, "VEND:", strlen("VEND:")) == 0)
      myVendor = &buf[5];
    else if (strncasecmp(buf, "PROD:", strlen("PROD:")) == 0)
      myProduct = &buf[5];
    else if (strncasecmp(buf, "FIRM:", strlen("FIRM:")) == 0)
      myFirmwareVersion = &buf[5];
    else if (strncasecmp(buf, "PROT:", strlen("PROT:")) == 0)
      myProtocolVersion = &buf[5];
    else if (strncasecmp(buf, "SERI:", strlen("SERI:")) == 0)
      mySerialNumber = &buf[5];
    else if (strncasecmp(buf, "STAT:", strlen("STAT:")) == 0)
      myStat = &buf[5];
  }

  if (myVendor.empty() || myProduct.empty() || myFirmwareVersion.empty() || 
      myProtocolVersion.empty() || mySerialNumber.empty())
  {
    ArLog::log(ArLog::Normal, 
	       "%s::blockingConnect: Missing information in version response",
	       getName());
    return false;
  }

  if (!(ret = sendCommandAndRecvStatus(
		"PP", "parameter info request", 
		buf, sizeof(buf), 10000)) || 
      strcasecmp(buf, "00") != 0)      
  {
    ArLog::log(ArLog::Normal, 
	       "%s::blockingConnect: Bad response to parameter info request",
	       getName());
    return false;
  }

  while (readLine(buf, sizeof(buf), 10000, false, true))
  {
    if (strlen(buf) == 0)
      break;

    if (strncasecmp(buf, "MODL:", strlen("MODL:")) == 0)
      myModel = &buf[5];
    else if (strncasecmp(buf, "DMIN:", strlen("DMIN:")) == 0)
      myDMin = atoi(&buf[5]);
    else if (strncasecmp(buf, "DMAX:", strlen("DMAX:")) == 0)
      myDMax = atoi(&buf[5]);
    else if (strncasecmp(buf, "ARES:", strlen("ARES:")) == 0)
      myARes = atoi(&buf[5]);
    else if (strncasecmp(buf, "AMIN:", strlen("AMIN:")) == 0)
      myAMin = atoi(&buf[5]);
    else if (strncasecmp(buf, "AMAX:", strlen("AMAX:")) == 0)
      myAMax = atoi(&buf[5]);
    else if (strncasecmp(buf, "AFRT:", strlen("AFRT:")) == 0)
      myAFront = atoi(&buf[5]);
    else if (strncasecmp(buf, "SCAN:", strlen("SCAN:")) == 0)
      myScan = atoi(&buf[5]);
  }

  if (myModel.empty() || myDMin == 0 || myDMax == 0 || myARes == 0 ||
      myAMin == 0 || myAMax == 0 || myAFront == 0 || myScan == 0)
  {
    ArLog::log(ArLog::Normal, 
	       "%s::blockingConnect: Missing information in parameter info response",
	       getName());
    //log();
    return false;
  }

  myStepSize = 360.0 / myARes;
  myStepFirst = myAFront * myStepSize;
  
  if (myMaxRange > myDMax)
    setMaxRange(myDMax);

  //log();

  setParams(getStartDegrees(), getEndDegrees(), getIncrement(), getFlipped());

  //myLogMore = true;
  //  myLogMore = false;
  ArUtil::sleep(100);
  
  
  //printf("myRequestString %s\n", myRequestString);

  if (!(ret = sendCommandAndRecvStatus(
		myRequestString, "request distance reading", 
		buf, sizeof(buf), 10000)) || 
      strcasecmp(buf, "00") != 0)
  {
    if (ret && strcasecmp(buf, "00") != 0) 
      ArLog::log(ArLog::Normal, 
	 "%s::blockingConnect: Bad status on distance reading response (%s)",
		 getName(), buf);
    return false;
  }
  
  //myLogMore = false;

  ArTime started;
  started.setToNow();
  while (started.secSince() < 10 && 
	 readLine(buf, sizeof(buf), 10000, true, false))
  {
    if (strlen(buf) == 0)
      return true;
  }

  ArLog::log(ArLog::Normal, "%s::blockingConnect: Did not get distance reading back",
	     getName());
  return false;
}

AREXPORT bool ArUrg_2_0::asyncConnect(void)
{
  myStartConnect = true;
  if (!getRunning())
    runAsync();
  return true;
}

AREXPORT bool ArUrg_2_0::disconnect(void)
{
  if (!isConnected())
    return true;

  ArLog::log(ArLog::Normal, "%s: Disconnecting", getName());

  bool ret;

  ret = writeLine("RS");

  char buf[2048];
  sprintf(buf, "SS%06d", atoi(getStartingBaudChoice()));
  ret = (ret & writeLine(buf));

  laserDisconnectNormally();
  return ret;

}


void ArUrg_2_0::sensorInterp(void)
{
  ArTime readingRequested;
  std::string reading;
  myReadingMutex.lock();
  if (myReading.empty())
  {
    myReadingMutex.unlock();
    return;
  }

  readingRequested = myReadingRequested;
  reading = myReading;
  myReading = "";
  myReadingMutex.unlock();

  ArTime time = readingRequested;
  ArPose pose;
  int ret;
  int retEncoder;
  ArPose encoderPose;

  //time.addMSec(-13);
  if (myRobot == NULL || !myRobot->isConnected())
  {
    pose.setPose(0, 0, 0);
    encoderPose.setPose(0, 0, 0);
  } 
  else if ((ret = myRobot->getPoseInterpPosition(time, &pose)) < 0 ||
	   (retEncoder = 
	    myRobot->getEncoderPoseInterpPosition(time, &encoderPose)) < 0)
  {
    ArLog::log(ArLog::Normal, "%s: reading too old to process", getName());
    return;
  }

  ArTransform transform;
  transform.setTransform(pose);

  unsigned int counter = 0; 
  if (myRobot != NULL)
    counter = myRobot->getCounter();

  lockDevice();
  myDataMutex.lock();

  //double angle;
  int i;
  int len = reading.size();

  int range;
  int giant;
  int big; 
  int little;
  //int onStep;

  std::list<ArSensorReading *>::reverse_iterator it;
  ArSensorReading *sReading;
  
  int iMax;
  int iIncr;

  if (myUseThreeDataBytes)
  {
    iMax = len - 2;
    iIncr = 3;
  }
  else
  {
    iMax = len - 1;
    iIncr = 2;
  }

  bool ignore;
  for (it = myRawReadings->rbegin(), i = 0; 
       it != myRawReadings->rend() && i < iMax; //len - 2; 
       it++, i += iIncr) //3)
  {
    ignore = false;

    if (myUseThreeDataBytes)
    {
      giant = reading[i] - 0x30;
      big = reading[i+1] - 0x30;
      little = reading[i+2] - 0x30;
      range = (giant << 12 | big << 6 | little);
    }
    else
    {
      big = reading[i] - 0x30;
      little = reading[i+1] - 0x30;
      range = (big << 6 | little);
    }
    
    if (range < myDMin)
      range = myDMax+1;

    sReading = (*it);
    sReading->newData(range, pose, encoderPose, transform, counter, 
		      time, ignore, 0);
  }

  myDataMutex.unlock();

  int previous = getCumulativeBuffer()->size();
  laserProcessReadings();
  int now = getCumulativeBuffer()->size();

  unlockDevice();
}

AREXPORT void * ArUrg_2_0::runThread(void *arg)
{
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

    // if we have a robot but it isn't running yet then don't have a
    // connection failure
    if (laserCheckLostConnection())
    {
      ArLog::log(ArLog::Terse, 
		 "%s:  Lost connection to the laser because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(), 
		 myLastReading.mSecSince()/1000.0, 
		 getConnectionTimeoutSeconds());
      myIsConnected = false;
      laserDisconnectOnError();
      continue;
    }

    internalGetReading();

    ArUtil::sleep(1);
  }
  return NULL;
}

bool ArUrg_2_0::internalGetReading(void)
{
  ArTime readingRequested;
  std::string reading;
  char buf[1024];

  reading = "";
  /*
  if (!writeLine(myRequestString))
  {
    ArLog::log(ArLog::Terse, "Could not send request distance reading to urg");
    return false;
  }
  */

  ArTime firstByte;

  if (!readLine(buf, sizeof(buf), 1000, true, false, &firstByte) || 
      strcasecmp(buf, myRequestString) != 0)
  {
    ArLog::log(ArLog::Normal, 
	       "%s: Did not get distance reading response (%s)",
	       getName(), buf);
    return false;
  }
  // TODO this isn't the right time, but for most of what we do that
  // won't matter
  readingRequested.setToNow();
  readingRequested.addMSec(-100);

  if (!readLine(buf, sizeof(buf), 100, false, false) || 
      strcasecmp(buf, "99") != 0)		  
  {
    ArLog::log(ArLog::Normal, 
	       "%s: Bad status on distance reading response (%s)",
	       getName(), buf);
    return false;
  }

  if (!readLine(buf, sizeof(buf), 100, false, false))
  {
    ArLog::log(ArLog::Normal, 
       "%s: Could not read timestamp in distance reading response (%s)",
	       getName(), buf);
    return false;
  }
  
  while (readLine(buf, sizeof(buf), 100, false, false))
  {
    if (strlen(buf) == 0)
    {
      myReadingMutex.lock();
      myReadingRequested = readingRequested;
      myReading = reading;
      myReadingMutex.unlock();	
      if (myRobot == NULL)
	sensorInterp();
      return true;
    }
    else
    {
      reading += buf;
    }
  }

  return false;
}

void ArUrg_2_0::failedToConnect(void)
{
  lockDevice();
  myTryingToConnect = true;
  unlockDevice();
  laserFailedConnect();
}

bool ArUrg_2_0::laserCheckParams(void)
{
  return true;
}
