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
// Recurrent task class
#ifndef WIN32
#include <pthread.h>
#include <unistd.h>
#endif
#include "ariaOSDef.h"
#include "ArASyncTask.h"
#include "ArLog.h"
#include "ArRecurrentTask.h"

//
// Async recurrent tasks
//
// This class must be subclassed with the particular Task that will 
//   be run
//

// constructor: start up thread, leave it ready for go()

AREXPORT 
ArRecurrentTask::ArRecurrentTask()
{
  setThreadName("ArRecurrentTask");
  running = go_req = killed = false;
  create();			// create the thread
}


AREXPORT 
ArRecurrentTask::~ArRecurrentTask()
{
  kill();
}

// Entry to the thread's main process
// Here we check if a Go request has been made, and
//   if so, we run Task()
// When done, set running to false, and wait for
//   the next request

AREXPORT void *
ArRecurrentTask::runThread(void *ptr) 
{
  threadStarted();
#ifndef WIN32
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
#endif
  while (myRunning)
    {
      bool doit;

      while (myRunning)
	{
	  lock();
	  doit = go_req;
	  unlock();
	  if (doit)
	    break;
//	  yield();		// don't hog resources
#ifndef WIN32
	  usleep(10000);
#else
	  Sleep(10);
#endif
	}
      if (!myRunning)
	break;
      lock();
      go_req = false;
      running = true;		// we've been requested to go
      unlock();
      task();			// do what we've got to do...
      lock();
      running = false;		// say we're done
      unlock();
    }

  threadFinished();
  return NULL;
}

AREXPORT void ArRecurrentTask::go()
{
  lock();
  go_req = true;
  running = true;
  killed = false;
  unlock();
}

AREXPORT int ArRecurrentTask::done()
{
  lock();
  bool is_running = running;
  bool is_killed = killed;
  unlock();
  if (is_running) return 0;
  if (is_killed) return 2;	// we didn't complete, were killed
  else return 1;
}

AREXPORT void ArRecurrentTask::reset()
{
  lock();
  go_req = false;
  if (running)			// async task is going, kill and restart
    {
      killed = true;
      running = false;
      unlock();
      cancel();
      create();
    }
  else
    unlock();
}

AREXPORT void ArRecurrentTask::kill()
{
  lock();
  go_req = false;
  killed = true;
  running = false;
  unlock();
  cancel();
}
