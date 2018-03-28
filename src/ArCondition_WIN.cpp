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
#include "ArExport.h"
#include "ariaOSDef.h"
#include "ArCondition.h"
#include "ArLog.h"


ArStrMap ArCondition::ourStrMap;


AREXPORT ArCondition::ArCondition() :
  myFailedInit(false),
  myCond(),
  myCount(0)
{
  myCond=CreateEvent(0, FALSE, FALSE, 0);
  if (myCond == NULL)
  {
	DWORD err = GetLastError(); // see http://msdn.microsoft.com/en-us/library/windows/desktop/ms681381(v=vs.85).aspx
    ArLog::log(ArLog::Terse, "ArCondition(%s)::ArCondition: Error %d trying to create the condition.", getLogName(), err);
    myFailedInit=true;
  }

  ourStrMap[STATUS_FAILED]="General failure";
  ourStrMap[STATUS_FAILED_DESTROY]=
  "Another thread is waiting on this condition so it can not be destroyed";
  ourStrMap[STATUS_FAILED_INIT] =
  "Failed to initialize thread. Requested action is imposesible";
  ourStrMap[STATUS_MUTEX_FAILED_INIT]="The underlying mutex failed to init";
  ourStrMap[STATUS_MUTEX_FAILED]="The underlying mutex failed in some fashion";
}

AREXPORT ArCondition::~ArCondition()
{
  if (!myFailedInit && !CloseHandle(myCond))
    ArLog::log(ArLog::Terse, "ArCondition(%s)::~ArCondition: Unknown error while trying to destroy the condition.", getLogName());
}

AREXPORT int ArCondition::signal()
{
  if (myFailedInit)
  {
    ArLog::log(ArLog::Terse, "ArCondition(%s)::signal: Initialization of condition failed, failed to signal", getLogName());
    return(STATUS_FAILED_INIT);
  }

  if (!PulseEvent(myCond))
  {
    ArLog::log(ArLog::Terse, "ArCondition(%s)::signal: Unknown error while trying to signal the condition.", getLogName());
    return(STATUS_FAILED);
  }

  return(0);
}

AREXPORT int ArCondition::broadcast()
{
  int ret=0;

  if (myFailedInit)
  {
    ArLog::log(ArLog::Terse, "ArCondition(%s)::broadcast: Initialization of condition failed, failed to broadcast", getLogName());
    return(STATUS_FAILED_INIT);
  }

  //ArLog::log(ArLog::Normal, "broadcasting %s", getLogName());
  for (; myCount != 0; --myCount)
  {
    if (PulseEvent(myCond) == 0) // PulseEvent returns 09 on error according to http://msdn.microsoft.com/en-us/library/windows/desktop/ms684914(v=vs.85).aspx
    {
	  DWORD err = GetLastError(); // see http://msdn.microsoft.com/en-us/library/windows/desktop/ms681381(v=vs.85).aspx
      ArLog::log(ArLog::Terse, "ArCondition(%s)::broadcast: Error %d while trying to broadcast the condition.", getLogName(), err);
      ret=STATUS_FAILED;
    }
  }

  return(ret);
}

AREXPORT int ArCondition::wait()
{
  DWORD ret;

  if (myFailedInit)
  {
    ArLog::log(ArLog::Terse, "ArCondition(%s)::wait: Initialization of condition failed, failed to wait", getLogName());
    return(STATUS_FAILED_INIT);
  }

  ++myCount;
  ret=WaitForSingleObject(myCond, INFINITE);
  if (ret == WAIT_OBJECT_0)
    return(0);
  else
  {
    ArLog::logNoLock(ArLog::Terse, "ArCondition(%s)::wait: Failed to lock due to an unknown error", getLogName());
    return(STATUS_FAILED);
  }
}

AREXPORT int ArCondition::timedWait(unsigned int msecs)
{
  int ret;

  if (myFailedInit)
  {
    ArLog::log(ArLog::Terse, "ArCondition(%s)::wait: Initialization of condition failed, failed to wait", getLogName());
    return(STATUS_FAILED_INIT);
  }

  ++myCount;
  ret=WaitForSingleObject(myCond, msecs);
  if (ret == WAIT_OBJECT_0)
    return(0);
  else if (ret == WAIT_TIMEOUT)
    return(STATUS_WAIT_TIMEDOUT);
  else
  {
    ArLog::logNoLock(ArLog::Terse, "ArCondition(%s)::timedWait: Failed to lock due to an unknown error", getLogName());
    return(STATUS_FAILED);
  }
}

AREXPORT const char *ArCondition::getError(int messageNumber) const
{
  ArStrMap::const_iterator it;
  if ((it = ourStrMap.find(messageNumber)) != ourStrMap.end())
    return (*it).second.c_str();
  else
    return NULL;
}
