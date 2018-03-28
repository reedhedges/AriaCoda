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
#include "ariaOSDef.h"
#include "ArCommands.h"
#include "ArExport.h"
#include "ArAnalogGyro.h"
#include "ArRobot.h"
#include "ArRobotConfigPacketReader.h"

/**
**/
AREXPORT ArAnalogGyro::ArAnalogGyro(ArRobot *robot) : 
  myHandleGyroPacketCB(this, &ArAnalogGyro::handleGyroPacket),
  myEncoderCorrectCB(this, &ArAnalogGyro::encoderCorrect),
  myStabilizingCB(this, &ArAnalogGyro::stabilizingCallback),
  myUserTaskCB(this, &ArAnalogGyro::userTaskCallback)
{
  myRobot = robot;
  myHandleGyroPacketCB.setName("ArAnalogGyro");
  myRobot->addStabilizingCB(&myStabilizingCB);
  // this scaling factor now comes from the parameter file
  myScalingFactor = 0;
  myHeading = 0;
  myReadingThisCycle = false;

  myAverageTotal = 0;
  myAverageCount = 0;
  myLastAverageTaken.setSec(0);
  myLastAverageTaken.setMSec(0);
  myLastAverage = 0;
  // nominal values
  myGyroSigma = .01;
  myInertialVarianceModel = 0.001;
  myRotVarianceModel = .25; // deg2/deg
  myTransVarianceModel = 4.0; // deg2/meter
  myAccumulatedDelta = 0;
  myIsActive = true;
  myHaveGottenData = false;
  myLogAnomalies = false;
  myGyroType = GYRO_NONE;
  myHasGyroOnlyMode = false;
  myHasNoData = true;
  myGyroWorking = true;
  if (myRobot->isConnected())
    stabilizingCallback();
  myUserTaskCB.setName("ArAnalogGyro");
  myRobot->addUserTask("ArAnalogGyro", 50, &myUserTaskCB);
  
}

AREXPORT ArAnalogGyro::~ArAnalogGyro()
{
  myRobot->comInt(ArCommands::GYRO, 0);
  myRobot->remPacketHandler(&myHandleGyroPacketCB);
  myRobot->remStabilizingCB(&myStabilizingCB);
  myRobot->setEncoderCorrectionCallback(NULL);
}

void ArAnalogGyro::stabilizingCallback(void)
{
  if (myRobot->getOrigRobotConfig() != NULL &&
      myRobot->getOrigRobotConfig()->getHasGyro() == 1)
  {
    myGyroType = GYRO_ANALOG_COMPUTER;
    myHasGyroOnlyMode = false;
    myHasNoData = false;
    myHaveGottenData = false;
    // moved these two here from above
    myRobot->setEncoderCorrectionCallback(&myEncoderCorrectCB);
    myRobot->addPacketHandler(&myHandleGyroPacketCB);

    myScalingFactor = myRobot->getRobotParams()->getGyroScaler();  
    
    if (!myRobot->isConnected())
      ArLog::log(ArLog::Normal, "Stabilizing gyro");
    if (!myRobot->isConnected() && myRobot->getStabilizingTime() < 3000)
      myRobot->setStabilizingTime(3000);
    myRobot->comInt(ArCommands::GYRO, 1);
  }
  else if (myRobot->getOrigRobotConfig() != NULL &&
	   myRobot->getOrigRobotConfig()->getGyroType() >= 2)
  {
    myGyroType = GYRO_ANALOG_CONTROLLER;
    myHasGyroOnlyMode = true;
    myHasNoData = true;
    myHaveGottenData = true;
    if (!myRobot->isConnected())
      ArLog::log(ArLog::Normal, "Stabilizing microcontroller gyro");
    myRobot->setStabilizingTime(2000);
    // only set this if it isn't already set (since this now comes in
    // the config packet so its variable per connection/robot)
    if (myRobot->getOdometryDelay() == 0)
      myRobot->setOdometryDelay(25);
  }
}

