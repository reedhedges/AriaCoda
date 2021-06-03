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
#include "Aria/ariaTypedefs.h"

#include "Aria/ariaOSDef.h"
#include "Aria/ArASyncTask.h"
#include "Aria/ArLog.h"

#if !defined(_WIN32) || defined(MINGW)
// NOTE on MinGW pthread.h must be included after ariaTypedefs.h which includes winsock2.h.
// (on MinGW, pthread.h will include winsock.h to get an errno definition, after which
// winsock2.h will assert a preprocessor error.)
#include <pthread.h>
#endif

AREXPORT ArASyncTask::ArASyncTask() :
  myFunc(this, &ArASyncTask::runThread, NULL)
{
}

/* AREXPORT ArASyncTask::~ArASyncTask()
{
} */

AREXPORT int ArASyncTask::create(bool joinable, bool lowerPriority)
{
  return(ArThread::create(&myFunc, joinable, lowerPriority));
}

/**
   This will run the code of the ArASyncTask without creating a new
   thread to run it in. It performs the needed setup then calls runThread().
   This is good if you have a task which you wish to run multiple
   instances of and you want to use the main() thread  instead of having
   it block, waiting for exit of the program.
   @param arg the argument to pass to the runThread()
*/
AREXPORT void * ArASyncTask::runInThisThread(void *arg)
{
  myJoinable=true;
  myRunning=true;
#if defined(_WIN32) && !defined(MINGW)
  myThread=GetCurrentThreadId();
#else
  myThread=pthread_self();
#endif
  
  if (myName.size() == 0)
  {
    ArLog::log(ourLogLevel, "Running anonymous thread with ID %d", 
	       myThread);
    //ArLog::logBacktrace(ArLog::Normal);
  }
  else
  {
    ArLog::log(ourLogLevel, "Running %s thread", myName.c_str());
  }
  
  addThreadToMap(myThread, this);
  /*
  ourThreadsMutex.lock();
  // MPL BUGFIX, this wasn't workign for some reason (was printing
  // 0)...  so I got rid of it and did it the easier way anyhow
  //printf("!!!! %d\n", ourThreads.insert(MapType::value_type(myThread, this)).second);
  ourThreads[myThread] = this;
  ourThreadsMutex.unlock();
  */

  return(runThread(arg));
}
