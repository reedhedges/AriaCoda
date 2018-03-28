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
#include "ArActionAvoidFront.h"
#include "ArResolver.h"
#include "ArRobot.h"
#include "ArLog.h"

/**
   @param name the name of the action
   @param obstacleDistance distance at which to turn. (mm)
   @param avoidVelocity Speed at which to go while avoiding an obstacle. 
   (mm/sec)
   @param turnAmount Degrees to turn relative to current heading while 
   avoiding obstacle (deg)
   @param useTableIRIfAvail Whether to use the table sensing IR if they are 
   available
*/
AREXPORT ArActionAvoidFront::ArActionAvoidFront(const char *name, 
						double obstacleDistance,
						double avoidVelocity,
						double turnAmount, 
						bool useTableIRIfAvail) :
  ArAction(name, "Slows down and avoids obstacles in front of the robot.")
{
  setNextArgument(ArArg("obstacle distance", &myObsDist, 
			"Distance at which to turn. (mm)"));
  myObsDist = obstacleDistance;
  
  setNextArgument(ArArg("avoid speed", &myAvoidVel,
	"Speed at which to go while avoiding an obstacle. (mm/sec)"));
  myAvoidVel = avoidVelocity;

  setNextArgument(ArArg("turn ammount", &myTurnAmountParam, 
	"Degrees to turn relative to current heading while avoiding obstacle (deg)"));
  myTurnAmountParam = turnAmount;

  setNextArgument(ArArg("use table IR", &myUseTableIRIfAvail,
		"true to use table sensing IR for avoidance if the robot has them, false otherwise"));
  myUseTableIRIfAvail = useTableIRIfAvail;

  myTurning = 0;
}

AREXPORT ArActionAvoidFront::~ArActionAvoidFront()
{

}

AREXPORT ArActionDesired *ArActionAvoidFront::fire(ArActionDesired currentDesired)
{
  double dist, angle;

  if (currentDesired.getDeltaHeadingStrength() >= 1.0)
    myTurning = 0;

  myDesired.reset();

  dist = (myRobot->checkRangeDevicesCurrentPolar(-70, 70, &angle) 
	  - myRobot->getRobotRadius());
  
  //  printf("%5.0f %3.0f ", dist, angle);

  if (dist > myObsDist && 
      (!myUseTableIRIfAvail || 
       (myUseTableIRIfAvail && !myRobot->hasTableSensingIR()) || 
       (myUseTableIRIfAvail && myRobot->hasTableSensingIR() && 
       !myRobot->isLeftTableSensingIRTriggered() &&
       !myRobot->isRightTableSensingIRTriggered())))
  {
    if (myTurning != 0)
    {
      myDesired.setDeltaHeading(0);
      myTurning = 0;
      return &myDesired;
    }
    else
    {
      //printf("\n");
      myTurning = 0;
      return NULL;
    }
  }
  
//  printf("Avoiding ");
  
  if (myTurning == 0)
  {
    if (myUseTableIRIfAvail && myRobot->hasTableSensingIR() && 
        myRobot->isLeftTableSensingIRTriggered())
      myTurning = 1;
    else if (myUseTableIRIfAvail && myRobot->hasTableSensingIR() && 
             myRobot->isRightTableSensingIRTriggered())
      myTurning = -1;
    else if (angle < 0)
      myTurning = 1;
    else
      myTurning = -1;
    myTurnAmount = myTurnAmountParam;
    myQuadrants.clear();
  }

  myQuadrants.update(myRobot->getTh());
  if (myTurning && myQuadrants.didAll())
  {
    myQuadrants.clear();
    myTurnAmount /= 2;
    if (myTurnAmount == 0)
      myTurnAmount = myTurnAmountParam;
  }

  myDesired.setDeltaHeading(myTurning * myTurnAmount);

  if (dist > myObsDist/2 && 
      (!myUseTableIRIfAvail || 
       (myUseTableIRIfAvail && !myRobot->hasTableSensingIR()) || 
       (myUseTableIRIfAvail && myRobot->hasTableSensingIR() && 
       !myRobot->isLeftTableSensingIRTriggered() &&
       !myRobot->isRightTableSensingIRTriggered())))
  {
    //printf(" scaling %f %f %f ", myAvoidVel * dist / myObsDist, 
    //dist, myObsDist);
    myDesired.setVel(myAvoidVel * dist / myObsDist);
  }
  else
  {
//   printf(" zerovel\n");
    myDesired.setVel(0);
  }
    
  //printf("\n");
  return &myDesired;
}
