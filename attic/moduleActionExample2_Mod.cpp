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


/** @example moduleExample_Mod2.cpp 
 * @brief Example demonstrating how to implement a module with ArModule
 *
 * This is a second module loaded by moduleExample.cpp.
  @sa moduleExample.cpp.
  @sa ArModule in the reference manual.
*/


class SimpleMod : public ArModule
{
public:

  bool init(ArRobot *robot, void *argument = NULL);
  bool exit();
  
  ArActionLimiterForwards myLimiterForwards;
  
};

SimpleMod module2;
ARDEF_MODULE(module2);

bool SimpleMod::init(ArRobot *robot, void *argument)
{
  ArLog::log(ArLog::Normal, "moduleActionExample2_Mod: Robot name is %s", robot->getRobotName());
  ArLog::log(ArLog::Terse, "moduleActionExample2_Mod: init(%p) called in moduleActionExample2_Mod!", robot);
  if (argument != NULL)
    ArLog::log(ArLog::Terse, "moduleActionExample2_Mod: Argument given to ArModuleLoader::load was the string '%s'.", 
	   (char *)argument);
  else
    ArLog::log(ArLog::Terse, "moduleActionExample2_Mod: No argument was given to ArModuleLoader (this is OK).");
    
  // Do stuff here...
  
  robot->addAction(&myLimiterForwards, 40);
  myLimiterForwards.activate();
  robot->clearDirectMotion();
  robot->enableMotors();

  return(true);
}

bool SimpleMod::exit()
{
  ArLog::log(ArLog::Terse, "moduleActionExample2_Mod: exit() called.");

  // Do stuff here...

  return(true);
}
