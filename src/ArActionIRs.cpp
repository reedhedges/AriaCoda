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
#include "ArActionIRs.h"
#include "ArRobot.h"
#include "ArCommands.h"

/**
   @param name name of the action
   @param backOffSpeed speed at which to back away (mm/sec)
   @param backOffTime number of msec to back up for (msec)
   @param turnTime number of msec to alow for turn (msec)
   @param setMaximums if true, set desired maximum speed limits to backOffSpeed when performing the action; otherwise use existing speed limits.
*/
AREXPORT ArActionIRs::ArActionIRs(const char *name, 
					  double backOffSpeed,
					  int backOffTime, int turnTime,
					  bool setMaximums) :
  ArAction(name, "Reacts to the IRs triggering")
{
  setNextArgument(ArArg("back off speed", &myBackOffSpeed, 
			"Speed at which to back away (mm/sec)"));
  myBackOffSpeed = backOffSpeed;

  setNextArgument(ArArg("back off time", &myBackOffTime,
			"Number of msec to back up for (msec)"));
  myBackOffTime = backOffTime;

  myStopTime = 1000;

  setNextArgument(ArArg("turn time", &myTurnTime,
			"Number of msec to allow for turn (msec)"));
  myTurnTime = turnTime;

  setNextArgument(ArArg("set maximums", &mySetMaximums,
			"Whether to set maximum vels or not (bool)"));
  mySetMaximums = setMaximums;
  
  myFiring = false; 
  mySpeed = 0.0;
  myHeading = 0.0;
}

AREXPORT ArActionIRs::~ArActionIRs()
{

}

AREXPORT void ArActionIRs::setRobot(ArRobot *robot)
{
  myRobot = robot;
  const ArRobotParams *params;
  params = myRobot->getRobotParams();
  myParams = *params;
  
  for(int i = 0; i < myParams.getNumIR(); i++)
    cycleCounters.push_back(1);
}

AREXPORT ArActionDesired *ArActionIRs::fire(ArActionDesired currentDesired)
{
  myDesired.reset();

  double angle = 0;
  int counter = 0;
  double turnRange = 135;

  ArUtil::BITS bit;

  if(myFiring)
    {  
      if (myStartBack.mSecSince() < myBackOffTime)
	{
	  myDesired.setVel(mySpeed);
	  myDesired.setDeltaHeading(0);
	  return &myDesired;
	}
      else if (myStartBack.mSecSince() < myBackOffTime + myTurnTime &&
	     ArMath::fabs(ArMath::subAngle(myRobot->getTh(), myHeading)) > 3)
	{
	  myDesired.setVel(0);
	  myDesired.setHeading(myHeading);
	  return &myDesired;
	}  
      else if(stoppedSince.mSecSince() < myStopTime)
	{
	  myDesired.setVel(0);
	  myDesired.setDeltaHeading(0);
	  return &myDesired;
	}
    
      myFiring = false;
    }


  if(myParams.haveTableSensingIR())
    {
      for (int i = 0; i < myParams.getNumIR(); ++i)
	{
	  switch(i)
	    {
	    case 0:
	      bit = ArUtil::BIT0;
	      break;
	    case 1:
	      bit = ArUtil::BIT1;
	      break;
	    case 2:
	      bit = ArUtil::BIT2;
	      break;
	    case 3:
	      bit = ArUtil::BIT3;
	      break;
	    case 4:
	      bit = ArUtil::BIT4;
	      break;
	    case 5:
	      bit = ArUtil::BIT5;
	      break;
	    case 6:
	      bit = ArUtil::BIT6;
	      break;
	    case 7:
	      bit = ArUtil::BIT7;
	      break;
	    }
	  if(myParams.haveNewTableSensingIR() && myRobot->getIODigInSize() > 3)
	    {
	      if((myParams.getIRType(i) && !(myRobot->getIODigIn(3) & bit)) ||
		 (!myParams.getIRType(i) && (myRobot->getIODigIn(3) & bit)))
	      {
		if(cycleCounters[i] < myParams.getIRCycles(i))
		  {
		    cycleCounters[i] = cycleCounters[i] + 1;	      
		  }
		else
		  {
		    cycleCounters[i] = 1;
		   
		    ArPose pose;
		    pose.setX(myParams.getIRX(i));
		    pose.setY(myParams.getIRY(i));
		    if(pose.getX() > 0)
		      {
			ArPose center(0,0,0);
			angle += center.findAngleTo(pose);
			counter++;
		      }
		  }
	      }
	      else
		{
		  cycleCounters[i] = 1;
		}
	    }
	  else
	    {
	      if(!(myRobot->getDigIn() & bit))
	      {
		if(cycleCounters[i] < myParams.getIRCycles(i))
		  {
		    cycleCounters[i] = cycleCounters[i] + 1;	      
		  }
		else
		  {
		    cycleCounters[i] = 1;
		    
		    ArPose pose;
		    pose.setX(myParams.getIRX(i));
		    pose.setY(myParams.getIRY(i));
		    if(pose.getX() > 0)
		      {
			ArPose center(0,0,0);
			angle += center.findAngleTo(pose);
			counter++;
		      }
		  }
	      }
	      else
		{
		  cycleCounters[i] = 1;
		}
	      
	    }
	}
 
      if(counter > 0 && myRobot->getVel() > 50)
	{
	  angle = angle / (double) counter;
	  if(angle > (turnRange / 2))
	    angle = turnRange / 2;
	  else if(angle < -(turnRange / 2))
	    angle = -(turnRange / 2);
	  
	  if(angle < 0) angle = ((turnRange / 2) + angle) * -1;
	  else angle = ((turnRange / 2) - angle);
	
	  myHeading = ArMath::addAngle(myRobot->getTh(), angle);
	  mySpeed = -myBackOffSpeed;
	  myStartBack.setToNow();
	  ArLog::log(ArLog::Normal, "ArActionIRS: estopping");
	  myRobot->comInt(ArCommands::ESTOP, 0);
	  myFiring = true;
	  
	  myDesired.setVel(mySpeed);
	  myDesired.setHeading(myHeading);
  
	}
      else if(counter > 0 && (myRobot->getVel() > -50 && myRobot->getVel() < 50))
	{
	  stoppedSince.setToNow();
	}
      else return NULL;
    }
  else return NULL;
  

  return &myDesired;
}  
