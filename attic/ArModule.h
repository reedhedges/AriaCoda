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
#ifndef ARMODULE_H
#define ARMODULE_H


#include "ariaTypedefs.h"
#include "ArRobot.h"


/// Dynamicly loaded module base class, read warning in more
/**
   Right now only one module's init will be called, that is the first
   one, its a bug that I just don't have time to fix at the moment.
   I'll get to it when I have time or if someone needs it... someone
   else wrote this code so it'll take me a little longer to fix it.

   This class defines a dyanicmly loaded module of code. This is usefull
   for an application to load piece of code that it does not know about.
   The ArModule defines and interface in which to invoke that piece of code
   that the program does not know about. For instance, a module could
   contain an ArAction and the modules init() could instantiate the ArAction
   and add it to the supplied ArRobot. The init() takes a reference to an
   ArRobot. The module should use that robot for its purposes. If the module
   wants to use more robots, assuming there are multiple robots, it can use
   Aria::getRobotList() to find all the ArRobot instantiated. The module
   should do all its clean up in exit().

   The user should derive their own class from ArModule and implement the
   init() and exit() functions. The users code should always clean up
   when exit() is called. exit() is called right before the module (dynamic
   library .dll/.so) is closed and removed from the program.

   The macro ARDEF_MODULE() must be called within the .cpp file of the users
   module. A global instance of the users module must be defined and a
   reference to that instance must be passed to ARDEF_MODULE(). This allows
   the ArModuleLoader to find the users module class and invoke it.

   One thing to note about the use of code wrapped in ArModules and staticly
   linking in that code. To be able to staticly link .cpp files which contain
   an ArModule, the ARIA_STATIC preprocessor symbol should be defined. This will cause
   the ARDEF_MODULE() to do nothing. If it defined its normal functions and
   variables, the linker would fail to staticly link in multiple modules
   since they all have symbols with the same name.

   Refer to ArModuleLoader to see how to load an ArModule into a program.

   For examples, see the programs advanced/simpleMod.cpp and advanced/simpleModule.cpp. 
*/
class ArModule
{
public:

  /// Constructor
  AREXPORT ArModule();
  /// Destructor
  AREXPORT virtual ~ArModule();

  /// Initialize the module. The module should use the supplied ArRobot pointer
  /**
     @param robot Robot this module should attach to or operate on. Note, may be NULL.

     @param argument an optional argument passed to the module. Default
     is NULL.  The module can interpret this in any way, it is recommended
     that you make sure this is documented if used. 
  */
  AREXPORT virtual bool init(ArRobot *robot, 
			     void *argument = NULL) = 0;

  /// Close down the module and have it exit
  AREXPORT virtual bool exit() = 0;

  /// Get the ArRobot pointer the module should be using
  AREXPORT ArRobot * getRobot() {return(myRobot);}

  /// Set the ArRobot pointer
  AREXPORT void setRobot(ArRobot *robot) {myRobot=robot;}

protected:

  /// Stored ArRobot pointer that the module should use
  ArRobot *myRobot;
};


/*
  Beware ye all who pass beyond this point. Ugly macros abound and you might
  get eaten by a gru if your light fails.  */


#ifdef ARIA_STATIC

#define ARDEF_MODULE(mod)

#else

#ifdef WIN32

#define ARDEF_MODULE(mod) \
extern "C" {\
static ArModule *__AriaModule_##mod = &mod; \
_declspec(dllexport) bool \
ariaInitModule(ArRobot *robot, void *argument = NULL) \
{ \
  if (__AriaModule_##mod) \
  { \
    __AriaModule__->setRobot(robot); \
    return(__AriaModule_##mod->init(robot, argument)); \
  } \
  else \
    return(false); \
} \
_declspec(dllexport) bool ariaExitModule() \
{ \
  if (__AriaModule_##mod) \
    return(__AriaModule_##mod->exit()); \
  return(false); \
} \
}
#else // WIN32

#define ARDEF_MODULE(mod) \
static ArModule *__AriaModule_##mod = &mod; \
extern "C" {\
bool ariaInitModule(ArRobot *robot, void *argument = NULL) \
{ \
  if (__AriaModule_##mod) \
  { \
    __AriaModule_##mod->setRobot(robot); \
    return(__AriaModule_##mod->init(robot, argument)); \
  } \
  else \
    return(false); \
} \
bool ariaExitModule() \
{ \
  if (__AriaModule_##mod) \
    return(__AriaModule_##mod->exit()); \
  return(false); \
} \
}

#endif // WIN32

#endif // ARIA_STATIC


#endif // ARMODULE_H
