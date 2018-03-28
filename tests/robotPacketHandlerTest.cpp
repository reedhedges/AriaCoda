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
  ArRobot robot;
  Aria::init();
  ArSimpleConnector connector(&argc, argv);
  if (!Aria::parseArgs())
  {
    Aria::logOptions();
    Aria::shutdown();
    return 1;
  }
  
  if (!connector.connectRobot(&robot))
  {
    ArLog::log(ArLog::Normal, "robotPacketHandlerTest: Could not connect to robot... exiting");
    return 2;
  }

  ArLog::log(ArLog::Normal, "robotPacketHandlerTest: Connected.");

  robot.addPacketHandler(new ArGlobalRetFunctor1<bool, ArRobotPacket*>(&testcb), ArListPos::FIRST);

  robot.run(true);

  return 0;
}
