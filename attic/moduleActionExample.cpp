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


/** @example moduleActionExample.cpp Example demonstrating how to load an ArModule
 *
  This is a simple program that loads the modules moduleActionExample_Mod and
moduleActionExample_Mod2, which are
  defined in moduleActionExample_Mod.cpp and moduleActionExample_Mod2.cpp. These modules
  create and add some ArActions to the robot, but your module(s) may do
anything.
  This
  program simply calls ArModuleLoader::load() with a short message
  argument and ArModuleLoader::reload with no argument and finally
  calls ArModuleLoader::close(). The return status of the load(),
  reload(), and close() are checked and printed out.
  ArModuleLoader uses the name of the module file without the platform-specific
  suffix (i.e. ".dll" on Windows and ".so" on Linux) to load the module.

  @sa moduleActionExample_Mod.cpp.
  @sa moduleActionExample_Mod2.cpp.
  @sa ArModuleLoader in the reference manual.  
*/


void printStatus(ArModuleLoader::Status status)
{
  if (status == ArModuleLoader::STATUS_ALREADY_LOADED)
    ArLog::log(ArLog::Terse, "moduleActionExample: Module already loaded.");
  else if (status == ArModuleLoader::STATUS_FAILED_OPEN)
    ArLog::log(ArLog::Terse, "moduleActionExample: Failed to find or open the simpleMod module.");
  else if (status == ArModuleLoader::STATUS_INVALID)
    ArLog::log(ArLog::Terse, "moduleActionExample: Invalid file.");
  else if (status == ArModuleLoader::STATUS_INIT_FAILED)
    ArLog::log(ArLog::Terse, "moduleActionExample: Module Init failed.");
  else if (status == ArModuleLoader::STATUS_SUCCESS)
    ArLog::log(ArLog::Terse, "moduleActionExample: Module succedded.");
  else if (status == ArModuleLoader::STATUS_EXIT_FAILED)
    ArLog::log(ArLog::Terse, "moduleActionExample: Module exit sequence failed.");
  else if (status == ArModuleLoader::STATUS_NOT_FOUND)
    ArLog::log(ArLog::Terse, "moduleActionExample: Module not found.");
  else
    ArLog::log(ArLog::Terse, "moduleActionExample: Module returned unknown status!");
  ArLog::log(ArLog::Terse, "");
}

int main(int argc, char **argv)
{

  Aria::init();

  ArArgumentParser parser(&argc, argv);
  // set up our simple connector
  ArSimpleConnector simpleConnector(&parser);  
  ArRobot robot;

  // set up the robot for connecting
  if (!simpleConnector.connectRobot(&robot))
  {
    printf("Could not connect to robot... exiting\n");
    Aria::exit(1);
  }

  robot.runAsync(true);

  ArModuleLoader::Status status;
  std::string str;

  ArLog::log(ArLog::Terse, "moduleActionExample: Loading the module \"moduleActionExample_Mod\" with a string argument...");
  status=ArModuleLoader::load("./moduleActionExample_Mod", &robot, (char *)"You've loaded a module!");
  printStatus(status);

  ArLog::log(ArLog::Terse, "moduleActionExample: Loading the module \"moduleActionExample_Mod2\" with a string argument...");
  status=ArModuleLoader::load("./moduleActionExample2_Mod", &robot, (char *)"You've loaded a second module!");
  printStatus(status);

  //ArLog::log(ArLog::Terse, "moduleActionExample: Reloading \"moduleActionExample_Mod\" with no argument...");
  //status=ArModuleLoader::reload("./moduleActionExample_Mod", &robot);
  //printStatus(status);

  //ArLog::log(ArLog::Terse, "moduleActionExample: Closing  \"moduleActionExample_Mod\"...");
  //status=ArModuleLoader::close("./moduleActionExample_Mod");
  //printStatus(status);

  ArUtil::sleep(3000);

  Aria::exit(0);
  return(0);
}
