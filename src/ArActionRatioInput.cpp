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
#include "ArActionRatioInput.h"
#include "ArRobot.h"

/**
   @param name name of the action
*/
AREXPORT ArActionRatioInput::ArActionRatioInput(const char *name) :
    ArAction(name, "RatioInputs vel and heading")
{
  myTransRatio = 0;
  myRotRatio = 0;
  myLatRatio = 0;
  myThrottleRatio = 0;
  myFullThrottleForwards = 0;
  myFullThrottleBackwards = 0;
  myRotAtFullForwards = 25;
  myRotAtFullBackwards = 25;
  myRotAtStopped = 50;

  myLatAtFullForwards = 0;
  myLatAtFullBackwards = 0;
  myLatAtStopped = 0;

  myPrinting = false;

  myTransDeadZone = 10;
  myRotDeadZone = 5;
  myLatDeadZone = 10;
}

AREXPORT ArActionRatioInput::~ArActionRatioInput()
{
}

AREXPORT void ArActionRatioInput::addToConfig(ArConfig *config, 
					      const char *section)
{
  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::NORMAL);
  config->addParam(
	  ArConfigArg("FullThrottleForwards", &myFullThrottleForwards,
		      "The maximum forwards speed (0 means robot's TransVelMax) (mm/sec)", 0),
	  section, ArPriority::NORMAL);

  config->addParam(
	  ArConfigArg("FullThrottleBackwards", &myFullThrottleBackwards,
		      "The maximum backwards speed (0 means 1/4 robot's TransVelMax) (mm/sec)", 0),
	  section, ArPriority::NORMAL);

  config->addParam(
	  ArConfigArg("RotAtFullForwards", &myRotAtFullForwards,
	      "The maximum speed we turn at when going full forwards (0 means 1/2 robots RotVelMax) (deg/sec)", 0),
	  section, ArPriority::NORMAL);

  config->addParam(
	  ArConfigArg("RotAtFullBackwards", &myRotAtFullBackwards,
	      "The maximum speed we turn at when going full backwards (0 means 1/2 robots RotVelMax) (deg/sec)", 0),
	  section, ArPriority::NORMAL);

  config->addParam(
	  ArConfigArg("RotAtStopped", &myRotAtStopped,
	      "The maximum speed we turn at when stopped (0 means robot's RotVelMax) (deg/sec)", 0),
  section, ArPriority::NORMAL);
  
  if (myRobot && myRobot->hasLatVel())
  {
    config->addParam(
	    ArConfigArg("LatAtFullForwards", &myLatAtFullForwards,
			"The maximum speed we turn at when going full forwards (0 means 1/2 robots LatVelMax) (mm/sec)", 0),
	    section, ArPriority::NORMAL);
    
    config->addParam(
	    ArConfigArg("LatAtFullBackwards", &myLatAtFullBackwards,
			"The maximum speed we turn at when going full backwards (0 means 1/2 robots LatVelMax) (mm/sec)", 0),
	    section, ArPriority::NORMAL);
    
    config->addParam(
	    ArConfigArg("LatAtStopped", &myLatAtStopped,
			"The maximum speed we turn at when stopped (0 means robot's LatVelMax) (mm/sec)", 0),
	    section, ArPriority::NORMAL);
  }

  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::NORMAL);

    config->addParam(
	    ArConfigArg("TransDeadZone", &myTransDeadZone,
			"The percentage in the middle of the translation direction to not drive (percent)", 0),
	    section, ArPriority::EXPERT);

    config->addParam(
	    ArConfigArg("RotDeadZone", &myRotDeadZone,
			"The percentage in the middle of the rotation direction to not drive (percent)", 0),
	    section, ArPriority::EXPERT);

  if (myRobot && myRobot->hasLatVel())
  {
    config->addParam(
	    ArConfigArg("LatDeadZone", &myLatDeadZone,
			"The percentage in the middle of the lateral direction to not drive (percent)", 0),
	    section, ArPriority::EXPERT);
  }

  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::NORMAL);

}

/**
   This checks the inputs and pulls them into the valid range.
**/
AREXPORT void ArActionRatioInput::setRatios(double transRatio, 
					    double rotRatio, 
					    double throttleRatio,
					    double latRatio)
{
  setTransRatio(transRatio);
  setRotRatio(rotRatio);
  setThrottleRatio(throttleRatio);
  setLatRatio(latRatio);
}

/**
   This checks the input for greather than 100 and less than -100 and
   pulls it to within that range.
**/
AREXPORT void ArActionRatioInput::setTransRatio(double transRatio)
{
  if (transRatio > 100)
    myTransRatio = 100;
  else if (transRatio < -100)
    myTransRatio = -100;
  else
    myTransRatio = transRatio;
}

