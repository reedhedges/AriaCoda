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

/** @example seekurPower.cpp Control Seekur or Seekur Jr. accessory power
 * switching from command line options.   Run with -help for help on command
 * line options. See robot manuals and notes on http://robots.mobilerobots.com
 * for more information.
 */

typedef struct {
  const char *option;
  int port;
  const char *description;
  int set;
} powerspec;

powerspec PowerSpecs[] = {
  {"pc2", 1,  "Turn onboard computer #2 on or off.", 0},
  {"pc3", 13,  "Turn onboard computer #3 on or off.", 0},
  {"pc4", 14,  "Turn onboard computer #4 on or off", 0},
  {"pc5", 15,  "Turn onboard computer #5 on or off.", 0},
  {"lrf1", 9,  "Turn laser rangefinder #1 on or off.", 0},
  {"lrf2", 24,  "Turn laser rangefinder #2 on or off.", 0},
  {"lrf1heat", 11,  "Turn laser rangefinder #1's heater on or off.", 0},
  {"lrf2heat", 27,  "Turn laser rangefinder #2's heater on or off.", 0},
  {"gps", 6, "Turn GPS receeiver on or off.", 0},
  {"poe", 4, "Turn 12V Power over Ethernet port on or off.", 0},
  {"lan", 5, "Turn internal LAN ethernet switch on or off.", 0},
  {"camera-raw", 12, "Turn the 24V RVision camera on or off.", 0},
  {"rvision-raw", 12, "Turn the 24V RVision camera on or off.", 0},
  {"camera", 10, "Turn the 24V RVision camera on or off.", 0},
  {"rvision", 10, "Turn the 24V RVision camera on or off.", 0},
  {"ptu1", 9, "Turn pan/tilt unit #1 on or off.", 0},
  {"ptu2", 22, "Turn pan/tilt unit #2 on or off.", 0},
  {"ptu3", 23, "Turn pan/tilt unit #3 on or off.", 0},
  {"arm", 29, "Turn the raw 24V manipulator arm drive power on or off.", 0},
  {"armpower", 29, "Turn the raw 24V manipulator arm drive power on or off.", 0},
  {"armlogic", 8, "Turn the regulated manipulator arm controller/logic power on or off.", 0},
  {"armcam", 7, "Turn the arm camera or other 12V arm accessories on or off.", 0}
};

int main(int argc, char **argv)
{
  int numPowerSpecs = sizeof(PowerSpecs)/sizeof(powerspec);
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;
  ArRobotConnector robotConnector(&parser, &robot);

  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "seekurPower: Error: Could not connect to the robot.");
    Aria::logOptions();
    Aria::exit(1);
  }

  if (!Aria::parseArgs() || parser.checkArgument("help"))
  {
    Aria::logOptions();
    ArLog::log(ArLog::Terse, "Options for seekurPower command (your robot may not have some of these devices):");
    ArLog::log(ArLog::Terse, "-<n> <on|off|reset>\t\tTurn port <n> on or off, or reset by turning off then on again. Refer to robot documentation or http://robots.mobilerobots.com/wiki/Seekur_Switched_Power_Outputs for list and notes.");
    for(int i = 0; i < numPowerSpecs; ++i)
      ArLog::log(ArLog::Terse, "-%s <on|off|reset>\t\t%s (port %d)", PowerSpecs[i].option, PowerSpecs[i].description, PowerSpecs[i].port);
    ArLog::log(ArLog::Terse, "-robotOff\t\tTurn whole robot off");
    Aria::exit(2);
  }

  if(parser.checkArgument("-robotOff"))
  {
    ArLog::log(ArLog::Terse, "-robotOff given -- powering down entire robot with command #119 in 5 seconds...");
    for(int i = 5; i > 0; --i)
    {
      ArLog::log(ArLog::Terse, "Shutting down in %d seconds...", i);
      ArUtil::sleep(1000);
    }
    robot.com(119);
    Aria::exit(0);
  }
      

  std::list<powerspec> todo;

  int nargs = parser.getArgc();
  for(int argi = 1; argi < nargs; ++argi)
  {
    const char *opt = parser.getArg(argi);
    if(opt[0] != '-')
    {
      ArLog::log(ArLog::Terse, "seekurPower: Error: invalid option %s. Use -help for list of options.", opt);
      Aria::exit(4);
    }
    ++opt;
    if(opt[0] == '-')
      ++opt;
    bool found = false;
    if(argi == nargs-1) // option given as last argument, with no value
    {
      ArLog::log(ArLog::Terse, "seekurPower: Error: Missing argument to last option %s. Specify on, off or reset.", opt);
      Aria::exit(7);
    }
    const char *val = parser.getArg(++argi);
    for(int pi = 0; pi < numPowerSpecs; ++pi)
    {
      powerspec ps = PowerSpecs[pi];
      if(strcmp(opt, ps.option) == 0) 
      {
        found = true;
        if(strcmp(val, "on") == 0)
          ps.set = 1;
        else if(strcmp(val, "off") == 0)
          ps.set = 0;
        else if(strcmp(val, "reset") == 0)
          ps.set = 2;
        else
        {
          ArLog::log(ArLog::Terse, "seekurPower: Error: Invalid value '%s' for option %s. Use on, off or reset.", val, opt);
          Aria::exit(6);
        }
        todo.push_back(ps);
        break;
      }
    }
    if(!found)
    {
      if(ArUtil::isOnlyNumeric(opt))
      {
        found = true;
        powerspec ps;
        ps.option = opt;
        ps.description = NULL;
        ps.port = atoi(opt);
        if(strcmp(val, "on") == 0)
          ps.set = 1;
        else if(strcmp(val, "off") == 0)
          ps.set = 0;
        else if(strcmp(val, "reset") == 0)
          ps.set = 2;
        todo.push_back(ps);
      }
    }
    if(!found)
    {
      ArLog::log(ArLog::Terse, "seekurPower: Warning: unrecognized option %s. Use -help for list of options.", opt);
    }
  }


  for(std::list<powerspec>::const_iterator i = todo.begin(); i != todo.end(); ++i)
  {
    powerspec ps = *i;
    if(ps.set == 2)
    {
      ArLog::log(ArLog::Normal, "seekurPower: Switching %s (port %d) off, waiting 2 seconds., then switching it back on again...", ps.option, ps.port);
      robot.com2Bytes(116, ps.port, 0);
      ArUtil::sleep(2000);
      robot.com2Bytes(116, ps.port, 1);
    }
    else
    {
      ArLog::log(ArLog::Normal, "seekurPower: Switching %s (port %d) %s...", ps.option, ps.port, ps.set?"on":"off");
      robot.com2Bytes(116, ps.port, ps.set);
    }
  }
  

  Aria::exit(0);
}

