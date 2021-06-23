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
#include "Aria/ArStringInfoGroup.h"

/* 
   TODO the stringInt functions and such'll leak memory on a failed attempt 
*/

AREXPORT ArStringInfoGroup::ArStringInfoGroup()
{
  myDataMutex.setLogName("ArStringInfoGroup::myDataMutex");
}

/* AREXPORT ArStringInfoGroup::~ArStringInfoGroup()
{

} */

/**
  @note It is recommended that you avoid characters in a string name or value
  which may have special meaning in contexts like ArConfig files, such as 
  '#', ';', tabs and newlines.
*/
AREXPORT bool ArStringInfoGroup::addString(
	const char *name, size_t maxLength,
	ArFunctor2<char *, size_t> *functor)
{
  myDataMutex.lock();
  if (myAddedStrings.find(name) != myAddedStrings.end())
  {
    ArLog::log(ArLog::Normal, "ArStringInfoGroups: Cannot add info '%s', duplicate", name);
    myDataMutex.unlock();
    return false;
  }

  std::list<ArFunctor3<const char *, size_t,
  ArFunctor2<char *, size_t> *> *>::iterator it;
  ArLog::log(ArLog::Verbose, "ArStringInfoGroups: Adding info '%s'", name);
  myAddedStrings.insert(name);
  for (it = myAddStringCBList.begin(); it != myAddStringCBList.end(); it++)
  {
    (*it)->invoke(name, maxLength, functor);
  }
  ArLog::log(ArLog::Verbose, "ArStringInfoGroups: Added info '%s'", name);
  myDataMutex.unlock();
  return true;
}

AREXPORT bool ArStringInfoGroup::addStringInt(
	const char *name, size_t maxLength,
	ArRetFunctor<int> *functor, const char *format,
  int navalue)
{
  return addString(name, maxLength, 
	    (new ArGlobalFunctor5<char *, size_t, 
	     ArRetFunctor<int> *, 
	     const char *, int>(&ArStringInfoHolderFunctions::intWrapper, 
			   (char *)NULL, (size_t) 0, 
			   functor, format, navalue)));
}

AREXPORT bool ArStringInfoGroup::addStringDouble(
	const char *name, size_t maxLength,
	ArRetFunctor<double> *functor, const char *format)
{
  return addString(name, maxLength, 
	    (new ArGlobalFunctor4<char *, size_t, 
	     ArRetFunctor<double> *, 
	     const char *>(&ArStringInfoHolderFunctions::doubleWrapper, 
			   (char *)NULL, (size_t) 0, 
			   functor, format)));
}

AREXPORT bool ArStringInfoGroup::addStringFloat(
	const char *name, size_t maxLength,
	ArRetFunctor<float> *functor, const char *format)
{
  return addString(name, maxLength, 
	    (new ArGlobalFunctor4<char *, size_t, 
	     ArRetFunctor<float> *, 
	     const char *>(&ArStringInfoHolderFunctions::floatWrapper, 
			   (char *)NULL, (size_t) 0, 
			   functor, format)));
}


AREXPORT bool ArStringInfoGroup::addStringBool(
	const char *name, size_t maxLength,
	ArRetFunctor<bool> *functor, const char *format)
{
  return addString(name, maxLength, 
	    (new ArGlobalFunctor4<char *, size_t, 
	     ArRetFunctor<bool> *, 
	     const char *>(&ArStringInfoHolderFunctions::boolWrapper, 
			   (char *)NULL, (size_t) 0, 
			   functor, format)));
}

// for a functor returning a C string (const char*)
AREXPORT bool ArStringInfoGroup::addStringString(
	const char *name, size_t maxLength,
	ArRetFunctor<const char *> *functor, const char *format)
{
  return addString(name, maxLength, 
	    (new ArGlobalFunctor4<char *, size_t, 
	     ArRetFunctor<const char *> *, 
	     const char *>(&ArStringInfoHolderFunctions::stringWrapper, 
			   (char *)NULL, (size_t) 0, 
			   functor, format)));
}

// for a functor returning a C++ string (std::string)
AREXPORT bool ArStringInfoGroup::addStringString(
	const char *name, size_t maxLength,
	ArRetFunctor<std::string> *functor)
{
  return addString(name, maxLength, 
	    (new ArGlobalFunctor3<char *, size_t, ArRetFunctor<std::string>*>(
         &ArStringInfoHolderFunctions::cppStringWrapper, 
			   (char *)NULL, (size_t) 0, 
			   functor)
      )
  );
}

AREXPORT bool ArStringInfoGroup::addStringUnsignedLong(
	const char *name, size_t maxLength,
	ArRetFunctor<unsigned long> *functor, const char *format,
  unsigned long navalue)
{
  return addString(name, maxLength, 
	    (new ArGlobalFunctor5<char *, size_t, 
	     ArRetFunctor<unsigned long> *, 
	     const char *, unsigned long>(&ArStringInfoHolderFunctions::unsignedLongWrapper, 
			   (char *)NULL, (size_t) 0, 
			   functor, format, navalue)));
}

AREXPORT bool ArStringInfoGroup::addStringLong(
	const char *name, size_t maxLength,
	ArRetFunctor<long> *functor, const char *format,
  long navalue)
{
  return addString(name, maxLength, 
	    (new ArGlobalFunctor5<char *, size_t, ArRetFunctor<long> *, const char *, long>(
         &ArStringInfoHolderFunctions::longWrapper, 
			   (char *)NULL, (size_t) 0, 
			   functor, format, navalue)
      )
   );
}

/** @a format must include two %llu fields (unsigned long long), to accept
       the results of ArTime::getSecLL() and ArTime::getMSecLL().
*/
AREXPORT bool ArStringInfoGroup::addStringTime(
	const char *name, size_t maxLength,
	ArRetFunctor<ArTime> *functor, const char *format)
{
  return addString(name, maxLength, 
	    (new ArGlobalFunctor4<char *, size_t, 
	     ArRetFunctor<ArTime> *, 
	     const char *>(&ArStringInfoHolderFunctions::arTimeWrapper, 
			   (char *)NULL, (size_t) 0, 
			   functor, format)));
}


AREXPORT void ArStringInfoGroup::addAddStringCallback(
	ArFunctor3<const char *, size_t,
	ArFunctor2<char *, size_t> *> *functor,
	ArListPos::Pos position)
{
  if (position == ArListPos::FIRST)
    myAddStringCBList.push_front(functor);
  else if (position == ArListPos::LAST)
    myAddStringCBList.push_back(functor);
  else
    ArLog::log(ArLog::Terse, 
	       "ArStringInfoGroup::addAddStringCallback: Invalid position.");
}
