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
#include "ArSimulatedLaser.h"
#include "ArRobot.h"


AREXPORT ArSimulatedLaser::ArSimulatedLaser(ArLaser *laser) :
  ArLaser(laser->getLaserNumber(),
	  laser->getName(),
	  laser->getAbsoluteMaxRange(),
	  laser->isLocationDependent(),
	  false),
  mySimPacketHandler(this, &ArSimulatedLaser::simPacketHandler)
{
  myLaser = laser;
  
  std::string name;
  name = "sim_";
  name += getName();
  laserSetName(name.c_str());

  /*
  printf("@@@@@@ %d %d @ %d %d\n", 
	 laser->getCurrentRangeBuffer()->getSize(),
	 laser->getCumulativeRangeBuffer()->getSize(),
	 getCurrentRangeBuffer()->getSize(),
	 getCumulativeRangeBuffer()->getSize());
  */

  // laser parameters
  setInfoLogLevel(myLaser->getInfoLogLevel());
  setConnectionTimeoutSeconds(myLaser->getConnectionTimeoutSeconds());
  setCumulativeCleanDist(myLaser->getCumulativeCleanDist());
  setCumulativeCleanInterval(myLaser->getCumulativeCleanInterval());
  setCumulativeCleanOffset(myLaser->getCumulativeCleanOffset());

  setSensorPosition(myLaser->getSensorPosition());
  laserSetAbsoluteMaxRange(myLaser->getAbsoluteMaxRange());
  setMaxRange(myLaser->getMaxRange());
  
  // base range device parameters
  setMaxSecondsToKeepCurrent(myLaser->getMaxSecondsToKeepCurrent());
  setMinDistBetweenCurrent(getMinDistBetweenCurrent());
  setMaxSecondsToKeepCumulative(myLaser->getMaxSecondsToKeepCumulative());
  setMaxDistToKeepCumulative(myLaser->getMaxDistToKeepCumulative());
  setMinDistBetweenCumulative(myLaser->getMinDistBetweenCumulative());
  setMaxInsertDistCumulative(myLaser->getMaxInsertDistCumulative());
  setCurrentDrawingData(myLaser->getCurrentDrawingData(), false);
  setCumulativeDrawingData(myLaser->getCumulativeDrawingData(), false);

  // now all the specific laser settings
  if (myLaser->canSetDegrees())
    laserAllowSetDegrees(
	    myLaser->getStartDegrees(), myLaser->getStartDegreesMin(), 
	    myLaser->getStartDegreesMax(), myLaser->getEndDegrees(), 
	    myLaser->getEndDegreesMin(), myLaser->getEndDegreesMax());

  if (myLaser->canChooseDegrees())
    laserAllowDegreesChoices(myLaser->getDegreesChoice(), 
			myLaser->getDegreesChoicesMap());

  if (myLaser->canSetIncrement())
    laserAllowSetIncrement(myLaser->getIncrement(), 
			   myLaser->getIncrementMin(), 
			   myLaser->getIncrementMax());

  if (myLaser->canChooseIncrement())
    laserAllowIncrementChoices(myLaser->getIncrementChoice(), 
			  myLaser->getIncrementChoicesMap());

  if (myLaser->canChooseUnits())
    laserAllowUnitsChoices(myLaser->getUnitsChoice(), 
			   myLaser->getUnitsChoices());

  if (myLaser->canChooseReflectorBits())
    laserAllowReflectorBitsChoices(myLaser->getReflectorBitsChoice(), 
			      myLaser->getReflectorBitsChoices());
  
  if (canSetPowerControlled())
    laserAllowSetPowerControlled(myLaser->getPowerControlled());

  if (myLaser->canChooseStartingBaud())
    laserAllowStartingBaudChoices(myLaser->getStartingBaudChoice(), 
			      myLaser->getStartingBaudChoices());

  if (myLaser->canChooseAutoBaud())
    laserAllowAutoBaudChoices(myLaser->getAutoBaudChoice(), 
			      myLaser->getAutoBaudChoices());
  
  laserSetDefaultTcpPort(myLaser->getDefaultTcpPort());
  laserSetDefaultPortType(myLaser->getDefaultPortType());

  // actual stuff for the sim
  myStartConnect = false;
  myIsConnected = false;
  myTryingToConnect = false;
  myAssembleReadings = new std::list<ArSensorReading *>;
  myCurrentReadings = new std::list<ArSensorReading *>;
  myRawReadings = myCurrentReadings;
  myIter = myAssembleReadings->begin();
  myDataCBList.setLogging(false);
  myReceivedData = false;
}

