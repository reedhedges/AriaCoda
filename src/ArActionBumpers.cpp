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
#include "ArActionBumpers.h"
#include "ArRobot.h"

/**
   @param name name of the action instance
   @param backOffSpeed speed at which to back away (mm/sec)
   @param backOffTime number of milisecons to back up for (msec)
   @param turnTime number of milisecons to alow for turn (msec)
   @param setMaximums if true, set desired maximum translation velocity limits to backOffSpeed while performing the action; if false, retain existing limits.
*/
AREXPORT ArActionBumpers::ArActionBumpers(const char *name, 
					  double backOffSpeed,
					  int backOffTime, int turnTime,
					  bool setMaximums) :
  ArAction(name, "Reacts to the bumpers triggering")
{
  setNextArgument(ArArg("back off speed", &myBackOffSpeed, 
			"Speed at which to back away (mm/sec)"));
  myBackOffSpeed = backOffSpeed;

  setNextArgument(ArArg("back off time", &myBackOffTime,
			"Number of msec to back up for (msec)"));
  myBackOffTime = backOffTime;

  //myStopTime = 1000;

  setNextArgument(ArArg("turn time", &myTurnTime,
			"Number of msec to allow for turn (msec)"));
  myTurnTime = turnTime;

  setNextArgument(ArArg("set maximums", &mySetMaximums,
			"Whether to set maximum vels or not (bool)"));
  mySetMaximums = setMaximums;
  
  myFiring = false;
  mySpeed = 0.0;
  myHeading = 0.0;

  // MPL I wrote this code, but checking for BIT8 makes no sense, BIT0 is the stall, BIT8 would be beyond this data
  myBumpMask = (ArUtil::BIT1 | ArUtil::BIT2 | ArUtil::BIT3 | ArUtil::BIT4 | 
		ArUtil::BIT5 | ArUtil::BIT6 | ArUtil::BIT7 | ArUtil::BIT8); 
}

AREXPORT ArActionBumpers::~ArActionBumpers()
{

}

AREXPORT void ArActionBumpers::activate(void)
{
  myFiring = false;
  ArAction::activate();
}

AREXPORT double ArActionBumpers::findDegreesToTurn(int bumpValue, int whichBumper)
{
  double totalTurn = 0;
  int numTurn = 0;
  int numBumpers;

  double turnRange = 135;

  if(whichBumper == 1) numBumpers = myRobot->getNumFrontBumpers();
  else numBumpers = myRobot->getNumRearBumpers();

  for (int i = 0; i < numBumpers; i++)
    {
      // MPL I wrote this code, but checking for BIT8 makes no sense, BIT0 is  the stall, BIT8 would be beyond this data
      if((i == 0 && (bumpValue & ArUtil::BIT1)) || 
	 (i == 1 && (bumpValue & ArUtil::BIT2)) ||
	 (i == 2 && (bumpValue & ArUtil::BIT3)) || 
	 (i == 3 && (bumpValue & ArUtil::BIT4)) ||
	 (i == 4 && (bumpValue & ArUtil::BIT5)) || 
	 (i == 5 && (bumpValue & ArUtil::BIT6)) ||
	 (i == 6 && (bumpValue & ArUtil::BIT7)) || 
	 (i == 7 && (bumpValue & ArUtil::BIT8)))
	{
	  totalTurn = totalTurn +  (i * (turnRange / (double)numBumpers) + 
				    ((turnRange / (double)numBumpers) / 2) - (turnRange / 2));
	  ++numTurn;
	}
    } 
  
  totalTurn = totalTurn / (double)numTurn;

  if(totalTurn < 0) totalTurn = ((turnRange / 2) + totalTurn) * -1;
  else totalTurn = ((turnRange / 2) - totalTurn);

  return totalTurn;
}

AREXPORT ArActionDesired *ArActionBumpers::fire(ArActionDesired currentDesired)
{
  int frontBump;
  int rearBump;
  int whichBumper;

  if (myRobot->hasFrontBumpers())
    frontBump = ((myRobot->getStallValue() & 0xff00) >> 8) & myBumpMask;
  else
    frontBump = 0;
  if (myRobot->hasRearBumpers())
    rearBump = (myRobot->getStallValue() & 0xff) & myBumpMask;
  else
    rearBump = 0;

  if (frontBump != 0)
    ArLog::log(ArLog::Verbose, "########## Front bump %x\n", frontBump);
  if (rearBump != 0)
    ArLog::log(ArLog::Verbose, "########## Rear bump %x\n", rearBump);


  myDesired.reset();
  if (myFiring)
  {
    if (myStartBack.mSecSince() < myBackOffTime)
    {
      if ((mySpeed < 0 && rearBump != 0) ||
	  (mySpeed > 0 && frontBump != 0))
      {
	//printf("0 norot\n");
	myDesired.setVel(0);
      }
      else
      {
	//printf("%.0f norot\n", mySpeed);
	myDesired.setVel(mySpeed);
      }
      
      myDesired.setDeltaHeading(0);
      return &myDesired;
    }
    else if (myStartBack.mSecSince() < myBackOffTime + myTurnTime &&
	     ArMath::fabs(ArMath::subAngle(myRobot->getTh(), myHeading)) > 3)
    {
      //printf("0 %.0f\n", myHeading);
      myDesired.setVel(0);
      myDesired.setHeading(myHeading);
      return &myDesired;
    }
    /*
      else if (myStoppedSince.mSecSince() < myStopTime)
      {
      myDesired.setVel(0);
      myDesired.setDeltaHeading(0);
      return &myDesired;
      }
    */
    myFiring = false;
    ArLog::log(ArLog::Normal, "Bumpers: done");
  }
  
  if (myRobot->getVel() > 25)
  {
    if (frontBump == 0)
      return NULL;
    whichBumper = 1;
    ArLog::log(ArLog::Normal, 
	       "Bumpers: Bumped a forward bumper while going forward, turning %.0f",
	       findDegreesToTurn(frontBump, whichBumper));
    myHeading = ArMath::addAngle(myRobot->getTh(), 
				 findDegreesToTurn(frontBump, whichBumper));
    mySpeed = -myBackOffSpeed;
    myStartBack.setToNow();
  }
  else if (myRobot->getVel() < -25)
  {
    if (rearBump == 0)
      return NULL;
    whichBumper = 2;
    ArLog::log(ArLog::Normal, 
	       "Bumpers: Bumped a rear bumper while going backwards, turning %.0f \n",
	       findDegreesToTurn(rearBump, whichBumper));
    myHeading = ArMath::subAngle(myRobot->getTh(), 
				 findDegreesToTurn(rearBump, whichBumper));
    mySpeed = myBackOffSpeed;
    myStartBack.setToNow();
  } 
  /*
    else if (myRobot->getVel() >= -5 && myRobot->getVel() <= 5)
    {
    if (frontBump == 0 && rearBump == 0) 
    return NULL;
    myStoppedSince.setToNow();
    }
  */
  else
  {
    return NULL;
  }
  
  //myStartBack.setToNow();
  myFiring = true;
  myDesired.setVel(mySpeed);
  myDesired.setHeading(myHeading);
  // Set the maximums if that feature is desired
  if (mySetMaximums)
  {
    if (mySpeed > 0)
      myDesired.setMaxVel(mySpeed);
    else
      myDesired.setMaxNegVel(mySpeed);
  }
  return &myDesired;
}  
