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
#ifndef ARHASFILENAME_H
#define ARHASFILENAME_H

#include "ariaTypedefs.h"

/// Interface to access an object's associated file name.
/**
 * ArHasFileName provides a single abstract method which should be overridden
 * to return the complete file path name of the associated file.  It is 
 * implemented by classes that have external persistent storage, such as 
 * ArConfig and ArMap.
 * 
 * Copyright (c) Adept Technology, Inc. All rights reserved.
**/
class ArHasFileName
{
public:

  /// Constructor
	AREXPORT ArHasFileName() 
  {}
	
  /// Copy constructor
  ArHasFileName(ArHasFileName const &) 
  {}

	/// Assignment operator.
  ArHasFileName &operator=(ArHasFileName const & )
  {
    return *this;
  }

  /// Destructor
	AREXPORT virtual ~ArHasFileName()
  {}

  /// Returns the complete file path name of the associated file
  AREXPORT virtual const char *getFileName() const  = 0;

}; // end class


#endif // ARHASFILENAME_H