/**
   This checks the input for greather than 100 and less than -100 and
   pulls it to within that range.
**/
AREXPORT void ArActionRatioInput::setRotRatio(double rotRatio)
{
  if (rotRatio > 100)
    myRotRatio = 100;
  else if (rotRatio < -100)
    myRotRatio = -100;
  else
    myRotRatio = rotRatio;
}

/**
   This checks the input for greather than 100 and less than -100 and
   pulls it to within that range.
**/
AREXPORT void ArActionRatioInput::setLatRatio(double latRatio)
{
  if (latRatio > 100)
    myLatRatio = 100;
  else if (latRatio < -100)
    myLatRatio = -100;
  else
    myLatRatio = latRatio;
}

/**
   This checks the input for greather than 100 and less than 0 and
   pulls it to within that range.
**/
AREXPORT void ArActionRatioInput::setThrottleRatio(double throttleRatio)
{
  if (throttleRatio > 100)
    myThrottleRatio = 100;
  else if (throttleRatio < 0)
    myThrottleRatio = 0;
  else
    myThrottleRatio = throttleRatio;
}

/**
   @param fullThrottleForwards the speed we go forwards at at full 
   throttle (mm/sec)

   @param fullThrottleBackwards the speed we go backwards at at full 
   throttle (mm/sec)
   
   @param rotAtFullForwards the speed we turn at at full throttle forwards

   @param rotAtFullBackwards the speed we turn at at full throttle backwards

   @param rotAtStopped the speed we turn at if there is no forward/backward
   motion

   @param rotAtFullForwards the speed we turn at at full throttle forwards

   @param latAtFullForwards the lateral speed we go at at full 
   throttle (mm/sec)  (if robot supports lateral motion)

   @param latAtFullBackwards the lateral speed we go at at full 
   throttle (mm/sec) (if robot supports lateral motion)

   @param latAtStopped the lateral speed we go at if stopped.
   

**/
AREXPORT void ArActionRatioInput::setParameters(double fullThrottleForwards, 
						double fullThrottleBackwards, 
						double rotAtFullForwards,
						double rotAtFullBackwards,
						double rotAtStopped,
						double latAtFullForwards,
						double latAtFullBackwards,
						double latAtStopped)
{
  myFullThrottleForwards = fullThrottleForwards;
  myFullThrottleBackwards = fullThrottleBackwards;
  myRotAtFullForwards = rotAtFullForwards;
  myRotAtFullBackwards = rotAtFullBackwards;
  myRotAtStopped = rotAtStopped;
  myLatAtFullForwards = latAtFullForwards;
  myLatAtFullBackwards = latAtFullBackwards;
  myLatAtStopped = latAtStopped;
}

/**
   These callbacks are actually called in the order of lowest number
   to highest number, but this still means higher numbers are more
   important since throttle set by those will override the lower.
**/
AREXPORT void ArActionRatioInput::addFireCallback(int priority, 
						  ArFunctor *functor)
{
  myFireCallbacks.insert(std::pair<int, ArFunctor *>(priority, functor));
}

AREXPORT void ArActionRatioInput::remFireCallback(ArFunctor *functor)
{
  std::multimap<int, ArFunctor *>::iterator it;
  for (it = myFireCallbacks.begin(); it != myFireCallbacks.end(); it++)
  {
    if ((*it).second == functor)
      break;

  }
  if (it != myFireCallbacks.end())
  {
    myFireCallbacks.erase(it);
  }
  else 
    ArLog::log(ArLog::Normal, "ArActionRatioInput::RemFireCallback: could not remove callback");
}

AREXPORT void ArActionRatioInput::addActivateCallback(ArFunctor *functor, 
						ArListPos::Pos position)
{
  if (position == ArListPos::FIRST)
    myActivateCallbacks.push_front(functor);
  else if (position == ArListPos::LAST)
    myActivateCallbacks.push_back(functor);
  else
    ArLog::log(ArLog::Terse, 
	       "ArActionRatioInput::addActivateCallback: Invalid position.");
}

AREXPORT void ArActionRatioInput::remActivateCallback(ArFunctor *functor)
{
  myActivateCallbacks.remove(functor);
}

