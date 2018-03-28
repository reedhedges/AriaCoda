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
#ifndef ARSIMUTIL_H
#define ARSIMUTIL_H

#include "ariaUtil.h"
#include "ArGPSCoords.h"
#include "ArMutex.h"
#include "ArRobot.h"
#include "ArRobotPacket.h"
#include "ArCommands.h"

/**
  @todo add "relocate" command
*/
class ArSimUtil
{
  ArRobot *myRobot;
  ArRetFunctor1C<bool, ArSimUtil, ArRobotPacket*> myPacketCB;
  ArFunctorC<ArSimUtil> myConnectCB;
  ArMutex myMutex;
  ArTime myLastPacketTimeReceived;
  bool myHaveGPS;
  int mySimInterval, myRealInterval, myLastInterval;
  ArPose myTruePose;
  ArLLACoords myGPSPos;
  ArRetFunctorC<ArPose, ArSimUtil> myTruePoseFunc;
  bool myHaveOdomError;
  ArPose myOdomError;

public:
  ArSimUtil(ArRobot *robot) : 
    myRobot (robot),
    myPacketCB(this, &ArSimUtil::handleRobotPacket),
    myConnectCB(this, &ArSimUtil::connectCB),
    myHaveGPS(false),
    mySimInterval(0), myRealInterval(0), myLastInterval(0),
    myTruePoseFunc(this, &ArSimUtil::getSimTruePose),
    myHaveOdomError(false)
  {
    myRobot->addPacketHandler(&myPacketCB);
    if(myRobot->isConnected())
      connectCB();
    else
      myRobot->addConnectCB(&myConnectCB);
  }

  ~ArSimUtil() 
  {
    myRobot->comInt(ArCommands::SIM_STAT, 0);
    myRobot->remPacketHandler(&myPacketCB);
    myRobot->remConnectCB(&myConnectCB);
    lock(); // wait for any invocations of packet handler function to end
    unlock();
  }

  void lock() { myMutex.lock(); }
  void unlock() { myMutex.unlock(); }

  ArPose getSimTruePose() { ArScopedLock l(myMutex); return myTruePose; }
  double getSimTrueX() { ArScopedLock l(myMutex); return myTruePose.getX(); }
  double getSimTrueY() { ArScopedLock l(myMutex); return myTruePose.getY(); }
  double getSimTrueTh() { ArScopedLock l(myMutex); return myTruePose.getTh(); }
  ArRetFunctor<ArPose> *getSimTruePoseFunc() { ArScopedLock l(myMutex); return &myTruePoseFunc; }
  ArLLACoords getSimGPSPos() { ArScopedLock l(myMutex); return myGPSPos; }

  bool getHaveSimOdomError() { ArScopedLock l(myMutex); return myHaveOdomError; }
  ArPose getSimOdomError() { ArScopedLock l(myMutex); return myOdomError; }

  bool setSimTruePose(const ArPose& pose) {
    ArRobotPacket pkt;
    pkt.setID(ArCommands::SIM_SET_POSE);
    pkt.uByteToBuf(0);
    pkt.byte4ToBuf((ArTypes::Byte4)pose.getX());
    pkt.byte4ToBuf((ArTypes::Byte4)pose.getY());
    pkt.byte4ToBuf((ArTypes::Byte4)pose.getTh());
    pkt.finalizePacket();
    myRobot->lock();
    int r = myRobot->getDeviceConnection()->write(pkt.getBuf(), pkt.getLength());
    myRobot->unlock();
    return (r == pkt.getLength());
  }
    

protected:
  void connectCB()
  {
    myRobot->comInt(ArCommands::SIM_STAT, 2);
    myRobot->remConnectCB(&myConnectCB);
  }

  bool handleRobotPacket(ArRobotPacket *pkt)
  {
    if(pkt->getID() != 0x62) return false;
    lock();
    myLastPacketTimeReceived.setToNow();
    char v = pkt->bufToByte();
    v = pkt->bufToByte();
    ArTypes::UByte4 flags = pkt->bufToUByte4();
    myHaveGPS = flags & ArUtil::BIT1;
    myHaveOdomError = flags & ArUtil::BIT2;
    mySimInterval = pkt->bufToUByte2();
    myRealInterval = pkt->bufToUByte2();
    myLastInterval = pkt->bufToUByte2();
    myTruePose.setX(pkt->bufToByte4());
    myTruePose.setY(pkt->bufToByte4());
    int z = pkt->bufToByte4();
    myTruePose.setTh(pkt->bufToByte4());
    if(myHaveGPS)
    {
      myGPSPos.setLatitude(pkt->bufToByte4() / 10e6);
      myGPSPos.setLongitude(pkt->bufToByte4() / 10e6);
      myGPSPos.setAltitude(pkt->bufToByte4() / 10e6);
    }
    if(myHaveOdomError)
    {
      myOdomError.setX(pkt->bufToByte4() / 10e6);
      myOdomError.setY(pkt->bufToByte4() / 10e6);
      myOdomError.setTh(pkt->bufToByte4() / 10e6);
    }
    unlock();
    return true;
  }
};

#endif
