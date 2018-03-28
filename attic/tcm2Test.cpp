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
#include <time.h>

ArRobot *robot;

bool tcm2Printer(ArRobotPacket *packet)
{
  double myCompass;
  double myPitch;
  double myRoll;
  double myXMag;
  double myYMag;
  double myZMag;
  double myTemperature;
  int myError;
  char myCalibrationH;
  char myCalibrationV;
  double myCalibrationM;
  //printf("%d\n", packet->getID());
  if (packet->getID() != 0xC0)
    return false;
  
  myCompass = packet->bufToByte2() / 10.0;
  myPitch = packet->bufToByte2() / 10.0;
  myRoll = packet->bufToByte2() / 10.0;
  myXMag = packet->bufToByte2() / 100.0;  
  myYMag = packet->bufToByte2() / 100.0;
  myZMag = packet->bufToByte2() / 100.0;
  myTemperature = packet->bufToByte2() / 10.0;
  myError = packet->bufToByte2();
  myCalibrationH = packet->bufToByte();
  myCalibrationV = packet->bufToByte();
  myCalibrationM = packet->bufToByte2() / 100.0;
  printf("\r%6.1f %6.1f %6.1f %6.2f %6.2f %6.2f %6.1f 0x%08x %4d %4d %6.2f", myCompass, myPitch, myRoll, myXMag, myYMag, myZMag, myTemperature, myError, myCalibrationH, myCalibrationV, myCalibrationM);
  fflush(stdout);
  //, myError, myCalibrationH, myCalibrationV, myCalibration
  return true;
}
  

int main(int argc, char **argv) 
{
  std::string str;
  int ret;
  
  ArGlobalRetFunctor1<bool, ArRobotPacket *> tcm2PrinterCB(&tcm2Printer);
  ArSerialConnection con;
  Aria::init();
  
  robot = new ArRobot;

  robot->addPacketHandler(&tcm2PrinterCB, ArListPos::FIRST);
  if ((ret = con.open()) != 0)
  {
    str = con.getOpenMessage(ret);
    printf("Open failed: %s\n", str.c_str());
    exit(0);
  }

  robot->setDeviceConnection(&con);
  if (!robot->blockingConnect())
  {
    printf("Could not connect to robot... exiting\n");
    exit(0);
  }
  
  printf("%6s %6s %6s %6s %6s %6s %6s %10s %4s %4s %6s\n", 
	 "comp", "pitch", "roll", "magX", "magY", "magZ", "temp", "error",
	 "calH", "calV", "calM");
	 robot->comInt(ArCommands::TCM2, 3);

  robot->run(true);
  Aria::shutdown();
  
}

