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

bool encoderPrinter(ArRobotPacket *packet)
{
  long int left;
  long int right;
  printf("encoderTest received packet 0x%X %s\n", packet->getID(), 
    (packet->getID() == 0x90 ? "[ENCODERpac]" : 
      ( (packet->getID() == 0x32 || packet->getID() == 0x33) ? "[SIP]" : "" ) 
    ) 
  );
  if (packet->getID() != 0x90)
    return false;
  left = packet->bufToByte4();
  right = packet->bufToByte4();
  printf("### %ld %ld\n", left, right);
  return true;
}
  

int main(int argc, char **argv) 
{
  Aria::init();

  std::string str;
  int ret;
  
  ArGlobalRetFunctor1<bool, ArRobotPacket *> encoderPrinterCB(&encoderPrinter);

  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  robot = new ArRobot;
  ArRobotConnector robotConnector(&parser, robot);

  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "encoderTest: Could not connect to the robot.");
    if(parser.checkHelpAndWarnUnparsed())
    {
        // -help not given
        Aria::logOptions();
        Aria::exit(1);
    }
  }
  

  robot->addPacketHandler(&encoderPrinterCB, ArListPos::FIRST);

  robot->requestEncoderPackets();

  robot->run(true);
  Aria::shutdown();
  
}