bool ArAnalogGyro::handleGyroPacket(ArRobotPacket *packet)
{
  int numReadings;
  int i;
  double reading;
  int temperature;
  double rate;
  ArTime now;

  if (packet->getID() != 0x98)
    return false;

  numReadings = packet->bufToByte();
  if (numReadings > 0)
    myHaveGottenData = true;
  //packet->log();
  //printf("%d readings %d bytes ", numReadings, packet->getLength() - packet->getReadLength());
  for (i = 0; i < numReadings; i++)
  {
    reading = packet->bufToByte2();
    temperature = packet->bufToUByte();

    // if we haven't moved, check our average, to see if we've moved,
    // we see if the average is within .5% of the average and
    // if the velocity is less then 1 mm / sec and 
    // if the rotational velocity is less then 1 deg / sec
    //printf("%d %g %g %g %g\n", myAverageCount, myAverageTotal / myAverageCount, reading, myRobot->getVel(), myRobot->getRotVel());
    if ((myAverageCount == 0 || fabs(myAverageTotal / myAverageCount - reading) < myAverageTotal / myAverageCount * .005) &&
	fabs(myRobot->getVel()) < 1 && 
	fabs(myRobot->getRotVel()) < 1)
    {
      if (myAverageStarted.mSecSince() > 1000 && myAverageCount > 25)
      {
	//printf("new average\n");
	myLastAverage = myAverageTotal / myAverageCount;
	myLastAverageTaken.setToNow();
	myAverageTotal = 0;
	myAverageCount = 0;
	myAverageStarted.setToNow();
      }
      myAverageTotal += reading;
      myAverageCount++;
    }
    else
    {
      myAverageTotal = 0;
      myAverageCount = 0;
      myAverageStarted.setToNow();
    }
    
    if (myLastAverage == 0)
      continue;
    reading -= myLastAverage;
    rate = ((reading * 5.0 / 1023)) * 300 / 2.5 * myScalingFactor;
    rate *= -1;

    myTemperature = temperature;
    //printf("reading %10f rate %10f diff %10f temp %d, ave %g\n", reading, rate, rate * .025, temperature, myLastAverage);

    // if we're not moving and the reading is small disregard it
    if ((fabs(myRobot->getVel()) < 2 && fabs(myRobot->getRotVel()) < 2) &&
	ArMath::fabs(reading) < 2)      
    {
      rate = 0;
    }
    myHeading += rate * .025;
    //printf("rate %6.3f, reading %6.3f heading %6.3f\n", rate, reading, myHeading);

    myHeading = ArMath::fixAngle(myHeading);

    if (myTimeLastPacket != time(NULL)) 
    {
      myTimeLastPacket = time(NULL);
      myPacCount = myPacCurrentCount;
      myPacCurrentCount = 0;
    }
    myPacCurrentCount++;
    myReadingThisCycle = true;
    //printf("(%3d %3d)", reading, temperature);
  }
  //printf("\n");
  return true;
}

