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
#include "ArActionDeceleratingLimiter.h"
#include "ArRobot.h"
#include "ArCommands.h"
#include "ariaInternal.h"
#include "ArRobotConfigPacketReader.h"
#include "ArRangeDevice.h"

/**
   @param name name of the action
   @param type whether we're an action for going forwards (ArActionDeceleratingLimiter::FORWARDS) 
   backwards (ArActionDeceleratingLimiter::BACKWARDS), or laterally to the left
   (ArActionDeceleratingLimiter::LATERAL_LEFT) or laterally
   to the right (ArActionDeceleratingLimiter::LATERAL_RIGHT). This causes
   ArActionDeceleratingLimiter to choose the right values and choose X or Y translation
   decelerations and speeds.
*/
AREXPORT ArActionDeceleratingLimiter::ArActionDeceleratingLimiter(
	const char *name, 
	LimiterType type) :
  ArAction(name,
	   "Slows the robot down and cranks up deceleration so as not to hit anything in front of it.")
{
  myType = type;
  setParameters();

  myLastStopped = false;
  myUseLocationDependentDevices = true;
  myStopRotationToo = false;

}

AREXPORT ArActionDeceleratingLimiter::~ArActionDeceleratingLimiter()
{

}

/**
   @param clearance distance at which to estop  (mm)
   @param sideClearanceAtSlowSpeed distance on the side to stop for if going at slow speed or slower (mm)
   @param paddingAtSlowSpeed distance in addition to clerance at which to stop at slow speed (mm)
   @param slowSpeed speed which we consider to be "slow" (mm/sec)
   @param sideClearanceAtFastSpeed distance on the side to stop for if going at fast speed or faster (mm)
   @param paddingAtFastSpeed distance in addition to clerance at which to stop at fast speed (mm)
   @param fastSpeed speed which we consider to be "fast" (mm/sec)
   @param preferredDecel the maximum deceleration to slow for obstacles (unless it will be insufficient to keep the clearances free, then decelerate faster)
   @param useEStop if something is detected within the clearance, cause an immediate emergecy stop
   @param maxEmergencyDecel ultimate limit on deceleration to apply when slowing for an obstacle detected within clearance  (mm/sec/sec); if 0, use the robot's maximum decel parameter.
*/
AREXPORT void ArActionDeceleratingLimiter::setParameters(
	double clearance,
	double sideClearanceAtSlowSpeed,
	double paddingAtSlowSpeed,
	double slowSpeed,
	double sideClearanceAtFastSpeed,
	double paddingAtFastSpeed,
	double fastSpeed,
	double preferredDecel,
	bool useEStop,
	double maxEmergencyDecel)
{
  myClearance = clearance;
  mySideClearanceAtSlowSpeed = sideClearanceAtSlowSpeed;
  myPaddingAtSlowSpeed = paddingAtSlowSpeed;
  mySlowSpeed = slowSpeed;
  mySideClearanceAtFastSpeed = sideClearanceAtFastSpeed;
  myPaddingAtFastSpeed = paddingAtFastSpeed;
  myFastSpeed = fastSpeed;
  myPreferredDecel = preferredDecel;
  myUseEStop = useEStop;
  myMaxEmergencyDecel = maxEmergencyDecel;
}

