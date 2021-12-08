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


*/
#include "Aria/ArExport.h"
#include "Aria/ariaOSDef.h"
#include "Aria/ArRobotPacketReaderThread.h"
#include "Aria/ArLog.h"
#include "Aria/ariaUtil.h"
#include "Aria/ArRobot.h"


AREXPORT ArRobotPacketReaderThread::ArRobotPacketReaderThread() :
  ArASyncTask(),
  myStopRunIfNotConnected(false),
  myRobot(0)
{
  setThreadName("ArRobotPacketReader");
  myInRun = false;
}

/* AREXPORT ArRobotPacketReaderThread::~ArRobotPacketReaderThread()
{
} */

AREXPORT void ArRobotPacketReaderThread::setRobot(ArRobot *robot)
{
  myRobot=robot;
}

AREXPORT void ArRobotPacketReaderThread::stopRunIfNotConnected(bool stopRun)
{
  myStopRunIfNotConnected = stopRun;
}

AREXPORT void * ArRobotPacketReaderThread::runThread(void *)
{
  threadStarted();

  if (!myRobot)
  {
    ArLog::log(ArLog::Terse, "ArRobotPacketReaderThread::runThread: Trying to run the robot packet reader without a robot.");
    return(0);
  }

  // this skips the normal one, since the function will exit when the robot run stops
  myRobot->packetHandlerThreadedReader();
  
  /*
  while (myRunning)
  {
    myInRun = true;
    myRobot->packetHandlerThreadedReader();
    myInRun = false;
  }
  */

  threadFinished();
  return(0);
}


