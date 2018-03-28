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

/** @example mtxWheelLights.cpp Shows possible MTX wheel lamp states */

int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;

  ArRobotConnector robotConnector(&parser, &robot);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "mtxWheelLights: Could not connect to the robot.");
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
  ArLog::log(ArLog::Normal, "mtxWheelLights: Connected.");
  robot.runAsync(true);

  struct {
    ArTypes::UByte pattern;
    ArTypes::Byte value;
    ArTypes::UByte flags;
    ArTypes::UByte flags2;
  } lamp;

  lamp.value = 0;
  lamp.flags = 0;
  lamp.flags2 = 0;

  const int sleeptime = 3000;

  for(lamp.pattern = 1; lamp.pattern <= 10; ++(lamp.pattern))
  {
    ArLog::log(ArLog::Normal, "mtxWheelLights: pattern %d...", lamp.pattern);
    robot.comDataN(ArCommands::WHEEL_LIGHT, (const char*)&lamp, 4);
    ArUtil::sleep(sleeptime);

    if(lamp.pattern == 6)
    {
      for(lamp.value = 0; lamp.value <=  100; lamp.value += 10)
      {
        ArLog::log(ArLog::Normal, "mtxWheelLights: pattern %d with value %d...", lamp.pattern, lamp.value);
        robot.comDataN(ArCommands::WHEEL_LIGHT, (const char*)&lamp, 4);
        ArUtil::sleep(sleeptime);
      }
      lamp.value = 0;
    }
    else
    {
	lamp.value = 50;
	ArLog::log(ArLog::Normal, "mtxWheelLights: patterd %d with value 50...", lamp.pattern);
	robot.comDataN(ArCommands::WHEEL_LIGHT, (const char*)&lamp, 4);
        ArUtil::sleep(sleeptime);
        lamp.value = 0;
    }


    ArLog::log(ArLog::Normal, "mtxWheelLights: pattern %d with warning flag...", lamp.pattern);
    lamp.flags |= ArUtil::BIT1;
    robot.comDataN(ArCommands::WHEEL_LIGHT, (const char*)&lamp, 4);
    ArUtil::sleep(sleeptime);
    lamp.flags = 0;
  }

  ArLog::log(ArLog::Normal, "mtxWheelLights: Exiting.");
  Aria::exit(0);
  return 0;
}