AREXPORT void ArActionDeceleratingLimiter::addToConfig(ArConfig *config, 
						       const char *section, 
						       const char *prefix)
{
  std::string strPrefix;
  std::string name;
  if (prefix == NULL || prefix[0] == '\0')
    strPrefix = "";
  else
    strPrefix = prefix;

  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::NORMAL);  

  name = strPrefix;
  name += "Clearance";
  config->addParam(
	  ArConfigArg(name.c_str(), &myClearance, 
		      "Don't get closer than this to something in front or back. (mm)"), 
	  section, ArPriority::NORMAL);

  name = strPrefix;
  name += "SlowSpeed";
  config->addParam(
	  ArConfigArg(name.c_str(),
		      &mySlowSpeed,
		      "Consider this speed slow (mm/sec)"),
	  section, ArPriority::NORMAL);

  name = strPrefix;
  name += "SideClearanceAtSlowSpeed";
  config->addParam(
	  ArConfigArg(name.c_str(),
		      &mySideClearanceAtSlowSpeed, 
		      "Don't get closer than this to something on the side if we're going at slow speed or below. (mm)"),
	  section, ArPriority::NORMAL);

  name = strPrefix;
  name += "PaddingAtSlowSpeed";
  config->addParam(
	  ArConfigArg(name.c_str(), &myPaddingAtSlowSpeed, 
		      "Try to stop this far away from clearance at slow speed or below. (mm)", 0), 
	  section, ArPriority::NORMAL);

  name = strPrefix;
  name += "FastSpeed";
  config->addParam(
	  ArConfigArg(name.c_str(),
		      &myFastSpeed,
		      "Consider this speed fast (mm/sec)"),
	  section, ArPriority::NORMAL);

  name = strPrefix;
  name += "SideClearanceAtFastSpeed";
  config->addParam(
	  ArConfigArg(name.c_str(),
		      &mySideClearanceAtFastSpeed, 
		      "Don't get closer than this to something on the side if we're going at fast speed or above. (mm)"),
	  section, ArPriority::NORMAL);

  name = strPrefix;
  name += "PaddingAtFastSpeed";
  config->addParam(
	  ArConfigArg(name.c_str(), &myPaddingAtFastSpeed, 
		      "Try to stop this far away from clearance at fast speed or below. (mm)", 0), 
	  section, ArPriority::NORMAL);

  name = strPrefix;
  name += "PreferredDecel";
  config->addParam(
	  ArConfigArg(name.c_str(),
		      &myPreferredDecel,
		      "The maximum decel we'll use until something might infringe on clearance and sideClearanceAtSlowSpeed (mm/sec/sec"),
	  section, ArPriority::NORMAL);

  name = strPrefix;
  name += "MaxEmergencyDecel";
  config->addParam(
	  ArConfigArg(name.c_str(),
		      &myMaxEmergencyDecel,
		      "The maximum decel we'll ever use, 0 means use the robot's maximum (mm/sec/sec"),
	  section, ArPriority::NORMAL);

  name = strPrefix;
  name += "UseEStop";
  config->addParam(
	  ArConfigArg(name.c_str(),
		      &myUseEStop,
		      "Whether to use an EStop to stop if something will intrude on our clearance"),
	  section, ArPriority::NORMAL);

  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::NORMAL);
}

