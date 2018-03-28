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


bool gotConfigPacket = false;
ArCondition gotConfigPacketCondition;
bool handleConfigPacket(ArRobotPacket* pkt)
{
  if(pkt->getID() != 0x20) return false;
  if(gotConfigPacket) return true;
  printf("----------- CONFIG pkt received: ------------\n");
  char buf[256];
  pkt->bufToStr(buf, sizeof(buf));
  printf("Type=%s\n", buf);
  pkt->bufToStr(buf, sizeof(buf));
  printf("Subtype=%s\n", buf);
  pkt->bufToStr(buf, sizeof(buf));
  printf("SerialNumber=%s\n", buf);
  printf("unknown=%d\n", pkt->bufToUByte());
  printf("RotVelTop=%d\n", pkt->bufToUByte2());
  printf("TransVelTop=%d\n", pkt->bufToUByte2());
  printf("RotAccelTop=%d\n", pkt->bufToUByte2());
  printf("TransAccelTop=%d\n", pkt->bufToUByte2());
  printf("PWMMax=%d\n", pkt->bufToUByte2());
  pkt->bufToStr(buf, sizeof(buf));
  printf("Name=%s\n", buf);
  printf("SipCycleTime=%d\n", pkt->bufToUByte());
  printf("HostBaud=%d\n", pkt->bufToUByte());
  printf("Aux1Baud=%d\n", pkt->bufToUByte());
  printf("HasGripper=%d\n", pkt->bufToUByte2());
  printf("HasFrontSonar=%d\n", pkt->bufToUByte2());
  printf("HasRearSonar=%d\n", pkt->bufToUByte());
  printf("LowBattery=%d\n", pkt->bufToUByte2());
  printf("RevCount=%d\n", pkt->bufToUByte2());
  printf("Watchdog=%d\n", pkt->bufToUByte2());
  printf("NormalMPacs=%d\n", pkt->bufToUByte());
  printf("StallVal=0x%X\n", pkt->bufToUByte2());
  printf("StallCount=%d\n", pkt->bufToUByte2());
  printf("JoyVel=%d\n", pkt->bufToUByte2());
  printf("JoyRotVel=%d\n", pkt->bufToUByte2());
  printf("RotVelMax=%d\n", pkt->bufToUByte2());
  printf("TransVelMax=%d\n", pkt->bufToUByte2());
  printf("RotAccel=%d\n", pkt->bufToUByte2());
  printf("RotDecel=%d\n", pkt->bufToUByte2());
  printf("RotKP=%d\n", pkt->bufToUByte2());
  printf("RotKV=%d\n", pkt->bufToUByte2());
  printf("RotKI=%d\n", pkt->bufToUByte2());
  printf("TransAccel=%d\n", pkt->bufToUByte2());
  printf("TransDecel=%d\n", pkt->bufToUByte2());
  printf("TransKP=%d\n", pkt->bufToUByte2());
  printf("TransKV=%d\n", pkt->bufToUByte2());
  printf("TransKI=%d\n", pkt->bufToUByte2());
  printf("FrontBumps=%d\n", pkt->bufToUByte());
  printf("RearBumps=%d\n", pkt->bufToUByte());
  printf("HasCharger=%d\n", pkt->bufToUByte());
  printf("SonarCycle=%d\n", pkt->bufToUByte());
  printf("ResetBaud=%d\n", pkt->bufToUByte());
  printf("HasGyro=%d\n", pkt->bufToUByte());
  printf("DriftFactor=%d\n", pkt->bufToUByte2());
  printf("Aux2Baud=%d\n", pkt->bufToUByte());
  printf("Aux3Baud=%d\n", pkt->bufToUByte());
  printf("TicksPerMM=%d\n", pkt->bufToUByte2());
  printf("ShutdownVoltage=%d\n", pkt->bufToUByte2());
  pkt->bufToStr(buf, sizeof(buf));
  printf("FirmwareVersion=%s\n", buf);
  printf("ChargeThreshold=%d\n", pkt->bufToUByte2());
  gotConfigPacket = true;
  gotConfigPacketCondition.signal();
  return true;

}

int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  ArSimpleConnector connector(&parser);
  ArRobot robot;

  if (!connector.parseArgs())
  {
    connector.logOptions();
    return 1;
  }


  if (!connector.connectRobot(&robot))
  {
    printf("Could not connect to robot... exiting\n");
    return 2;
  }
  printf("Connected to robot.\n");
  robot.runAsync(true);

  ArGlobalRetFunctor1<bool, ArRobotPacket*> myConfigPacketHandler(&handleConfigPacket);
  robot.addPacketHandler(&myConfigPacketHandler);
  puts("-> CONFIG...");
  robot.com(18);
  gotConfigPacketCondition.wait();
  Aria::exit(0);
}
