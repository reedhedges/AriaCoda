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
#ifndef ARRESOLVER_H
#define ARRESOLVER_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ArActionDesired.h"
#include <string>

class ArAction;
class ArRobot;

/// Resolves a list of actions and returns what to do
/**
  ArResolver::resolve() is the function that ArRobot
  calls with the action list in order
  to produce a combined ArActionDesired object from them, according to
  the subclass's particular algorithm or policy.

  @todo is this missing assignment, and copy/move ctors?
*/
class ArResolver
{
public:

  typedef std::multimap<int, ArAction *> ActionMap;
  ArResolver(const char *name, const char * description = "")
    { myName = name; myDescription = description; }

  virtual ~ArResolver() = default;
  /// Figure out a single ArActionDesired from a list of ArAction s
  virtual ArActionDesired *resolve(ActionMap *actions, ArRobot *robot,
				   bool logActions = false) = 0;
  /// Gets the name of the resolver
  virtual const char *getName() const { return myName.c_str(); }
  /// Gets the long description fo the resolver
  virtual const char *getDescription() const { return myDescription.c_str(); }
  
protected:
  std::string myName;
  std::string myDescription;
};

#endif
