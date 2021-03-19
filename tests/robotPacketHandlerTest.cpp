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
#include "Aria/Aria.h"


/* Connects to the robot or sim and logs all packets recieved.
 * Adds a packet handler (as the first one) which logs the packet
 * data but allows subsequent packet handlers to have the packet as well.
 */

bool testcb(ArRobotPacket* pkt)
{
  pkt->log();
  return false;
}

int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser argParser(&argc, argv);
  argParser.loadDefaultArguments();
  ArRobot robot;

  ArRobotConnector robotConnector(&argParser, &robot);

  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Normal, "robotPacketHandlerTest: Could not connect to robot.");
    if(argParser.checkHelpAndWarnUnparsed())
    {
      Aria::logOptions();
      Aria::exit(2);
    }
    Aria::exit(1);
  }

  if(!Aria::parseArgs() || !argParser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(2);
  }


  ArLog::log(ArLog::Normal, "robotPacketHandlerTest: Connected.");

  robot.addPacketHandler(new ArGlobalRetFunctor1<bool, ArRobotPacket*>(&testcb), ArListPos::FIRST);

  robot.run(true);

  return 0;
}