AREXPORT void ArActionRatioInput::addDeactivateCallback(ArFunctor *functor, 
						ArListPos::Pos position)
{
  if (position == ArListPos::FIRST)
    myDeactivateCallbacks.push_front(functor);
  else if (position == ArListPos::LAST)
    myDeactivateCallbacks.push_back(functor);
  else
    ArLog::log(ArLog::Terse, 
	       "ArActionRatioInput::addDeactivateCallback: Invalid position.");
}

AREXPORT void ArActionRatioInput::remDeactivateCallback(ArFunctor *functor)
{
  myDeactivateCallbacks.remove(functor);
}

AREXPORT void ArActionRatioInput::activate(void)
{
  std::list<ArFunctor *>::iterator it;

  if (!isActive())
  {
    myTransRatio = 0;
    myRotRatio = 0;
    myThrottleRatio = 0;
    myLatRatio = 0;
    for (it = myActivateCallbacks.begin(); 
	 it != myActivateCallbacks.end(); 
	 it++)
      (*it)->invoke();
  }
  ArAction::activate();
}

AREXPORT void ArActionRatioInput::deactivate(void)
{
  std::list<ArFunctor *>::iterator it;

  if (!isActive())
  {
    myTransRatio = 0;
    myRotRatio = 0;
    myThrottleRatio = 0;
    myLatRatio = 0;
    for (it = myDeactivateCallbacks.begin(); 
	 it != myDeactivateCallbacks.end(); 
	 it++)
      (*it)->invoke();
  }
  ArAction::deactivate();
}

