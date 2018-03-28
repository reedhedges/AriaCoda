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

void task(ArRobotJoyHandler *j)
{
  if(j->gotData())
  {
    double x, y, z;
    j->getDoubles(&x, &y, &z);
    ArLog::log(ArLog::Normal, "x: %.3f, y: %.3f, z:%.3f, button1: %d, button2: %d",
      x, y, z, j->getButton1(), j->getButton2());
  }
  else
    ArLog::log(ArLog::Normal, "no joystick data.");
}

bool joystickPacket(ArRobotPacket *packet)
{
  if (packet->getID() != 0xF8)
    return false;
  ArLog::log(ArLog::Normal, "logRobotJoystick: Got a joystick packet.");
  return false; // false allows other handlers to get this packet, if this handler was inserted before them.
}
  

int main(int argc, char **argv)
{
  Aria::init();
  ArLog::init(ArLog::StdErr, ArLog::Normal);
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;



   // Use this to request joystick data and store it in an ArRobotJoyHandler object, and print that data every task cycle.
   ArRobotJoyHandler j(&robot);
   robot.addUserTask("joylog", 5, new ArGlobalFunctor1<ArRobotJoyHandler*>(&task, &j));

  // Use this to log whether we got the packet or not
  robot.addPacketHandler(new ArGlobalRetFunctor1<bool, ArRobotPacket*>(&joystickPacket), ArListPos::FIRST);
//  robot.comInt(ArCommands::JOYINFO, 2);

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  ArRobotConnector robotConnector(&parser, &robot);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "Could not connect to the robot.");
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

  robot.run(true);

  Aria::exit(0);
}
