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

bool log_simstat = true;

unsigned int simint, realint, lastint;
int realX, realY, realTh, realZ;
double lat, lon, alt;
double errorx, errory, errorth;

bool handleSimStatPacket(ArRobotPacket* pkt)
{
  if(pkt->getID() != 0x62) return false;
  if(log_simstat) printf("----------- SIMSTAT pkt received: ------------\n");
  char a = pkt->bufToByte();  // unused byte
  char b = pkt->bufToByte();  // unused byte
  ArTypes::UByte4 flags = pkt->bufToUByte4();
  if(log_simstat) printf("\tFlags=0x%x  (Georef? %s, OdomError? %s)\n", flags, (flags&ArUtil::BIT1)?"yes":"no", (flags&ArUtil::BIT2)?"yes":"no");
  simint = pkt->bufToUByte2();
  realint = pkt->bufToUByte2();
  lastint = pkt->bufToUByte2();
  if(log_simstat) printf("\tSimInterval=%d, RealInterval=%d, LastInterval=%d.\n", simint, realint, lastint);
  realX = pkt->bufToByte4();
  realY = pkt->bufToByte4();
  realZ = pkt->bufToByte4();
  realTh = pkt->bufToByte4();
  if(log_simstat) printf("\tTrue Pose = (%d, %d, %d, %d)\n", realX, realY, realZ, realTh);
  lat = pkt->bufToByte4() / 10e6;
  lon = pkt->bufToByte4() / 10e6;
  alt = pkt->bufToByte4() / 100.0;
  if(log_simstat) printf("\tGeo Pose = (%f, %f, %f)\n", lat, lon, alt);
  errorx = pkt->bufToByte4() / 10e6;
  errory = pkt->bufToByte4()  / 10e6;
  errorth = pkt->bufToByte4() / 10e6;
  if(log_simstat) printf("\tOdom Error = (%f, %f, %f)\n", errorx, errory, errorth);
  if(log_simstat) printf("-------------------------------------------------\n");
  return true;
}

bool handleSimDeviceData(ArRobotPacket* pkt)
{
  if(pkt->getID() != 100) return false;
  printf("------ SIM DEVICE DATA packet received: ---------\n");
  char name[24];
  char type[24];
  int idx;
  pkt->bufToStr(name, 23);
  pkt->bufToStr(type, 23);
  idx = pkt->bufToUByte2();
  printf("Data for %s which is #%d of type %s.\n", name, idx, type);
  if(strcmp(type, "gps") == 0)
  {
    double lat, lon, dop;
    lat = (double)pkt->bufToByte4() / (double)10e6;
    lon = (double)pkt->bufToByte4() / (double)10e6;
    dop = (double)pkt->bufToByte2() / 1000.0;
    printf("GPS Lat=%f, Lon=%f, DOP=%f\n", lat, lon, dop);
  }
  printf("-------------------------------------------------\n");
  return true;
}

ArCondition gotSimMapChangedPacketCondition;
bool handleSimMapChangedPacket(ArRobotPacket* pkt)
{
  if(pkt->getID() != 102) return false;
  printf("------- SIM MAP CHANGED packet received: --------\n");
  char name[256];
  int user = pkt->bufToUByte();
  int really_loaded = pkt->bufToUByte();
  if(user == 1)
    printf("user=%d: Loaded by user GUI\n", user);
  else
  {
    printf("user=%d: Loaded by client program\n", user);
    if(really_loaded)
      printf("loaded=%d: A new map was loaded.\n", really_loaded);
    else
      printf("loaded=%d: Map already loaded, not reloaded.\n", really_loaded);
  }
  pkt->bufToStr(name, 256);
  printf("filename=%s\n", name);
  printf("-------------------------------------------------\n");
  gotSimMapChangedPacketCondition.signal();
  return true;
}
  

int main(int argc, char **argv)
{
  Aria::init();
  ArLog::init(ArLog::StdErr, ArLog::Normal);
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
    printf("Could not connect to simulated robot... exiting\n");
    return 2;
  }
  printf("Connected to robot.\n");
  robot.runAsync(true);

  ArGlobalRetFunctor1<bool, ArRobotPacket*> mySimDeviceDataPacketHandler(&handleSimDeviceData);
  robot.addPacketHandler(&mySimDeviceDataPacketHandler);

  ArGlobalRetFunctor1<bool, ArRobotPacket*> mySimStatPacketHandler(&handleSimStatPacket);
  robot.addPacketHandler(&mySimStatPacketHandler);
  printf("-> SIM_STAT 2...\n");
  robot.comInt(ArCommands::SIM_STAT, 2);

  robot.waitForRunExit();

  Aria::exit(0);
  return 0;
}

