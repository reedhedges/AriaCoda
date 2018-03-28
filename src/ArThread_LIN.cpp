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
// ArThread.cc -- Thread classes


#include <errno.h>
#include <list>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include "ariaOSDef.h"
#include "ArThread.h"
#include "ArLog.h"
#include "ArSignalHandler.h"

#ifndef MINGW
#include <sys/syscall.h>
#endif

static void * run(void *arg)
{
  ArThread *t=(ArThread*)arg;
  void *ret=NULL;

  if (t->getBlockAllSignals())
    ArSignalHandler::blockCommonThisThread();

  if (dynamic_cast<ArRetFunctor<void*>*>(t->getFunc()))
    ret=((ArRetFunctor<void*>*)t->getFunc())->invokeR();
  else
    t->getFunc()->invoke();

  return(ret);
}


/**
   Initializes the internal structures which keep track of what thread is
   what. This is called by Aria::init(), so the user will not normaly need
   to call this function themselves. This funtion *must* be called from the
   main thread of the application. In otherwords, it should be called by
   main().
*/
void ArThread::init()
{
  ArThread *main;
  ThreadType pt;

  pt=pthread_self();

  ourThreadsMutex.setLogName("ArThread::ourThreadsMutex");

  ourThreadsMutex.lock();
  if (ourThreads.size())
  {
    ourThreadsMutex.unlock();
    return;
  }
  main=new ArThread;
  main->myJoinable=true;
  main->myRunning=true;
  main->myThread=pt;
  addThreadToMap(pt, main); // Recursive lock!
  //ourThreads.insert(MapType::value_type(pt, main));
  ourThreadsMutex.unlock();
}


AREXPORT void ArThread::shutdown()
{
  /*** This is the _WIN code.  Something similar (or identical?) should
   *** probably be implemented here.

  ourThreadsMutex.lock();

  // At this point, the ourThreads map should only contain the main thread 
  // that was created in init (presuming that joinAll was called, from 
  // the main thread).
  // 
  // Do not use deleteSetPairs because this causes the ourThreads map 
  // to be updated recursively (because the destructor updates the map).
  //
  std::list<ArThread *> threadList;

  for (MapType::iterator mapIter = ourThreads.begin(); 
       mapIter != ourThreads.end();
       mapIter++) {
    if (mapIter->second != NULL) {
      threadList.push_back(mapIter->second);
    }
  }
  for (std::list<ArThread *>::iterator listIter = threadList.begin();
      listIter != threadList.end();
      listIter++) {
    delete (*listIter);
  }
  if (!ourThreads.empty()) {
    ArLog::log(ArLog::Normal,
               "ArThread::shutdown() unexpected thread leftover");
  }
  ourThreadsMutex.unlock();

  ***/

} // end method shutdown

/**
   If a newly created thread calls self() on itself too soon, this will return
   NULL. This is due to the fact that the thread is first created and started.
   Then the operating system returns the thread ID and thread that called
   create() then updates the list of threads with the new thread ID. There
   is just not much that can be done about that. The use should be aware of
   this caveat.

   Note that this does a lookup on the thread map, so it might be a
   little slow, if you need to use this a lot you should probably use
   osSelf instead.
*/
ArThread * ArThread::self()
{
   return findThreadInMap(pthread_self());
   /*
  ThreadType pt;
  MapType::iterator iter;

  ourThreadsMutex.lock();
  pt=pthread_self();
  iter=ourThreads.find(pt);
  ourThreadsMutex.unlock();

  if (iter != ourThreads.end())
    return((*iter).second);
  else
    return(NULL);
*/
}

/**
   This returns the OS thread, and should be used in place of self if
   its being used a lot.
**/
ArThread::ThreadType ArThread::osSelf()
{
  return pthread_self();
}

void ArThread::cancelAll()
{
  MapType::iterator iter;

  ourThreadsMutex.lock();
  for (iter=ourThreads.begin(); iter != ourThreads.end(); ++iter)
  {
    pthread_cancel((*iter).first);
    (*iter).second->stopRunning();
  }
  ourThreads.clear();
  ourThreadsMutex.unlock();
}

