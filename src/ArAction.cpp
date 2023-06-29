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
#include "Aria/ArExport.h"
#include "Aria/ariaOSDef.h"
#include "Aria/ArResolver.h"
#include "Aria/ArAction.h"
#include "Aria/ArLog.h"
#include "Aria/ArRobot.h"
#include <string.h>

AREXPORT bool ArAction::ourDefaultActivationState = true;

AREXPORT ArAction::ArAction(const char *name, const char *description)
{
  myRobot = NULL;
  myNumArgs = 0;
  myName = name;
  myDescription = description;
  myIsActive = ourDefaultActivationState;
}

AREXPORT ArAction::~ArAction()
{
  if (myRobot != NULL)
    myRobot->remAction(this);
}

AREXPORT const char *ArAction::getName() const
{
  return myName.c_str();
}

AREXPORT const char *ArAction::getDescription() const
{
  return myDescription.c_str();
}

AREXPORT int ArAction::getNumArgs() const
{
  return myNumArgs;
}

AREXPORT void ArAction::setNextArgument(ArArg const &arg)
{
  myArgumentMap[myNumArgs] = arg;
  myNumArgs++;
}

AREXPORT ArArg *ArAction::getArg(int number) 
{
  std::map<int, ArArg>::iterator it;
  
  it = myArgumentMap.find(number);
  if (it != myArgumentMap.end())
    return &(*it).second;
  else
    return NULL;
}

AREXPORT const ArArg *ArAction::getArg(int number) const
{
  std::map<int, ArArg>::const_iterator it;
  
  it = myArgumentMap.find(number);
  if (it != myArgumentMap.end())
    return &(*it).second;
  else
    return NULL;
}

/**
 *  @swignote If you override this method in a Java or Python subclass, use
 *  setActionRobotObj(ArRobot) instead of trying to call super.setRobot() or 
 *  ArAction.setRobot(). (SWIG's subclassing "directors" feature cannot properly
 *  direct the call to the parent class, an infinite recursion results instead.)
 */
AREXPORT void ArAction::setRobot(ArRobot *robot)
{
  myRobot = robot;
}

AREXPORT bool ArAction::isActive() const
{
  return myIsActive;
}

AREXPORT void ArAction::activate()
{
  myIsActive = true;
}

AREXPORT void ArAction::deactivate()
{
  myIsActive = false;
}

AREXPORT void ArAction::log(bool verbose) const
{
  ArLog::log(ArLog::Terse, "Action %s isActive %d", getName(), myIsActive);
  const ArActionDesired *desired;
  if (myIsActive && (desired = getDesired()) != NULL)
    desired->log();
  if (!verbose)
    return;
  if (strlen(getDescription()) != 0)
    ArLog::log(ArLog::Terse, "Action %s is described as: %s", getName(), getDescription());
  else
    ArLog::log(ArLog::Terse, "Action %s has no description.", getName());
  if (getNumArgs() == 0)
    ArLog::log(ArLog::Terse, "Action %s has no arguments.\n", getName());
  else
  {
    ArLog::log(ArLog::Terse, "Action %s has %d arguments, of type(s):", getName(), getNumArgs());

    for (int i = 0; i < getNumArgs(); i++) 
    {
      const ArArg *arg = getArg(i);
      if (arg == NULL) continue;
      arg->log();
    }
    ArLog::log(ArLog::Terse, "");
  }
}



