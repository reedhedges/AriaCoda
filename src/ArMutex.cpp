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
#include "ArMutex.h"
#include "ariaOSDef.h"
#include "ariaUtil.h"
#include "ArThread.h"
#include <stdio.h>
#include <stdarg.h>


unsigned int ArMutex::ourLockWarningMS = 0;
unsigned int ArMutex::ourUnlockWarningMS = 0;
ArFunctor *ArMutex::ourNonRecursiveDeadlockFunctor = NULL;


AREXPORT void ArMutex::setLogNameVar(const char *logName, ...)
{
  char arg[2048];
  va_list ptr;
  va_start(ptr, logName);
  vsnprintf(arg, sizeof(arg), logName, ptr);
  arg[sizeof(arg) - 1] = '\0';
  va_end(ptr);
  return setLogName(arg);
}

void ArMutex::initLockTiming()
{
  myFirstLock = true;
  myLockTime = new ArTime;
  myLockStarted = new ArTime;
}

void ArMutex::uninitLockTiming()
{
  delete myLockTime;
  myLockTime = 0;
  delete myLockStarted;
  myLockStarted = 0;
}


void ArMutex::startLockTimer() 
{
  if(ourLockWarningMS > 0)
    myLockStarted->setToNow();
}

void ArMutex::checkLockTime() 
{
	//printf("ourLockWarningMS=%d, myLockStarted->mSecSince=%d\n", ourLockWarningMS, myLockStarted->mSecSince());
  if (ourLockWarningMS > 0 && myLockStarted &&
        myLockStarted->mSecSince() >= ourLockWarningMS)
    ArLog::logNoLock(
   	  ArLog::Normal, 
   	  "LockWarning: locking '%s' from thread '%s' %d pid %d took %.3f sec", 
      myLogName.c_str(), ArThread::getThisThreadName(),
   	  ArThread::getThisThread(), 
#ifdef WIN32
	  0,
#else
	  getpid(), 
#endif
   	  myLockStarted->mSecSince() / 1000.0);

}

void ArMutex::startUnlockTimer() 
{
  if (ourUnlockWarningMS > 0)
  {
    myLockTime->setToNow();
    myFirstLock = false;
  }
}

void ArMutex::checkUnlockTime() {
	//printf("checking unlock time: warningms=%d, myFirstLock=%d, msecSince=%d\n", ourUnlockWarningMS, myFirstLock, myLockTime->mSecSince());
  if (ourUnlockWarningMS > 0 && !myFirstLock &&  myLockTime &&
        myLockTime->mSecSince() >= ourUnlockWarningMS)
    ArLog::logNoLock(ArLog::Normal, 
		    "LockWarning: unlocking '%s' from thread ('%s' %d pid %d) was locked for %.3f sec", 
		    myLogName.c_str(), 
		    ArThread::getThisThreadName(), 
		    ArThread::getThisThread(), 
#ifdef WIN32
			0,
#else
			getpid(),
#endif
		    myLockTime->mSecSince() / 1000.0);
}