AREXPORT ArActionDesired *
ArActionDeceleratingLimiter::fire(ArActionDesired currentDesired)
{
  double dist;
  const ArRangeDevice *distRangeDevice = NULL;
  double distInner;
  const ArRangeDevice *distInnerRangeDevice = NULL;
  double absVel;
  bool printing = false;

  //ArLog::LogLevel verboseLogLevel = ArLog::Verbose;
  ArLog::LogLevel verboseLogLevel = ArLog::Verbose;
  if (printing)
    verboseLogLevel = ArLog::Normal;

  double lookAhead = 16000;
  
  //if (myType == LATERAL_RIGHT)
  //printing = true;

  myDesired.reset();
  // see if we're going the right direction for this to work
  if (myType == FORWARDS && myRobot->getVel() < -100)
    return NULL;
  else if (myType == BACKWARDS && myRobot->getVel() > 100)
    return NULL;
  else if (myType == LATERAL_LEFT && myRobot->getLatVel() < -100)
    return NULL;
  else if (myType == LATERAL_RIGHT && myRobot->getLatVel() > 100)
    return NULL;
  if (myType != LATERAL_LEFT && myType != LATERAL_RIGHT)
    absVel = ArMath::fabs(myRobot->getVel());
  else
    absVel = ArMath::fabs(myRobot->getLatVel());

  double sideClearance;
  double padding;
  
  // see if we're going slow
  if (absVel <= mySlowSpeed)
  {
    sideClearance = mySideClearanceAtSlowSpeed;
    padding = myPaddingAtSlowSpeed;
  }
  // or if we're going fast
  else if (absVel >= myFastSpeed)
  {
    sideClearance = mySideClearanceAtFastSpeed;
    padding = myPaddingAtFastSpeed;
  }
  // or if we have to interpolate
  else
  {
    sideClearance = (((mySideClearanceAtFastSpeed - 
		       mySideClearanceAtSlowSpeed) * 
		      ((absVel - mySlowSpeed) / 
		       (myFastSpeed - mySlowSpeed))) + 
		     mySideClearanceAtSlowSpeed);
    padding = (((myPaddingAtFastSpeed - 
		 myPaddingAtSlowSpeed) * 
		((absVel - mySlowSpeed) / 
		 (myFastSpeed - mySlowSpeed))) + 
	       myPaddingAtSlowSpeed);
  }
  
  
  //if (printing)
  //ArLog::log(ArLog::Normal, "%d side %.0f padding %.0f", myType, sideClearance, padding);
		
  ArPose obstaclePose(-1, -1, -1);
  ArPose obstacleInnerPose(-1, -1, -1);

  if (myType == FORWARDS)
    dist = myRobot->checkRangeDevicesCurrentBox(
	    0,
	    -(myRobot->getRobotWidth()/2.0 + sideClearance),
	    myRobot->getRobotLength()/2.0 + myClearance + padding + lookAhead,
	    (myRobot->getRobotWidth()/2.0 + sideClearance),
	    &obstaclePose,
	    &distRangeDevice, myUseLocationDependentDevices);
  else if (myType == BACKWARDS)
    dist = myRobot->checkRangeDevicesCurrentBox(
	    0,
	    -(myRobot->getRobotWidth()/2.0 + sideClearance),
	    -(myRobot->getRobotLength()/2.0 + myClearance + padding + lookAhead),
	    (myRobot->getRobotWidth()/2.0 + sideClearance),
	    &obstaclePose,
	    &distRangeDevice, myUseLocationDependentDevices);
  //todo
  else if (myType == LATERAL_LEFT)
    dist = myRobot->checkRangeDevicesCurrentBox(
	    -(myRobot->getRobotLength()/2.0 + sideClearance),
	    0,
	    (myRobot->getRobotLength()/2.0 + sideClearance),
	    myRobot->getRobotWidth()/2.0 + myClearance + padding + lookAhead,
	    &obstaclePose,
	    &distRangeDevice, myUseLocationDependentDevices);
  //todo
  else if (myType == LATERAL_RIGHT)
    dist = myRobot->checkRangeDevicesCurrentBox(
	    -(myRobot->getRobotLength()/2.0 + sideClearance),
	    -(myRobot->getRobotWidth()/2.0 + myClearance + padding + lookAhead),  
	    (myRobot->getRobotLength()/2.0 + sideClearance),
	    0,
	    &obstaclePose,
	    &distRangeDevice, myUseLocationDependentDevices);

  if (myType == FORWARDS)
    distInner = myRobot->checkRangeDevicesCurrentBox(
	    0,
	    -(myRobot->getRobotWidth()/2.0 + mySideClearanceAtSlowSpeed),
	    myRobot->getRobotLength()/2.0 + myClearance + lookAhead,
	    (myRobot->getRobotWidth()/2.0 + mySideClearanceAtSlowSpeed),
	    &obstacleInnerPose,
	    &distInnerRangeDevice, myUseLocationDependentDevices);
  else if (myType == BACKWARDS)
    distInner = myRobot->checkRangeDevicesCurrentBox(
	    0,
	    -(myRobot->getRobotWidth()/2.0 + mySideClearanceAtSlowSpeed),
	    -(myRobot->getRobotLength()/2.0 + myClearance + lookAhead),
	    (myRobot->getRobotWidth()/2.0 + mySideClearanceAtSlowSpeed),
	    &obstacleInnerPose,
	    &distInnerRangeDevice, myUseLocationDependentDevices);
  // todo
  else if (myType == LATERAL_LEFT)
    distInner = myRobot->checkRangeDevicesCurrentBox(
	    -(myRobot->getRobotLength()/2.0 + mySideClearanceAtSlowSpeed),
	    0,
	    (myRobot->getRobotLength()/2.0 + mySideClearanceAtSlowSpeed),
	    myRobot->getRobotWidth()/2.0 + myClearance + lookAhead,
	    &obstacleInnerPose,
	    &distInnerRangeDevice, myUseLocationDependentDevices);
  // todo
  else if (myType == LATERAL_RIGHT)
    distInner = myRobot->checkRangeDevicesCurrentBox(
	    -(myRobot->getRobotLength()/2.0 + mySideClearanceAtSlowSpeed),
	    -(myRobot->getRobotWidth()/2.0 + myClearance + lookAhead),
	    (myRobot->getRobotLength()/2.0 + mySideClearanceAtSlowSpeed),
	    0,
	    &obstacleInnerPose,
	    &distInnerRangeDevice, myUseLocationDependentDevices);

  // subtract off our clearance and padding to see how far we have to stop
  if (myType != LATERAL_LEFT && myType != LATERAL_RIGHT)
    dist -= myRobot->getRobotLength() / 2.0;
  else
    dist -= myRobot->getRobotWidth() / 2.0;
  dist -= myClearance;
  dist -= padding;

  // this is what we estop for, so don't subtract our padding from this
  if (myType != LATERAL_LEFT && myType != LATERAL_RIGHT)
    distInner -= myRobot->getRobotLength() / 2.0;
  else
    distInner -= myRobot->getRobotWidth() / 2.0;
  distInner -= myClearance;

  if (printing)
    ArLog::log(ArLog::Normal, "%d dist %.0f (%.0f %.0f from %s)", myType, dist,
	       obstaclePose.getX(), obstaclePose.getY(),
	       distRangeDevice != NULL ? distRangeDevice->getName() : "none");
  // see if we need to throw an estop
  if (distInner < 1 && absVel > 5)
  {
    if (printing && !myLastStopped)
      ArLog::log(ArLog::Normal, "%s: Stopping", getName());
    if (absVel > 100)
    {
      if (myUseEStop)
      {
	if (distInnerRangeDevice != NULL)
	  ArLog::log(ArLog::Normal, "%s: estopping because of reading from %s", getName(), distInnerRangeDevice->getName());
	else
	  ArLog::log(ArLog::Normal, "%s: estopping because of reading from unknown", getName());
	myRobot->comInt(ArCommands::ESTOP, 1);
      }
      else
      {
	if (distInnerRangeDevice != NULL)
	  ArLog::log(ArLog::Normal, 
		     "%s: maximum deceleration because of reading from %s", 
		     getName(), distInnerRangeDevice->getName());
	else
	  ArLog::log(ArLog::Normal, 
	     "%s: maximum deceleration because of reading from unknown", 
		     getName());
      }
      // if we have a maximum emergency decel, use that
      if (fabs(myMaxEmergencyDecel) > 1)
      {
	if (printing)
	  ArLog::log(ArLog::Normal, "Max emergency decel %.0f", 
		 myMaxEmergencyDecel);
	if (myType != LATERAL_LEFT && myType != LATERAL_RIGHT)
	  myDesired.setTransDecel(myMaxEmergencyDecel);
	else
	  myDesired.setLatDecel(myMaxEmergencyDecel);
      }
      //  if we don't use the robot's top decel
      else if (myRobot->getOrigRobotConfig() != NULL && 
	  myRobot->getOrigRobotConfig()->hasPacketArrived())
      {
	if (myType != LATERAL_LEFT && myType != LATERAL_RIGHT)
	  ArLog::log(ArLog::Normal, "Robots max decel %d", 
		 myRobot->getOrigRobotConfig()->getTransAccelTop());
	else
	  ArLog::log(ArLog::Normal, "Robots max lat decel %d", 
		 myRobot->getOrigRobotConfig()->getLatAccelTop());
	if (myType != LATERAL_LEFT && myType != LATERAL_RIGHT)
	  myDesired.setTransDecel(
		  myRobot->getOrigRobotConfig()->getTransAccelTop());
	else
	  myDesired.setLatDecel(
		  myRobot->getOrigRobotConfig()->getLatAccelTop());
      }
      // if we don't have that either use our preferred decel
      else
      {
	if (printing)
	  ArLog::log(ArLog::Normal, "Prefered decel %g", myPreferredDecel);
	if (myType != LATERAL_LEFT && myType != LATERAL_RIGHT)
	  myDesired.setTransDecel(myPreferredDecel);
	else
	  myDesired.setLatDecel(myPreferredDecel);
      }
    }
    myLastStopped = true;
    if (myType == FORWARDS)
      myDesired.setMaxVel(0);
    else if (myType == BACKWARDS)
      myDesired.setMaxNegVel(0);
    if (myType == LATERAL_LEFT)
      myDesired.setMaxLeftLatVel(0);
    else if (myType == LATERAL_RIGHT)
      myDesired.setMaxRightLatVel(0);

    if (myType != LATERAL_LEFT && myType != LATERAL_RIGHT)
      myDesired.setVel(0);
    else
      myDesired.setLeftLatVel(0);
    if (myStopRotationToo)
      myDesired.setRotVel(0);
    if (distInnerRangeDevice != NULL)
      ArLog::log(verboseLogLevel,
		 "%s: Stopping (inner) because of reading from %s", getName(),
		 distInnerRangeDevice->getName());
    else
      ArLog::log(verboseLogLevel,
		 "%s: Stopping (inner) because of reading from unknown", 
		 getName());
    return &myDesired;
  }
  // if our distance is greater than how far it'd take us to stop
  //ArLog::log(ArLog::Normal, "%.0f %.0f %.0f", dist, absVel, absVel * absVel / 2.0 / myRobot->getTransDecel());
  double robotDeceleration;
  if (myType != LATERAL_LEFT && myType != LATERAL_RIGHT)
    robotDeceleration = myRobot->getTransDecel();
  else
    robotDeceleration = myRobot->getLatDecel();
  if (dist > absVel * absVel / 2.0 / robotDeceleration)
  {
    if (printing)
      ArLog::log(ArLog::Normal, "%d Nothing", myType);
    return NULL;
  }

  if (printing && myLastStopped)
    ArLog::log(ArLog::Normal, "%d moving", myType);
  myLastStopped = false;
  //ArLog::log(ArLog::Normal, "%f ", dist);
  //maxVel = (dist - clearance);
  double deceleration = - absVel * absVel / dist / 2.0;
  double decelerationInner = - absVel * absVel / distInner / 2.0;
  // make sure the robot or other actions aren't already decelerating
  // more than we want to

  // the reason its okay that below uses deceleration instead of
  // decelerationInner is because unless padding is negative (no
  // longer allowed) the deceleration before the cap will always be
  // higher than decelerationInner


  //ArLog::log(ArLog::Normal, "%.0f %.0f %.0f %.0f", deceleration, myRobot->getTransDecel(), 	 currentDesired.getTransDecelStrength(), currentDesired.getTransDecel());
  if ((myType != LATERAL_LEFT &&  myType != LATERAL_RIGHT &&
       fabs(deceleration) > fabs(myRobot->getTransDecel()) &&
       (currentDesired.getTransDecelStrength() < ArActionDesired::MIN_STRENGTH 
	|| fabs(deceleration) > fabs(currentDesired.getTransDecel()))) ||
      ((myType == LATERAL_LEFT || myType == LATERAL_RIGHT) && 
       fabs(deceleration) > fabs(myRobot->getLatDecel()) &&
       (currentDesired.getLatDecelStrength() < ArActionDesired::MIN_STRENGTH 
	|| fabs(deceleration) > fabs(currentDesired.getLatDecel()))))
  {
    double decelDesired;
    // if our deceleration is faster than we want to decel see if we
    // actually will have to decel that fast or not
    if (fabs(myMaxEmergencyDecel) > 1 && 
	fabs(decelerationInner) > myMaxEmergencyDecel)
      decelDesired = myMaxEmergencyDecel;
    else if (fabs(decelerationInner) > myPreferredDecel)
      decelDesired = fabs(decelerationInner);
    else if (fabs(myMaxEmergencyDecel) > 1 && 
	     fabs(deceleration) > myMaxEmergencyDecel)
      decelDesired = myMaxEmergencyDecel;
    else if (fabs(deceleration) > myPreferredDecel)
      decelDesired = myPreferredDecel;
    else 
      decelDesired = fabs(deceleration);

    if (myType != LATERAL_LEFT && myType != LATERAL_RIGHT)
      myDesired.setTransDecel(decelDesired);
    else
      myDesired.setLatDecel(decelDesired);
    if (printing)
      ArLog::log(ArLog::Normal, "Set deceleration to %g", decelDesired);
  }
  else
  {
    if (myType != LATERAL_LEFT && myType != LATERAL_RIGHT)
      deceleration = myRobot->getTransDecel();
    else
      deceleration = myRobot->getLatDecel();
  }

  //double maxVel = absVel - deceleration  / 10.0;

  if (printing)
    ArLog::log(ArLog::Normal, "%d accel %.0f", myType, deceleration);
  //ArLog::log(ArLog::Normal, "Max vel %f (stopdist %.1f slowdist %.1f slowspeed %.1f", maxVel,	 myStopDist, mySlowDist, mySlowSpeed);

  /*
  ArLog::log(ArLog::Normal, "%s: Stopping because of reading from %s (%.0f %.0f %.0f %.0f)", 
	     getName(), distInnerRangeDevice->getName(),
	     dist, deceleration, robotDeceleration, 
	     absVel * absVel / 2.0 / robotDeceleration);
  */
  if (distInnerRangeDevice != NULL)
    ArLog::log(verboseLogLevel,
	       "%s: Stopping because of reading from %s", 
	       getName(), distInnerRangeDevice->getName());
  else
    ArLog::log(verboseLogLevel, 
	       "%s: Stopping because of reading from unknown", 
	       getName());

  if (myType == FORWARDS)
    myDesired.setMaxVel(0);
  else if (myType == BACKWARDS)
    myDesired.setMaxNegVel(0);
  else if (myType == LATERAL_LEFT)
    myDesired.setMaxLeftLatVel(0);
  else if (myType == LATERAL_RIGHT)
    myDesired.setMaxRightLatVel(0);

  return &myDesired;
}