int ArThread::create(ArFunctor *func, bool joinable, bool lowerPriority)
{
  int ret;
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  if (joinable)
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  else
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  myJoinable=joinable;
  myFunc=func;
  myRunning=true;
  if (myBlockAllSignals)
  {
    ArSignalHandler::blockCommonThisThread();
  }
  if ((ret=pthread_create(&myThread, &attr, &run, this)) != 0)
  {
    pthread_attr_destroy(&attr);
    if (ret == EAGAIN)
    {
      ArLog::log(ArLog::Terse, "ArThread::create: Error in create, not enough system resources in pthread_create() (EAGAIN)");
      return(STATUS_NORESOURCE);
    }
    else if(ret == ENOMEM)
    {
      ArLog::log(ArLog::Terse, "ArThread::create: Error in create, not enough system resources in pthread_create() (ENOMEM)");
      return(STATUS_NORESOURCE);
    }
    else
    {
      ArLog::log(ArLog::Terse, "ArThread::create: Unknown error in create.");
      return(STATUS_FAILED);
    }
  }
  else
  {
    if (myName.size() == 0)
    {
      ArLog::log(ourLogLevel, "Created anonymous thread with ID %d", 
		 myThread);
      //ArLog::logBacktrace(ArLog::Normal);
    }
    else
    {
      ArLog::log(ourLogLevel, "Created %s thread with ID %d", myName.c_str(),
		 myThread);
    }
	addThreadToMap(myThread, this);
	/*
    ourThreadsMutex.lock();
    ourThreads.insert(MapType::value_type(myThread, this));
    ourThreadsMutex.unlock();
	*/
    pthread_attr_destroy(&attr);
    return(0);
  }
}

int ArThread::doJoin(void **iret)
{
  int ret;
  if ((ret=pthread_join(myThread, iret)) != 0)
  {
    if (ret == ESRCH)
    {
      ArLog::log(ArLog::Terse, "ArThread::join: Error in join: No such thread found");
      return(STATUS_NO_SUCH_THREAD);
    }
    else if (ret == EINVAL)
    {
      ArLog::log(ArLog::Terse, "ArThread::join: Error in join: Thread is detached or another thread is waiting");
      return(STATUS_INVALID);
    }
    else if (ret == EDEADLK)
    {
      ArLog::log(ArLog::Terse, "ArThread::join: Error in join: Trying to join on self");
      return(STATUS_JOIN_SELF);
    }
  }

  return(0);
}

int ArThread::detach()
{
  int ret;

  if ((ret=pthread_detach(myThread)) != 0)
  {
    if (ret == ESRCH)
    {
      ArLog::log(ArLog::Terse, "ArThread::detach: Error in detach: No such thread found");
      return(STATUS_NO_SUCH_THREAD);
    }
    else if (ret == EINVAL)
    {
      ArLog::log(ArLog::Terse, "ArThread::detach: Error in detach: ArThread is already detached");
      return(STATUS_ALREADY_DETATCHED);
    }
  }

  myJoinable=false;
  return(0);
}

void ArThread::cancel()
{
  removeThreadFromMap(myThread);
  /*
  ourThreadsMutex.lock();
  ourThreads.erase(myThread);
  ourThreadsMutex.unlock();
  */
  pthread_cancel(myThread);
}

void ArThread::yieldProcessor()
{
  sched_yield();
}

AREXPORT void ArThread::threadStarted(void)
{
  myStarted = true;
  myPID = getpid();
  // MPL 12/3/2012 gettid isn't implemented, but there's a hack with syscall
  //myTID = gettid();
#ifdef MINGW
  myTID = -1;
#else
  myTID = (pid_t) syscall(SYS_gettid);
#endif
  if (myName.size() == 0)
    ArLog::log(ourLogLevel, "Anonymous thread (%d) is running with pid %d tid %d",
	       myThread, myPID, myTID);
  else
    ArLog::log(ourLogLevel, "Thread %s (%d) is running with pid %d tid %d",
	       myName.c_str(), myThread, myPID, myTID);
}

AREXPORT void ArThread::threadFinished(void)
{
  myFinished = true;
  /// 12/3/2012 MPL Taking this out since it should be set already
  //myPID = getpid();
  if (myName.size() == 0)
    ArLog::log(ourLogLevel, "Anonymous thread (%d) with pid %d tid %d has finished",
	       myThread, myPID, myTID);
  else
    ArLog::log(ourLogLevel, "Thread %s (%d) with pid %d tid %d has finished",
	       myName.c_str(), myThread, myPID, myTID);
}

AREXPORT void ArThread::logThreadInfo(void)
{
  if (myName.size() == 0)
    ArLog::log(ourLogLevel, "Anonymous thread (%d) is running with pid %d tid %d",
	       myThread, myPID, myTID);
  else
    ArLog::log(ourLogLevel, "Thread %s (%d) is running with pid %d %d",
	       myName.c_str(), myThread, myPID, myTID);
}
