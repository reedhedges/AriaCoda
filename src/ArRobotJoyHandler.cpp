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
#include "ArRobotJoyHandler.h"
#include "ArRobot.h"
#include "ArCommands.h"
#include "ariaInternal.h"

AREXPORT ArRobotJoyHandler::ArRobotJoyHandler(ArRobot *robot) : 
    myHandleJoystickPacketCB(this, &ArRobotJoyHandler::handleJoystickPacket),
    myConnectCB(this, &ArRobotJoyHandler::connectCallback),
    myStopPacketsCB(this, &ArRobotJoyHandler::stopPackets)
{
  myRobot = robot;

  myHandleJoystickPacketCB.setName("ArRobotJoyHandler");
  myRobot->addConnectCB(&myConnectCB);
  myRobot->addPacketHandler(&myHandleJoystickPacketCB, ArListPos::FIRST);
  if (myRobot->isConnected())
    connectCallback();

  myStarted.setToNow();
  myButton1 = 0;
  myButton2 = 0;
  myJoyX = 0;
  myJoyY = 0;

  myJoyXCenter = 512.0;
  myJoyYCenter = 512.0;

  myRawX = -1;
  myRawX = -1;
  myRawX = -1;

  myThrottle = 1;
  myGotData = false;
}

AREXPORT ArRobotJoyHandler::~ArRobotJoyHandler()
{
  myRobot->remConnectCB(&myConnectCB);
  myRobot->remPacketHandler(&myHandleJoystickPacketCB);
  stopPackets();
  myRobot->remDisconnectNormallyCB(&myStopPacketsCB);
  Aria::remExitCallback(&myStopPacketsCB);
}

AREXPORT void ArRobotJoyHandler::connectCallback(void)
{
  myRobot->addDisconnectNormallyCB(&myStopPacketsCB);
  Aria::addExitCallback(&myStopPacketsCB);
  myRobot->comInt(ArCommands::JOYINFO, 2);
}

void ArRobotJoyHandler::stopPackets()
{
  myRobot->comInt(ArCommands::JOYINFO, 0);
}

AREXPORT bool ArRobotJoyHandler::handleJoystickPacket(ArRobotPacket *packet)
{

  if (packet->getID() != 0xF8)
    return false;
  
  myDataReceived.setToNow();

  if (packet->bufToUByte() != 0)
    myButton1 = true;
  else
    myButton1 = false;

  if (packet->bufToUByte() != 0)
    myButton2 = true;
  else
    myButton2  = false;

  myRawX = packet->bufToUByte2();
  myRawY = packet->bufToUByte2();
  myRawThrottle = packet->bufToUByte2();

  // these should vary between 1 and -1
  if (myJoyXCenter > 511.9 && myJoyXCenter < 512.1)
  {
    myJoyX = -((double)myRawX - 512.0) / 512.0;
  }
  else
  {
    if (myRawX > myJoyXCenter) 
    {
      myJoyX = -(myRawX - myJoyXCenter) / (double)(1024 - myJoyXCenter);
    }
    else if (myRawX < myJoyXCenter)
    {
      myJoyX = -(myRawX - myJoyXCenter) / (double)(myJoyXCenter);
    }
  }

  if (myJoyYCenter > 511.9 && myJoyYCenter < 512.1)
  {
    myJoyY = ((double)myRawY - 512.0) / 512.0;
  }
  else
  {
    if (myRawY > myJoyYCenter) 
    {
      myJoyY = (myRawY - myJoyYCenter) / (double)(1024 - myJoyYCenter);
    }
    else if (myRawY < myJoyYCenter)
    {
      myJoyY = (myRawY - myJoyYCenter) / (double)(myJoyYCenter);
    }
  }

  // these should vary between 1 and 0
  myThrottle = (double)myRawThrottle / 1024.0;

  //%10d.%03d ago 
  //myStarted.secSince(), myStarted.mSecSince() % 1000, 
  /*
  ArLog::log(ArLog::Normal, 
	     "%6.3f %6.3f %5.3f %d %d raw %4d %4d %4d center %4d %4d", 
	     myJoyX, myJoyY, myThrottle, myButton1, myButton2, 
	     myRawX, myRawY, myRawThrottle, myJoyXCenter, myJoyYCenter);
  */
  //  printf("%d %d %g %g %g\n", myButton1, myButton2, myJoyX, myJoyY, myThrottle);
  if (!myGotData)
  {
    ArLog::log(ArLog::Verbose, "Received joystick information from the robot");
    myGotData = true;
  }
  return true;
}

AREXPORT void ArRobotJoyHandler::getDoubles(double *x, double *y, double *z)
{
  if (x != NULL)
    *x = myJoyX;
  if (y != NULL)
    *y = myJoyY;
  if (z != NULL)
    *z = myThrottle;
}

AREXPORT void ArRobotJoyHandler::addToConfig(ArConfig *config, 
					     const char *section)
{
  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::NORMAL);
  config->addParam(
	  ArConfigArg("JoyXCenter", &myJoyXCenter,
		      "The X center", 0.0, 1024.0),
	  section, ArPriority::NORMAL);

  config->addParam(
	  ArConfigArg("JoyYCenter", &myJoyYCenter,
		      "The Y center", 0.0, 1024.0),
	  section, ArPriority::NORMAL);
  config->addParam(ArConfigArg(ArConfigArg::SEPARATOR), section, ArPriority::NORMAL);  
}
