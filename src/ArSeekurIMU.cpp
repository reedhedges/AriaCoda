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
#include "ariaUtil.h"
#include "ArCommands.h"
#include "ArSeekurIMU.h"
#include "ArRobot.h"

AREXPORT ArSeekurIMU::ArSeekurIMU(ArRobot *robot) : 
  myHaveData(false),
  myNumAxes(0),
  myAvgTemperature(0),
  myRobot(robot),
  myHandleIMUPacketCB(this, &ArSeekurIMU::handleIMUPacket),
  myStabilizingCB(this, &ArSeekurIMU::stabilizingCallback)
{
  mySpeed[X] = mySpeed[Y] = mySpeed[Z] = 0.0;
  myAccel[X] = myAccel[Y] = myAccel[Z] = 0.0;
  myPos[X] = myPos[Y] = myPos[Z] = 0.0;

  myHandleIMUPacketCB.setName("ArSeekurIMU");
  myRobot->addPacketHandler(&myHandleIMUPacketCB);
  myRobot->addStabilizingCB(&myStabilizingCB);

  if (myRobot->isConnected())
    stabilizingCallback();
}

AREXPORT ArSeekurIMU::~ArSeekurIMU()
{
  myRobot->comInt(ArCommands::SEEKURIMU, 0);
  myRobot->remPacketHandler(&myHandleIMUPacketCB);
  myRobot->remStabilizingCB(&myStabilizingCB);
}

void ArSeekurIMU::stabilizingCallback(void)
{
  //if (myRobot->getOrigRobotConfig() != NULL &&
  //    myRobot->getOrigRobotConfig()->getHasIMU() == 1)
  {
    if (!myRobot->isConnected())
      ArLog::log(ArLog::Normal, "Waiting for IMU to stabilize/calibrate");
    if (!myRobot->isConnected() && myRobot->getStabilizingTime() < 3000)
      myRobot->setStabilizingTime(3000);
    myRobot->comInt(ArCommands::IMUREQUEST, 2);
  }
}

bool ArSeekurIMU::handleIMUPacket(ArRobotPacket *packet)
{
  ArTime timeThisPacketReceived; // gets set to now

  if (packet->getID() != 0x9A)
    return false; // give packet to other handlers

  /*int time =*/ packet->bufToByte();

  int nr = packet->bufToByte(); // number of readings
  if (nr <= 0) return true;

  double reading_interval_ms = (timeThisPacketReceived.mSecSince() - myTimeLastPacketReceived.mSecSince()) / nr;

  myTimeLastPacketReceived = timeThisPacketReceived;
  myHaveData = true;

  for (int r = 0; r < nr; ++r)
  {

    // velocity data: 

    int na = packet->bufToByte(); // number of axes 

    int rangemode = packet->bufToByte();
    double convert;
    if(rangemode == 1)
      convert = 0.07326;
    else if(rangemode == 2)
      convert = 0.03663;
    else if(rangemode == 3)
      convert = 0.01832;
    else
    {
      ArLog::log(ArLog::Terse, "ArSeekurIMU: Warning: Invalid range mode indicated (%d), ignoring rest of this packet.", rangemode);
      myHaveData = false;
      return true;
    }
    
    if(na == 1)
    {
      mySpeed[Z] = (packet->bufToByte2() * convert);
    }
    else if (na == 3)
    {
      for(int a = 0; a < na && a < 3; ++a)
        mySpeed[a] = (packet->bufToByte2() * convert);
    }
    else
    {
      ArLog::log(ArLog::Terse, "ArSeekurIMU: Warning: Unexpected number of axes indicated in IMU packet! (%d, expected 3 or 1). Malformed packet?", na);
      myHaveData = false;
    }


    // acceleration data:

    na = packet->bufToByte();

    convert = 0.002522 * 9806.65; // convert to deg/s/s?

    if (na == 3)
    {
      for(int a = 0; a < na && a < 3; ++a)
        myAccel[a] = (packet->bufToByte2() * convert);
    }
    else if(na != 0)
    {
      ArLog::log(ArLog::Terse, "ArSeekurIMU: Warning: Unexpected number of acceleration axes indicated in IMU packet! (%d, expected 3 or 0). Malformed packet?", na);
      myHaveData = false;
    }


    // temperature:

    na = packet->bufToByte();

    convert = 0.1453; 
    const double offset = 25.0;

    if(na == 1)
    {
      myAvgTemperature = (packet->bufToByte2() * convert) + offset;
    }
    else if (na == 3)
    {
      myAvgTemperature = 0;
      int a;
      for(a = 0; a < na && a < 3; ++a)
        myAvgTemperature += (packet->bufToByte2() * convert) + offset;
      myAvgTemperature /= a;
    }
    else
    {
      ArLog::log(ArLog::Terse, "ArSeekurIMU: Warning: Unexpected number of temperature axes indicated in IMU packet! (%d, expected 3 or 1). Malformed packet?", na);
      myHaveData = false;
    }


    // TODO might want to test for small drift while we think we are stationary
    // and ignore it.  Could also use averages over time to smooth this, and/or 
    // use acceleration instead of speed.

    // Update position
  
    myPos[X] += mySpeed[X] / (reading_interval_ms * 1000.0);
    myPos[Y] += mySpeed[Y] / (reading_interval_ms * 1000.0);
    myPos[Z] += mySpeed[Z] / (reading_interval_ms * 1000.0);
    
  }

  return true;
}