AREXPORT ArSimulatedLaser::~ArSimulatedLaser()
{

}

AREXPORT bool ArSimulatedLaser::blockingConnect(void)
{
  if (myLaserNumber != 1)
  {
    ArLog::log(ArLog::Normal, "%s: Cannot use the simulator with multiple lasers yet", getName());
    laserFailedConnect();
    return false;
  }

  if (!finishParams())
  {
    laserFailedConnect();
    return false;
  }

  if (myRobot == NULL)
  {
    ArLog::log(ArLog::Normal, 
	       "%s: Cannot connect to simulated laser because it has no robot",
	       getName());
    laserFailedConnect();
    return false;
  }
  
  lockDevice();
  
  if (canSetDegrees())
  {
    mySimBegin = ArUtil::findMin(getStartDegrees(), getEndDegrees());
    mySimEnd = ArUtil::findMax(getStartDegrees(), getEndDegrees());
  }
  else if (canChooseDegrees())
  {
    mySimBegin = -getDegreesChoiceDouble() / 2.0;
    mySimEnd = getDegreesChoiceDouble() / 2.0;
  }
  else
  {
    ArLog::log(ArLog::Normal, 
	   "%s: This laser type does not have field of view (start/end degrees) parameters configured, and does not have any defaults, cannot configure the simulated laser. Failing connection",
	       getName());
    unlockDevice();
    laserFailedConnect();
    return false;
  }

  if (canSetIncrement())
  {
    mySimIncrement = fabs(getIncrement());
  }
  else if (canChooseIncrement())
  {
    mySimIncrement = fabs(getIncrementChoiceDouble());
  }
  else
  {
    ArLog::log(ArLog::Normal, 
	   "%s: This laser type does not have increment (resolution) parameter configured, and does not have a default, cannot configure the simulated laser. Failing connection",
	       getName());
    unlockDevice();
    laserFailedConnect();
    return false;
  }
  unlockDevice();

  myRobot->lock();

  mySimPacketHandler.setName(getName());
  myRobot->remPacketHandler(&mySimPacketHandler);
  myRobot->addPacketHandler(&mySimPacketHandler, ArListPos::LAST);

  bool failed = false;
  bool robotIsRunning = myRobot->isRunning();

  // return true if we could send all the commands
  if (!failed && !myRobot->comInt(36, ArMath::roundInt(mySimBegin)))
    failed = true;
  if (!failed && !myRobot->comInt(37, ArMath::roundInt(mySimEnd)))
    failed = true;
  if (!failed && !myRobot->comInt(38, 
				  ArMath::roundInt(mySimIncrement * 100.0)))
    failed = true;
  // Enable sending data, with extended info 
  ///@todo only choose extended info if reflector bits desired, also shorten range.
  if (!failed && !myRobot->comInt(35, 2))
    failed = true;

  myRobot->unlock();

  if (robotIsRunning)
  {
    ArTime startWait;
    while (!failed && !myReceivedData)
    {
      if (startWait.secSince() >= 30)
	failed = true;
    }
    if (!failed && myReceivedData)
    {
      ArLog::log(ArLog::Verbose, "%s::blockingConnect: Got data back", 
		 getName());
    }
  }
  else
  {
    ArLog::log(ArLog::Normal, "%s::blockingConnect: Robot isn't running so can't wait for data", getName());
  }


  if (!failed && (!robotIsRunning || (robotIsRunning && myReceivedData)))
  {
    lockDevice();
    myIsConnected = true;
    myTryingToConnect = false;
    //madeConnection();
    ArLog::log(ArLog::Terse, "%s: Connected to simulated laser.",
	       getName());
    unlockDevice();
    laserConnect();
    return true;
  }
  else
  {
    //failedConnect();
    lockDevice();
    myIsConnected = false;
    myTryingToConnect = false;
    unlockDevice();
    ArLog::log(ArLog::Terse, 
	       "%s: Failed to connect to simulated laser.",
	       getName());
    laserFailedConnect();
    return false;
  }
}

AREXPORT bool ArSimulatedLaser::asyncConnect(void)
{
  if (myLaserNumber != 1)
  {
    ArLog::log(ArLog::Normal, "%s: Cannot use the simulator with multiple lasers yet", getName());
    return false;
  }

  if (!finishParams())
    return false;

  myStartConnect = true;
  return true;
}

