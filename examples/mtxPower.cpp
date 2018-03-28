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
#include "ArMTXIO.h"

typedef struct {
  const char *option;
  int bank;
  int bit;
  const char *description;
  int set;
} powerspec;

// all bank and bit values are 0-indexed.
powerspec PowerSpecs[] = {
  {"aux5v", 3-1, 1,  "5V AUX POWER pin #4", 0},
  {"aux12v", 3-1, 2,  "12V AUX POWER pin #5", 0},
  {"aux20v", 3-1, 3,  "20V AUX POWER pin #6", 0},
  {"user1", 2-1, 5,  "Raw 24V USER POWER 1: pin #7", 0},
  {"user2", 2-1, 6,  "Raw 24V USER POWER 2: pin #8", 0},
  {"user3", 2-1, 7,  "Raw 24V USER POWER 3 and 4: pin #9, #10, #11, #12", 0},
  {"wheellights", 1-1, 6,  "Wheel Lights", 0},
  {"lrf", 1-1, 3,  "Primary Laser", 0},
  {"audio", 1-1, 4, "Audio Amplifier", 0},
  {"sonar1", 1-1, 7, "Sonar1", 0},
  {"sonar2", 1-1, 8, "Sonar2", 0},
  {"vertlaser", 2-1, 3, "Aux Laser 1 (vert) (aux sensors pins 5 and 10)", 0},
  {"footlaser", 2-1, 4, "Aux Laser 2 (foot) (aux sensors pin 15)", 0},
  {"usb12", 2-1, 1, "USB 1 and 2 and aux sensors pin 9", 0},
  {"usb3", 2-1, 2, "USB 3 and aux sensors pin 14", 0},
  {"hmipower", 1-1, 5, "HMI Panel Power", 0}
};


int main(int argc, char **argv)
{
  int numPowerSpecs = sizeof(PowerSpecs)/sizeof(powerspec);
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();

/*
  ArRobot robot;
  ArRobotConnector robotConnector(&parser, &robot);

  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "mtxPower: Error: Could not connect to the robot.");
    Aria::logOptions();
    Aria::exit(1);
  }
  robot.runAsync();
*/

  if (!Aria::parseArgs() || parser.checkArgument("help"))
  {
    Aria::logOptions();
    ArLog::log(ArLog::Terse, "Options for mtxPower command (your robot may not have some of these devices):");
    for(int i = 0; i < numPowerSpecs; ++i)
      ArLog::log(ArLog::Terse, "-%s <on|off|reset>\t\t%s (%d:%d)", PowerSpecs[i].option, PowerSpecs[i].description, PowerSpecs[i].bank, PowerSpecs[i].bit);
    Aria::exit(2);
  }


  ArMTXIO io;
  if(!io.isEnabled())
  {
    ArLog::log(ArLog::Terse, "mtxPower: Error opening MTX IO device interface. Is driver loaded and device file created?");
    Aria::exit(4);
  }

  std::list<powerspec> todo;

  int nargs = parser.getArgc();
  for(int argi = 1; argi < nargs; ++argi)
  {
    const char *opt = parser.getArg(argi);
    if(opt[0] != '-')
    {
      ArLog::log(ArLog::Terse, "mtxPower: Error: invalid option %s. Use -help for list of options.", opt);
      Aria::exit(4);
    }
    ++opt;
    if(opt[0] == '-')
      ++opt;
    bool found = false;
    if(argi == nargs-1) // option given as last argument, with no value
    {
      ArLog::log(ArLog::Terse, "mtxPower: Error: Missing argument to last option %s. Specify on, off or reset.", opt);
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
          ArLog::log(ArLog::Terse, "mtxPower: Error: Invalid value '%s' for option %s. Use on, off or reset.", val, opt);
          Aria::exit(6);
        }
        todo.push_back(ps);
        break;
      }
    }
    if(!found)
    {
      ArLog::log(ArLog::Terse, "mtxPower: Warning: unrecognized option %s. Use -help for list of options.", opt);
    }
  }


  for(std::list<powerspec>::const_iterator i = todo.begin(); i != todo.end(); ++i)
  {
    powerspec ps = *i;
    if(ps.set == 2)
    {
      ArLog::log(ArLog::Normal, "mtxPower: Switching %s (%d:%d) off, waiting 2 seconds., then switching it back on again...", ps.option, ps.bank, ps.bit);
      io.lock();
      io.setPowerOutput(ps.bank, ps.bit, false);
      io.unlock();
      ArUtil::sleep(2000);
      io.lock();
      io.setPowerOutput(ps.bank, ps.bit, true);
      io.unlock();
    }
    else
    {
      ArLog::log(ArLog::Normal, "mtxPower: Switching %s (%d:%d) %s...", ps.option, ps.bank, ps.bit,  ps.set?"on":"off");
      io.lock();
      io.setPowerOutput(ps.bank, ps.bit, ps.set);
      io.unlock();
    }
  }
  

  Aria::exit(0);
}

