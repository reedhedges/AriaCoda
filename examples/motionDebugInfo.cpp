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

#include "Aria.h"

/* This program requests a set of motion debugging info. Most of this can be
 * supplied by MARC.  A few items may be supplied by SeekurOS.   ARCOS and uARCS
 * do not provide this info (no data will be returned.) */ 

bool handleAbsoluteMax(ArRobotPacket *pkt)
{
  if(pkt->getID() != 213) return false;
  printf("AbsoluteMaxTransVel %d\n", pkt->bufToByte2());
  printf("AbsoluteMaxTransNegVel -%d\n", pkt->bufToByte2());
  printf("AbsoluteMaxTransAccel %d\n", pkt->bufToByte2());
  printf("AbsoluteMaxTransDecel %d\n", pkt->bufToByte2());
  printf("AbsoluteMaxRotVel %d\n", pkt->bufToByte2());
  printf("AbsoluteMaxRotAccel %d\n", pkt->bufToByte2());
  printf("AbsoluteMaxRotDecel %d\n", pkt->bufToByte2());
  printf("AbsoluteMaxLatVel %d\n", pkt->bufToByte2());
  printf("AbsoluteMaxLatAccel %d\n", pkt->bufToByte2());
  printf("AbsoluteMaxLatDecel %d\n", pkt->bufToByte2());
  return true;
}

bool handleWheelInfo(ArRobotPacket *pkt)
{
  if(pkt->getID() != 218) return false;
  std::string s = pkt->bufToString();
  printf("WheelInfo: %s\n", s.c_str());
  return true;
}

bool handleAccelData(ArRobotPacket *pkt)
{
  if(pkt->getID() != 123) return false;
  unsigned char status = pkt->bufToUByte();
  printf("AccelerometerData: status=0x%x, ok=%d, tilt=%d\n", status, status&ArUtil::BIT0, status&ArUtil::BIT1);
  printf("AccelerometerData: theta=%f deg\n", pkt->bufToByte2() / 100.0);
  printf("AccelerometerData: RMS1=%f\n", pkt->bufToByte2() / 1000.0);
  printf("AccelerometerData: Max1=%f\n", pkt->bufToByte2() / 1000.0);
  printf("AccelerometerData: RMS2=%f\n", pkt->bufToByte2() / 1000.0);
  printf("AccelerometerData: Max2=%f\n", pkt->bufToByte2() / 1000.0);
  printf("AccelerometerData: RMS3=%f\n", pkt->bufToByte2() / 1000.0);
  printf("AccelerometerData: Max3=%f\n", pkt->bufToByte2() / 1000.0);
  return true;
}

bool handleTorqueData(ArRobotPacket *pkt)
{
  if(pkt->getID() != 124) return false;
  unsigned char flags = pkt->bufToUByte();
  printf("TorqueData: flags=0x%x\n", flags);
  for(int i = 0; i < 4; ++i)
  {
    printf("TorqueData: drive motor %d: current=%f, command=%f deg/s, speed=%f deg/s\n", 
      i, pkt->bufToByte2()/1000.0, pkt->bufToByte2()/10.0, pkt->bufToByte2()/10.0);
  }
  for(int i = 0; i < 4; ++i)
  {
    printf("TorqueData: steering motor %d: current=%f, command=%f deg, position=%f deg\n", 
      i, pkt->bufToByte2()/1000.0, pkt->bufToByte2()/10.0, pkt->bufToByte2()/10.0);
  }
  printf("TorqueData: EncRotVel=%f, GyroRotVel=%f, RotVelCommand=%f\n",
    pkt->bufToByte2()/10.0,
    pkt->bufToByte2()/10.0,
    pkt->bufToByte2()/10.0
  );
  return true;
}

bool handleHeadingInfo(ArRobotPacket *pkt)
{
  if(pkt->getID() != 149) return false;
  unsigned char flags = pkt->bufToUByte();
  printf("HeadingInfo: EncoderRotVel=%f deg/s\n", pkt->bufToByte2() / 10.0);
  printf("HeadingInfo: GyroRotVel=%f deg/s\n", pkt->bufToByte2() / 10.0);
  printf("HeadingInfo: RobotRotVel=%f deg/s\n", pkt->bufToByte2() / 10.0);
  printf("HeadingInfo: RotVelCmd=%f deg/s\n", pkt->bufToByte2() / 10.0);
  printf("HeadingInfo: RobotHeading=%f deg\n", pkt->bufToByte2() / 10.0);
  printf("HeadingInfo: GyroOffset=%f deg\n", pkt->bufToByte2() / 10.0);
  printf("HeadingInfo: TimeSinceGyroCentered=%d\n", pkt->bufToByte2());
  return true;
}

int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;

  robot.addPacketHandler(new ArGlobalRetFunctor1<bool, ArRobotPacket*>(&handleAbsoluteMax));
  robot.addPacketHandler(new ArGlobalRetFunctor1<bool, ArRobotPacket*>(&handleWheelInfo));
  robot.addPacketHandler(new ArGlobalRetFunctor1<bool, ArRobotPacket*>(&handleAccelData));
  robot.addPacketHandler(new ArGlobalRetFunctor1<bool, ArRobotPacket*>(&handleTorqueData));
  robot.addPacketHandler(new ArGlobalRetFunctor1<bool, ArRobotPacket*>(&handleHeadingInfo));

  ArRobotConnector robotConnector(&parser, &robot);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "motionDebugInfo: Could not connect to the robot.");
    if(parser.checkHelpAndWarnUnparsed())
    {
        // -help not given
        Aria::logOptions();
        Aria::exit(1);
    }
  }

  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(1);
  }

  ArLog::log(ArLog::Normal, "motionDebugInfo: Connected to robot.");


  robot.runAsync(true);

  robot.lock();
  robot.com(213); // request absolute max values
  robot.comInt(214, 1); // safety state
  robot.comInt(218, 1); // wheel info
  robot.comInt(149, 1); // heading info
  robot.comInt(124, 1); // torque info
  robot.comInt(123, 1); // accel info
  robot.unlock();

  // wait for the thread to stop
  robot.waitForRunExit();

  // exit
  ArLog::log(ArLog::Normal, "motionDebugInfo: Exiting.");
  Aria::exit(0);
  return 0;
}