AREXPORT bool ArSimulatedLaser::disconnect(void)
{
  laserDisconnectNormally();  
  return true;
}

AREXPORT bool ArSimulatedLaser::finishParams(void)
{
  if (!getRunning())
    runAsync();

  if (!laserPullUnsetParamsFromRobot())
  {
    ArLog::log(ArLog::Normal, "%s: Couldn't pull params from robot",
	       getName());
    return false;
  }

  return laserCheckParams();
}

AREXPORT bool ArSimulatedLaser::laserCheckParams(void)
{
  if (canSetDegrees() && (!myLaser->setStartDegrees(getStartDegrees()) || 
			  !myLaser->setEndDegrees(getEndDegrees())))
    return false;

  if (canChooseDegrees() && !myLaser->chooseDegrees(getDegreesChoice()))
    return false;
			
  if (canSetIncrement() && !myLaser->setIncrement(getIncrement()))
    return false;

  if (canChooseIncrement() && !myLaser->chooseIncrement(getIncrementChoice()))
    return false;

  if (canChooseUnits() && !myLaser->chooseUnits(getUnitsChoice()))
    return false;

  if (canChooseReflectorBits() && 
      !myLaser->chooseReflectorBits(getReflectorBitsChoice()))
    return false;

  if (canSetPowerControlled() && 
      !myLaser->setPowerControlled(getPowerControlled()))
    return false;

  if (canChooseStartingBaud() && 
      !myLaser->chooseStartingBaud(getStartingBaudChoice()))
    return false;

  if (canChooseAutoBaud() && 
      !myLaser->chooseAutoBaud(getAutoBaudChoice()))
    return false;

  if (!myLaser->laserCheckParams())
    return false;
  
  laserSetAbsoluteMaxRange(myLaser->getAbsoluteMaxRange());

  return true;
}

AREXPORT void *ArSimulatedLaser::runThread(void *arg)
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
      ArUtil::sleep(100);
      continue;
    }
    unlockDevice();

    if (!myIsConnected)
    {
      ArUtil::sleep(100);
      continue;
    }

    if (getConnectionTimeoutSeconds() > 0 && 
	getLastReadingTime().secSince() > getConnectionTimeoutSeconds())
    {
      ArLog::log(ArLog::Terse, 
		 "%s:  Lost connection to the laser because of error.  Nothing received for %g seconds (greater than the timeout of %g).", getName(), 
		 myLastReading.mSecSince()/1000.0, 
		 getConnectionTimeoutSeconds());
      myIsConnected = false;
      laserDisconnectOnError();
      continue;
    }

    ArUtil::sleep(100);
    continue;
  }

  return NULL;
}