AREXPORT ArActionDesired *ArActionRatioInput::fire(
	ArActionDesired currentDesired)
{
  std::multimap<int, ArFunctor *>::iterator it;
  //ArLog::log(ArLog::Normal, "Calling");
  // call the callbacks that'll set our ratios
  for (it = myFireCallbacks.begin(); it != myFireCallbacks.end(); it++)
  {
    (*it).second->invoke();
    /*ArLog::log(ArLog::Normal, "Called %s now %g %g %g %g\n", 
	       (*it).second->getName(), 
	       myTransRatio, myRotRatio, myThrottleRatio, myLatRatio);
    */
  }
  
  myDesired.reset();

  if (myPrinting)
    printf("trans %.0f rot %.0f throttle %.0f lat %.0f\n", 
	   myTransRatio, myRotRatio, myThrottleRatio, myLatRatio);

  double fullThrottleForwards, fullThrottleBackwards;
  double rotAtFullForwards, rotAtFullBackwards, rotAtStopped;
  double latAtFullForwards, latAtFullBackwards, latAtStopped;


  if (myFullThrottleForwards < 1)
    fullThrottleForwards = myRobot->getTransVelMax();
  else
    fullThrottleForwards = myFullThrottleForwards;

  if (myFullThrottleBackwards < 1)
    fullThrottleBackwards = myRobot->getTransVelMax() / 4;
  else
    fullThrottleBackwards = myFullThrottleBackwards;

  if (myRotAtFullForwards < 1)
    rotAtFullForwards = myRobot->getRotVelMax() / 2;
  else
    rotAtFullForwards = myRotAtFullForwards;

  if (myRotAtFullBackwards < 1)
    rotAtFullBackwards = myRobot->getRotVelMax() / 2;
  else
    rotAtFullBackwards = myRotAtFullBackwards;

  if (myRotAtStopped < 1)
    rotAtStopped = myRobot->getRotVelMax();
  else
    rotAtStopped = myRotAtStopped;

  if (myLatAtFullForwards < 1)
    latAtFullForwards = myRobot->getLatVelMax() / 2;
  else
    latAtFullForwards = myLatAtFullForwards;

  if (myLatAtFullBackwards < 1)
    latAtFullBackwards = myRobot->getLatVelMax() / 2;
  else
    latAtFullBackwards = myLatAtFullBackwards;

  if (myLatAtStopped < 1)
    latAtStopped = myRobot->getLatVelMax();
  else
    latAtStopped = myLatAtStopped;


  // forwards
  if (myTransRatio > myTransDeadZone)
  {
    myDesired.setVel(myTransRatio/100.0 * 
		     fullThrottleForwards * myThrottleRatio/100.0);
    //double totalThrottle = ArMath::fabs(myTransRatio/100.0 * 
    //myThrottleRatio/100.0);
    double speedRatio = fabs(myRobot->getVel() / fullThrottleForwards);
    if (ArMath::fabs(myRotRatio) < myRotDeadZone)
      myDesired.setRotVel(0);
    else
      myDesired.setRotVel(
	      myRotRatio/100.0 * ((rotAtFullForwards - rotAtStopped) * 
				  speedRatio + rotAtStopped) * 
	      myThrottleRatio/100.0);
    if (myPrinting)
      printf("forwards rot %.0f %.0f %.0f %.0f\n", myRotRatio/100.0, rotAtFullForwards - rotAtStopped, speedRatio, rotAtStopped);
    if (myRobot->hasLatVel() && ArMath::fabs(myLatRatio) < myLatDeadZone)
      myDesired.setLeftLatVel(0);
    else if (myRobot->hasLatVel())
      myDesired.setLeftLatVel(
	      myLatRatio/100.0 * ((latAtFullForwards - latAtStopped) * 
				  speedRatio + latAtStopped) * 
	      myThrottleRatio/100.0);
    if (myPrinting)
      printf("forwards lat %.0f %.0f %.0f %.0f\n", myLatRatio/100.0, latAtFullForwards - latAtStopped, speedRatio, latAtStopped);

  }
  // backwards
  else if (myTransRatio < -myTransDeadZone)
  {
    myDesired.setVel(myTransRatio/100.0 * 
		     fullThrottleBackwards * myThrottleRatio/100.0);
    //double totalThrottle = ArMath::fabs(myTransRatio/100.0 * 
    //myThrottleRatio/100.0);
    double speedRatio = fabs(myRobot->getVel() / fullThrottleForwards);
    if (ArMath::fabs(myRotRatio) < myRotDeadZone)
      myDesired.setRotVel(0);
    else
      myDesired.setRotVel(
	      myRotRatio/100.0 * ((rotAtFullBackwards - rotAtStopped) * 
				speedRatio + rotAtStopped) * 
	      myThrottleRatio/100.0);
    if (myPrinting)
      printf("backwards rot %.0f %.0f %.0f %.0f\n", myRotRatio/100.0, rotAtFullBackwards - rotAtStopped, speedRatio, rotAtStopped);
    if (myRobot->hasLatVel() && ArMath::fabs(myLatRatio) < myLatDeadZone)
      myDesired.setLeftLatVel(0);
    else if (myRobot->hasLatVel())
      myDesired.setLeftLatVel(
	      myLatRatio/100.0 * ((latAtFullBackwards - latAtStopped) * 
				  speedRatio + latAtStopped) * 
	      myThrottleRatio/100.0);
    if (myPrinting)
      printf("backwards lat %.0f %.0f %.0f %.0f\n", myLatRatio/100.0, latAtFullBackwards - latAtStopped, speedRatio, latAtStopped);

  }
  else
  {
    myDesired.setVel(0);
    if (ArMath::fabs(myRotRatio) < myRotDeadZone)
      myDesired.setRotVel(0);
    else
      myDesired.setRotVel(myRotRatio/100.0 * rotAtStopped * 
			  myThrottleRatio/100.0);
    if (myRobot->hasLatVel() && ArMath::fabs(myLatRatio) < myLatDeadZone)
      myDesired.setLeftLatVel(0);
    else if (myRobot->hasLatVel())
      myDesired.setLeftLatVel(myLatRatio/100.0 * latAtStopped * 
			  myThrottleRatio/100.0);
  }
  
  if (myPrinting)
    printf("ratioInput %.0f %.0f %.0f\n", myDesired.getVel(), 
	   myDesired.getRotVel(), myDesired.getLatVel());


  // see if we need to up the decel
  if ((myRobot->getVel() > 0 && myTransRatio < -50) || 
      (myRobot->getVel() < 0 && myTransRatio > 50))
  {
    if (myPrinting)
      printf("Decelerating trans more\n");
    myDesired.setTransDecel(myRobot->getTransDecel() * 3, 
			    ArActionDesired::MIN_STRENGTH);
  }

  // if they have the stick the opposite direction of the velocity
  // then let people crank up the deceleration
  if ((myRobot->getRotVel() > 0 && myRotRatio < -50) || 
      (myRobot->getRotVel() < 0 && myRotRatio > 50))
  {
    if (myPrinting)
      printf("Decelerating rot more\n");
    myDesired.setRotDecel(myRobot->getRotDecel() * 3,
			  ArActionDesired::MIN_STRENGTH);
  }

  // if they have the stick the opposite direction of the velocity
  // then let people crank up the deceleration
  if (myRobot->hasLatVel() && 
      ((myRobot->getLatVel() > 0 && myLatRatio < -50) || 
       (myRobot->getLatVel() < 0 && myLatRatio > 50)))
  {
    if (myPrinting)
      printf("Decelerating lat more\n");
    myDesired.setLatDecel(myRobot->getLatDecel() * 3,
			  ArActionDesired::MIN_STRENGTH);
  }

  return &myDesired;
}
