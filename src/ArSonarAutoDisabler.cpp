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
#include "ArSonarAutoDisabler.h"
#include "ArRobot.h"

AREXPORT ArSonarAutoDisabler::ArSonarAutoDisabler(ArRobot *robot) :
  myUserTaskCB(this, &ArSonarAutoDisabler::userTask),
  mySupressCB(this, &ArSonarAutoDisabler::supress),
  myUnsupressCB(this, &ArSonarAutoDisabler::unsupress),
  mySetAutonomousDrivingCB(this, &ArSonarAutoDisabler::setAutonomousDriving),
  myClearAutonomousDrivingCB(this, &ArSonarAutoDisabler::clearAutonomousDriving)
{
  myRobot = robot;
  myLastMoved.setToNow();
  mySupressed = false;
  myAutonomousDriving = false;

  if (!myRobot->isConnected() || myRobot->getNumSonar() > 0)
  {
    myUserTaskCB.setName("SonarAutoDisabler");
    myRobot->addUserTask("SonarAutoDisabler", -50, &myUserTaskCB);
  }
  else
  {
    ArLog::log(ArLog::Normal, "ArSonarAutoDisabler not active since there are no sonar");
  }
}

AREXPORT ArSonarAutoDisabler::~ArSonarAutoDisabler()
{
  myRobot->remUserTask("SonarAutoDisabler");
}

AREXPORT void ArSonarAutoDisabler::userTask(void)
{
  if (mySupressed && (myRobot->areSonarsEnabled() || 
		      myRobot->areAutonomousDrivingSonarsEnabled()))
  {
    ArLog::log(ArLog::Normal, "SonarAutoDisabler: Supression turning off sonar");
    myRobot->disableSonar();
  }
  
  if (mySupressed)
  {
    myLastSupressed.setToNow();
    return;
  }

  /*
  // if it was supressed in the last few cycles, then don't turn the
  // sonar back on yet... this isn't perfect, but looks like it works
  if ((myLastSupressed.mSecSince() < 175 && fabs(myRobot->getVel()) < 200 &&
	  fabs(myRobot->getRotVel()) < 25) ||
      (myLastSupressed.mSecSince() < 500 && fabs(myRobot->getVel()) < 50 &&
       fabs(myRobot->getRotVel()) < 5))
  {
    return;
  }
  */

  // see if we moved
  /// MPL 2014_04_17 centralizing all the places stopped is calculated
  //if (myRobot->isTryingToMove() || fabs(myRobot->getVel()) > 10 || 
  //fabs(myRobot->getRotVel()) > 5 || 
  //(myRobot->hasLatVel() && fabs(myRobot->getLatVel()) > 10))
  if (myRobot->isTryingToMove() || !myRobot->isStopped())
  {
    myLastMoved.setToNow();
    // if our sonar are disabled and we moved and our motors are
    // enabled then turn 'em on
    if (!myAutonomousDriving && !myRobot->areSonarsEnabled() && 
	myRobot->areMotorsEnabled())
    {
      ArLog::log(ArLog::Normal, 
		 "SonarAutoDisabler: Turning on all sonar (%d %.0f %.0f)",
		 myRobot->isTryingToMove(), fabs(myRobot->getVel()),
		 fabs(myRobot->getRotVel()));
      myRobot->enableSonar();
    }
    // if our sonar are disabled and we moved and our motors are
    // enabled then turn 'em on
    if (myAutonomousDriving && 
	!myRobot->areAutonomousDrivingSonarsEnabled() && 
	myRobot->areMotorsEnabled())
    {
      ArLog::log(ArLog::Normal, 
 "SonarAutoDisabler: Turning on sonar for autonomous driving (%d %.0f %.0f)",
		 myRobot->isTryingToMove(), fabs(myRobot->getVel()),
		 fabs(myRobot->getRotVel()));
      myRobot->enableAutonomousDrivingSonar();
    }
  }
  else
  {
    // if the sonar are on and we haven't moved in a while then turn
    // 'em off
    if ((myRobot->areSonarsEnabled() || 
	 myRobot->areAutonomousDrivingSonarsEnabled()) && 
	myLastMoved.mSecSince() > 1000)
    {
      ArLog::log(ArLog::Normal, "SonarAutoDisabler: Turning off sonar");
      myRobot->disableSonar();
    }
  }
}
