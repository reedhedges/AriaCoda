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
#include "ArExport.h"
#include "ArConfig.h"
#include "ArRobotConfig.h"
#include "ArRobot.h"
#include "ArAnalogGyro.h"
#include "ariaInternal.h"

AREXPORT ArRobotConfig::ArRobotConfig(ArRobot *robot) : 
  myConnectCB(this, &ArRobotConfig::connectCallback),
  myProcessFileCB(this, &ArRobotConfig::processFile)
{
  myRobot = robot;

  myAddedMovementParams = false;
  myTransVelMax = 0;
  myTransAccel = 0;
  myTransDecel = 0;
  myRotVelMax = 0;
  myRotAccel= 0;
  myRotDecel = 0;

  mySavedOriginalMovementParameters = false;
  myOriginalTransVelMax = 0;
  myOriginalTransAccel = 0;
  myOriginalTransDecel = 0;
  myOriginalRotVelMax = 0;
  myOriginalRotAccel= 0;
  myOriginalRotDecel = 0;


  myAnalogGyro = NULL;
  myUseGyro = true;
  myAddedGyro = false;

  myConnectCB.setName("ArRobotConfig");
  myProcessFileCB.setName("ArRobotConfig");

  myRobot->addConnectCB(&myConnectCB, ArListPos::FIRST);
  Aria::getConfig()->addProcessFileCB(&myProcessFileCB, 98);

  if (myRobot->isConnected())
    connectCallback();
}

AREXPORT ArRobotConfig::~ArRobotConfig()
{
}

AREXPORT bool ArRobotConfig::processFile(void)
{
    if (myTransVelMax != 0)
    myRobot->setTransVelMax(myTransVelMax);
  else
    myRobot->setTransVelMax(myOriginalTransVelMax);

  if (myTransAccel != 0)
    myRobot->setTransAccel(myTransAccel);
  else
    myRobot->setTransAccel(myOriginalTransAccel);

  if (myTransDecel != 0)
    myRobot->setTransDecel(myTransDecel);
  else
    myRobot->setTransDecel(myOriginalTransDecel);

  if (myRotVelMax != 0)
    myRobot->setRotVelMax(myRotVelMax);
  else
    myRobot->setRotVelMax(myOriginalRotVelMax);

  if (myRotAccel!= 0)
    myRobot->setRotAccel(myRotAccel);
  else
    myRobot->setRotAccel(myOriginalRotAccel);

  if (myRotDecel != 0)
    myRobot->setRotDecel(myRotDecel);
  else
    myRobot->setRotDecel(myOriginalRotDecel);

  if (myAnalogGyro != NULL && myAddedGyro)
  {
    if (myUseGyro && !myAnalogGyro->isActive())
    {
      ArLog::log(ArLog::Normal, "Gyro(analog) activated");
      myAnalogGyro->activate();
    }
    else if (!myUseGyro && myAnalogGyro->isActive())
    {
      ArLog::log(ArLog::Normal, "Gyro(analog) deactivated");
      myAnalogGyro->deactivate();
    }
  }
  
  return true;
}

AREXPORT void ArRobotConfig::addAnalogGyro(ArAnalogGyro *gyro)
{
  myAnalogGyro = gyro;
  if (myRobot->isConnected())
    connectCallback();
}

AREXPORT void ArRobotConfig::connectCallback(void)
{
  std::string section;
  section = "Robot config";

  if (!mySavedOriginalMovementParameters)
  {
    mySavedOriginalMovementParameters = true;
    myOriginalTransVelMax = ArMath::roundInt(myRobot->getTransVelMax());
    myOriginalTransAccel = ArMath::roundInt(myRobot->getTransAccel());
    myOriginalTransDecel = ArMath::roundInt(myRobot->getTransDecel());
    myOriginalRotVelMax = ArMath::roundInt(myRobot->getRotVelMax());
    myOriginalRotAccel = ArMath::roundInt(myRobot->getRotAccel());
    myOriginalRotDecel = ArMath::roundInt(myRobot->getRotDecel());
  }

  if (!myAddedMovementParams)
  {
    myAddedMovementParams = true;
    Aria::getConfig()->addParam(
	    ArConfigArg("TransVelMax", &myTransVelMax, 
			"maximum translational speed (mm/sec) (0 means use original value)",
			0, (int)myRobot->getAbsoluteMaxTransVel()), 
	    section.c_str(), ArPriority::TRIVIAL);
    Aria::getConfig()->addParam(
	    ArConfigArg("TransAccel", &myTransAccel, 
			"translational acceleration (mm/sec/sec) (0 means use original value)",
			0, (int)myRobot->getAbsoluteMaxTransAccel()), 
	    section.c_str(), ArPriority::TRIVIAL);
    Aria::getConfig()->addParam(
	    ArConfigArg("TransDecel", &myTransDecel, 
			"translational deceleration (mm/sec/sec) (0 means use original value)",
			0, (int)myRobot->getAbsoluteMaxTransDecel()), 
	    section.c_str(), ArPriority::TRIVIAL);
    Aria::getConfig()->addParam(
	    ArConfigArg("RotVelMax", &myRotVelMax, 
			"maximum rotational speed (deg/sec) (0 means use original value)",
			0, (int)myRobot->getAbsoluteMaxRotVel()), 
	    section.c_str(), ArPriority::TRIVIAL);
    Aria::getConfig()->addParam(
	    ArConfigArg("RotAccel", &myRotAccel, 
			"rotational acceleration (deg/sec/sec) (0 means use original value)",
			0, (int)myRobot->getAbsoluteMaxRotAccel()), 
	    section.c_str(), ArPriority::TRIVIAL);
    Aria::getConfig()->addParam(
	    ArConfigArg("RotDecel", &myRotDecel, 
			"rotational deceleration (deg/sec/sec) (0 means use original value)",
			0, (int)myRobot->getAbsoluteMaxRotDecel()), 
	    section.c_str(), ArPriority::TRIVIAL);
  }

  if (myAnalogGyro != NULL && !myAddedGyro && 
      myAnalogGyro->haveGottenData())
  {
    myAddedGyro = true;
    Aria::getConfig()->addParam(
	    ArConfigArg("UseGyro", &myUseGyro, 
			"True to use the gyro, false not to"),
	    section.c_str(), ArPriority::TRIVIAL);

  }

}
