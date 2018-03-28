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
#ifndef ARSTRINGINFOGROUP_H
#define ARSTRINGINFOGROUP_H

#include "ariaUtil.h"
#include "ArMutex.h"
#include <string>
#include <set>
#include <list>

/**
   This class takes callbacks from different classes that want this
   string information and then lets you just add the information here
   instead of to each individual class.

  @ingroup OptionalClasses
 **/
class ArStringInfoGroup
{
public:
  /// Constructor
  AREXPORT ArStringInfoGroup();
  /// Destructor
  AREXPORT virtual ~ArStringInfoGroup();
  /// Adds a string to the list in the raw format
  AREXPORT bool addString(const char *name, ArTypes::UByte2 maxLen, 
			  ArFunctor2<char *, ArTypes::UByte2> *functor);

  /// Adds an int to the list in the helped way
  /// @param navalue if value is >= this value, then display "N/A" instead
  AREXPORT bool addStringInt(const char *name, ArTypes::UByte2 maxLen, 
			     ArRetFunctor<int> *functor, 
			     const char *format = "%d", 
           int navalue = INT_MAX);

  /// Adds a double to the list in the helped way
  AREXPORT bool addStringDouble(const char *name, ArTypes::UByte2 maxLen, 
				ArRetFunctor<double> *functor, 
				const char *format = "%g");

  /// Adds a bool to the list in the helped way
  AREXPORT bool addStringBool(const char *name, ArTypes::UByte2 maxLen, 
			      ArRetFunctor<bool> *functor,
			      const char *format = "%s");

  /// Adds a string to the list in the helped way
  AREXPORT bool addStringString(const char *name, ArTypes::UByte2 maxLen, 
			      ArRetFunctor<const char *> *functor,
			      const char *format = "%s");

  /// Adds a std::string to the list. std::string::c_str() will be used to  
  AREXPORT bool addStringString(const char *name, ArTypes::UByte2 maxLen,
            ArRetFunctor<std::string> *functor);

  /// Adds an int to the list in the helped way
  /// @param navalue if value is >= this value, then display "N/A" instead
  AREXPORT bool addStringUnsignedLong(const char *name, 
				      ArTypes::UByte2 maxLen, 
				      ArRetFunctor<unsigned long> *functor, 
				      const char *format = "%lu",
              unsigned long navalue = ULONG_MAX);

  /// Adds an int to the list in the helped way
  /// @param navalue if value is >= this value, then display "N/A" instead
  AREXPORT bool addStringLong(const char *name, 
			      ArTypes::UByte2 maxLen, 
			      ArRetFunctor<long> *functor, 
			      const char *format = "%ld",
            long navalue = LONG_MAX);

  AREXPORT bool addStringTime(const char *name,
            ArTypes::UByte2 maxLen,
            ArRetFunctor<ArTime> *functor,
            const char *format = "%llu:%llu");

  AREXPORT bool addStringFloat(const char *name, ArTypes::UByte2 maxLen, 
				ArRetFunctor<float> *functor, 
				const char *format = "%g");

  /// This is the function to add a callback to be called by addString
  AREXPORT void addAddStringCallback(
	  ArFunctor3<const char *, ArTypes::UByte2,
	  ArFunctor2<char *, ArTypes::UByte2> *> *functor,
	  ArListPos::Pos position = ArListPos::LAST);

protected:
  ArMutex myDataMutex;
  std::set<std::string, ArStrCaseCmpOp> myAddedStrings;
  std::list<ArFunctor3<const char *, ArTypes::UByte2,
	      ArFunctor2<char *, ArTypes::UByte2> *> *> myAddStringCBList;
};


#endif // ARSTRINGINFOHELPER_H