double ArAnalogGyro::encoderCorrect(ArPoseWithTime deltaPose)
{
  ArPose ret;

  // variables
  double inertialVariance;
  double encoderVariance;

  double robotDeltaTh;
  double inertialDeltaTh;
  double deltaTh;

  /*
  ArPose lastPose;
  double lastTh;
  ArPose thisPose;
  double thisTh;
  */
  ArPoseWithTime zero(0.0, 0.0, 0.0);


  // if we didn't get a reading this take what we got at face value
  if (!myReadingThisCycle)
  {
    //ArLog::log(ArLog::Verbose, "ArAnalogGyro: no inertial reading, using encoder");
    myAccumulatedDelta += deltaPose.getTh();
    //printf("adding %f\n", myAccumulatedDelta);
    return deltaPose.getTh();
  }

  // 6/20/05 MPL added this fix
  robotDeltaTh = ArMath::fixAngle(myAccumulatedDelta + deltaPose.getTh());
  //printf("using %f %f %f\n", robotDeltaTh, myAccumulatedDelta, deltaPose.getTh());

  inertialVariance = (myGyroSigma * myGyroSigma * 10);
  // was: deltaPose.getTime().mSecSince(myLastAsked)/10.0);


  //printf("@ %10f %10f %10f %10f\n", multiplier, ArMath::subAngle(thisTh, lastTh), thisTh, lastTh);
  inertialDeltaTh = ArMath::subAngle(myHeading, myLastHeading);

  inertialVariance += fabs(inertialDeltaTh) * myInertialVarianceModel;
  encoderVariance = (fabs(deltaPose.getTh()) * myRotVarianceModel +
		     deltaPose.findDistanceTo(zero) * myTransVarianceModel);
  
  
  if (myLogAnomalies)
  {
    if (fabs(inertialDeltaTh) < 1 && fabs(robotDeltaTh) < 1)
    {

    }
    else if (fabs(robotDeltaTh) < 1 && fabs(inertialDeltaTh) > 2)
      ArLog::log(ArLog::Normal, "ArAnalogGyro::anomaly: Gyro (%.1f) moved but encoder (%.1f) didn't, using gyro", inertialDeltaTh, robotDeltaTh);
    else if ((inertialDeltaTh < -1 && robotDeltaTh > 1) ||
	     (robotDeltaTh < -1 && inertialDeltaTh > 1))
      ArLog::log(ArLog::Normal, "ArAnalogGyro::anomaly: gyro (%.1f) moved opposite of robot (%.1f)", inertialDeltaTh, robotDeltaTh);
    else if (fabs(robotDeltaTh) < fabs(inertialDeltaTh * .66666))
      ArLog::log(ArLog::Normal, "ArAnalogGyro::anomaly: robot (%.1f) moved less than gyro (%.1f)", robotDeltaTh, inertialDeltaTh);
    else if (fabs(inertialDeltaTh) < fabs(robotDeltaTh * .66666))
      ArLog::log(ArLog::Normal, "ArAnalogGyro::anomaly: gyro (%.1f) moved less than robot (%.1f)", inertialDeltaTh, robotDeltaTh);
  }


  //don't divide by 0, or close to it
  if (fabs(inertialVariance + encoderVariance) < .00000001)
    deltaTh = 0;
  // if we get no encoder readings, but we get gyro readings, just
  // believe the gyro (this case is new 6/20/05 MPL)
  else if (fabs(robotDeltaTh) < 1 && fabs(inertialDeltaTh) > 2)
    deltaTh = ArMath::fixAngle(inertialDeltaTh);
  else
    deltaTh = ArMath::fixAngle(
	    (robotDeltaTh * 
	     (inertialVariance / (inertialVariance + encoderVariance))) +
	    (inertialDeltaTh *
	     (encoderVariance / (inertialVariance + encoderVariance))));

  // now we need to compensate for the readings we got when we didn't
  // have gyro readings
  deltaTh -= myAccumulatedDelta;
  myReadingThisCycle = false;
  myLastHeading = myHeading;
  //printf("%6.3f %6.3f %6.3f\n", deltaTh, inertialDeltaTh, robotDeltaTh);

  myAccumulatedDelta = 0;
  
  if (myIsActive)
    return deltaTh;
  else
    return deltaPose.getTh();
}


AREXPORT void ArAnalogGyro::activate(void)
{ 
  if (!myIsActive || myIsGyroOnlyActive)
    ArLog::log(ArLog::Normal, "Activating gyro"); 
  myIsActive = true; 
  myIsGyroOnlyActive = false;
  if (myGyroType == GYRO_ANALOG_CONTROLLER)
    myRobot->comInt(ArCommands::GYRO, 1);
}

AREXPORT void ArAnalogGyro::deactivate(void)
{ 
  if (myIsActive || myIsGyroOnlyActive)
    ArLog::log(ArLog::Normal, "Dectivating gyro"); 
  myIsActive = false; 
  myIsGyroOnlyActive = false;
  if (myGyroType == GYRO_ANALOG_CONTROLLER)
    myRobot->comInt(ArCommands::GYRO, 0);
}

AREXPORT void ArAnalogGyro::activateGyroOnly(void)
{ 
  if (!myHasGyroOnlyMode)
  {
    ArLog::log(ArLog::Normal, 
	       "An attempt was made (and rejected) to set gyro only mode on a gyro that cannot do that mode"); 
    return;
  }
  if (!myIsActive || !myIsGyroOnlyActive)
    ArLog::log(ArLog::Normal, "Activating gyro only"); 
  myIsActive = false; 
  myIsGyroOnlyActive = true;
  if (myGyroType == GYRO_ANALOG_CONTROLLER)
    myRobot->comInt(ArCommands::GYRO, 2);
}

void ArAnalogGyro::userTaskCallback(void)
{
  if ((myRobot->getFaultFlags() & ArUtil::BIT4) && myGyroWorking)
  {
    ArLog::log(ArLog::Normal, "ArAnalogGyro: Gyro failed");
    myGyroWorking = false;
  }

  if (!(myRobot->getFaultFlags() & ArUtil::BIT4) && !myGyroWorking)
  {
    ArLog::log(ArLog::Normal, "ArAnalogGyro: Gyro recovered");
    myGyroWorking = true;
  }
}

