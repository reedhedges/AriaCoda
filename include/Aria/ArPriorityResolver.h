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
#ifndef ARPRIORITYRESOLVER_H
#define ARPRIORITYRESOLVER_H

#include "Aria/ArResolver.h"

/// (Default resolver), takes the action list and uses the priority to resolve
/** 
    This is the default resolver for ArRobot, meaning if you don't do a 
    non-normal init on the robot, or a setResolver, you'll have one these.
*/
class ArPriorityResolver final : public ArResolver
{
public:
  /// Constructor
  AREXPORT ArPriorityResolver();
  //AREXPORT virtual ~ArPriorityResolver();
  AREXPORT virtual ArActionDesired *resolve(ArResolver::ActionMap *actions,
					    ArRobot *robot,
					    bool logActions = false) override;
protected:
  ArActionDesired myActionDesired;
};

#endif // ARPRIORITYRESOLVER_H
