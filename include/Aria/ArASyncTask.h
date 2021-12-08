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

#ifndef ARASYNCTASK_H
#define ARASYNCTASK_H


#include "Aria/ariaTypedefs.h"
#include "Aria/ArFunctor.h"
#include "Aria/ArThread.h"


/// Asynchronous task (runs in its own thread)
/**
   The ArAsynTask is a task that runs in its own thread. This is a
   rather simple class. The user simply needs to derive their own
   class from ArAsyncTask and define the runThread() function. They
   then need to create an instance of their task and call run or
   runAsync. The standard way to stop a task is to call stopRunning()
   which sets ArThread::myRunning to false. In their run loop, they
   should pay attention to the getRunning() or the ArThread::myRunning
   variable. If this value goes to false, the task should clean up
   after itself and exit its runThread() function.  

    @ingroup UtilityClasses
**/
class ArASyncTask : public ArThread
{
public:

  /// Constructor
  AREXPORT ArASyncTask();

  /// Destructor. Override if neccesary in your subclass.
  virtual ~ArASyncTask() = default;

  // todo implement?
  ArASyncTask(const ArASyncTask& other) = delete;
  ArASyncTask(ArASyncTask&& old) = delete;
  ArASyncTask& operator=(const ArASyncTask& other) = delete;
  ArASyncTask& operator=(ArASyncTask&& other) = delete;

  /// The main run loop
  /**
     Override this function and put your tasks run loop here. Check the
     value of getRunning() periodically in your loop. If the value
     is false, the loop should exit and runThread() should return.
     The argument and return value are specific to the platform thread implementation, and 
     can be ignored.
     @swignote In the wrapper libraries, this method takes no arguments and has no return value.
  */
  AREXPORT virtual void * runThread(void *arg) = 0;

  /// Run without creating a new thread
/**
   This will run the the ArASyncTask without creating a new
   thread to run it in. It performs the needed setup then calls runThread() 
   directly instead of letting the system threading system do it in a new thread.
*/
  virtual void run() { runInThisThread(); }
  
  /// Run in its own thread
  virtual void runAsync() { create(); }

  // re-implemented here just so its easier to see in the docs
  /// Stop the thread
  virtual void stopRunning() override {myRunning=false;}

  /// Create the task and start it going
  AREXPORT int create(bool joinable=true, bool lowerPriority=true);

  /** Internal function used with system threading system to run the new thread.
      In general, use run() or runAsync() instead.
      @internal
  */
  AREXPORT void * runInThisThread(void *arg=0);

private:

  // Hide regular Thread::Create
  virtual int create(ArFunctor * /*func*/, bool /*joinable=true*/,
		     bool /*lowerPriority=true*/) override {return(false);}


  ArRetFunctor1C<void*, ArASyncTask, void*> myFunc;
};


#endif // ARASYNCTASK_H