/** @internal */
AREXPORT bool ArSimulatedLaser::simPacketHandler(ArRobotPacket *packet)
{
  std::list<ArFunctor *>::iterator it;

  unsigned int totalNumReadings;
  unsigned int readingNumber;
  double atDeg;
  unsigned int i;
  ArSensorReading *reading;
  std::list<ArSensorReading *>::iterator tempIt;
  unsigned int newReadings;
  int range;
  int refl = 0;
  ArPose encoderPose;
  //std::list<double>::iterator ignoreIt;  
  bool ignore;

  if (packet->getID() != 0x60 && packet->getID() != 0x61)
    return false;

  myReceivedData = true;

  bool isExtendedPacket = (packet->getID() == 0x61);
   
  // if we got here, its the right type of packet

  //printf("Got in a packet from the simulator\n");
  lockDevice();

  if(!isExtendedPacket)
  {
    // ignore the positional information
    packet->bufToByte2();
    packet->bufToByte2();
    packet->bufToByte2();
  }
  totalNumReadings = packet->bufToByte2(); // total for this reading
  readingNumber = packet->bufToByte2(); // which one we're on in this packet
  newReadings = packet->bufToUByte(); // how many are in this packet
  if (readingNumber == 0)
  {
    mySimPacketStart = myRobot->getPose();
    mySimPacketTrans = myRobot->getToGlobalTransform();
    mySimPacketEncoderTrans = myRobot->getEncoderTransform();
    mySimPacketCounter = myRobot->getCounter();
  }
  //printf("ArSimulatedLaser::simPacketHandler: On reading number %d out of %d, new %d\n", readingNumber, totalNumReadings, newReadings);
  // if we have too many readings in our list of raw readings, pop the extras
  while (myAssembleReadings->size() > totalNumReadings)
  {
    ArLog::log(ArLog::Terse, "ArSimulatedLaser::simPacketHandler, too many readings, popping one.");
    tempIt = myAssembleReadings->begin();
    if (tempIt != myAssembleReadings->end())
      delete (*tempIt);
    myAssembleReadings->pop_front();
  }
  
  // If we don't have any sensor readings created at all, make 'em all now
  if (myAssembleReadings->size() == 0)
    for (i = 0; i < totalNumReadings; i++)
      myAssembleReadings->push_back(new ArSensorReading);
  
  // Okay, we know where we're at, so get an iterator to the right spot, or 
  // make sure the one we keep around is in the right spot... if neither of
  // these trigger, then the iter should be in the right spot
  if ((readingNumber != myWhichReading + 1) || 
      totalNumReadings != myTotalNumReadings)
  {
    //printf("2\n");
    myWhichReading = readingNumber;
    myTotalNumReadings = totalNumReadings;
    for (i = 0, myIter = myAssembleReadings->begin(); i < readingNumber; i++)
    {
      tempIt = myIter;
      tempIt++;
      if (tempIt == myAssembleReadings->end() && (i + 1 != myTotalNumReadings))
      {
	myAssembleReadings->push_back(new ArSensorReading);
	//printf("@\n");
      }
      myIter++;
    }
  }
  else
  {
    //printf("3\n");
    myWhichReading = readingNumber;
  }

  //atDeg = (mySensorPose.getTh() - myOffsetAmount + 
  //readingNumber * myIncrementAmount);
  atDeg = (mySensorPose.getTh() + mySimBegin + 
	   readingNumber * mySimIncrement);
  //printf("4\n");
  encoderPose = mySimPacketEncoderTrans.doInvTransform(mySimPacketStart);
  // while we have in the readings and have stuff left we can read 
  for (i = 0; 
       //	 (myWhichReading < myTotalNumReadings && 
       //	  packet->getReadLength() < packet->getLength() - 4);
       i < newReadings;
       i++, myWhichReading++, atDeg += mySimIncrement)
       //i++, myWhichReading++, atDeg += myIncrementAmount)
  {
    reading = (*myIter);
    range = packet->bufToUByte2();
    if(isExtendedPacket)
    {
      refl = packet->bufToUByte();
      packet->bufToUByte(); // don't need this byte for anything yet
      packet->bufToUByte(); // don't need this byte for anything yet
    }
    ignore = false;

    /*
    for (ignoreIt = myIgnoreReadings.begin(); 
	 ignoreIt != myIgnoreReadings.end();
	 ignoreIt++)
    {
      //if (atDeg == 0)
      //printf("Ignoring %.0f\n", (*ignoreIt));
      if (ArMath::fabs(ArMath::subAngle(atDeg, *(ignoreIt))) < 1.0)
      {
      //printf("Ignoring %.0f\n", (*ignoreIt));
	ignore = true;
	break;
      }
    }
    //if (myMinRange != 0 && range < (int)myMinRange)
    //ignore = true;
    if (myMaxRange != 0 && range > (int)myMaxRange)
      ignore = true;
    */
    reading->resetSensorPosition(ArMath::roundInt(mySensorPose.getX()),
				 ArMath::roundInt(mySensorPose.getY()),
				 atDeg);
    //printf("dist %d\n", dist);
    reading->newData(range, mySimPacketStart, 
		     encoderPose,
		     mySimPacketTrans,
		     mySimPacketCounter, packet->getTimeReceived(), ignore, refl);
    
    //addReading(reading->getX(), reading->getY());
    tempIt = myIter;
    tempIt++;
    if (tempIt == myAssembleReadings->end() && 
	myWhichReading + 1 != myTotalNumReadings)
    {
      printf("!\n");
      myAssembleReadings->push_back(new ArSensorReading);
    }
    myIter++;
  }

  // check if the sensor set is complete
  //printf("%d %d %d\n", newReadings, readingNumber, totalNumReadings);
  if (newReadings + readingNumber >= totalNumReadings)
  {
    //printf("Got all readings...\n");
    // set ArRangeDevice buffer
    myRawReadings = myAssembleReadings;
    // switch internal buffers
    myAssembleReadings = myCurrentReadings;
    myCurrentReadings = myRawReadings;
    // We have in all the readings, now sort 'em and update the current ones
    //filterReadings();
    laserProcessReadings();
  }	
  
  unlockDevice();
  return true;
}
