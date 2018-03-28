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
#ifndef ARCONFIGGROUP_H
#define ARCONFIGGROUP_H

#include "ArConfig.h"

/// Container for holding a group of ArConfigs
class ArConfigGroup
{
public:
  /// Constructor
  AREXPORT ArConfigGroup(const char *baseDirectory = NULL);
  /// Destructor
  AREXPORT ~ArConfigGroup(void);
  /// Adds a config to the group
  AREXPORT void addConfig(ArConfig *config);
  /// Removes a config from the group
  AREXPORT void remConfig(ArConfig *config);
  /// Parses the given file (starting from the base directory)
  AREXPORT bool parseFile(const char *fileName, bool continueOnError = false);
  /// Reloads the last file parsed
  AREXPORT bool reloadFile(bool continueOnError = true);
  /// Writes a file out (overwrites any existing file)
  AREXPORT bool writeFile(const char *fileName);
  /// Sets the base directory on all configs this contains
  AREXPORT void setBaseDirectory(const char *baseDirectory);
  /// Gets the baes directory of this group (not the configs it contains)
  AREXPORT const char *getBaseDirectory(void) const;
protected:
  std::string myBaseDirectory;
  std::string myLastFile;
  std::list<ArConfig *> myConfigs;
};

#endif // ARCONFIGGROUP
