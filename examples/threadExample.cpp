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

/** @example threadExample.cpp Example showing ARIA's cross-platform threading tools.
 *
 *  ARIA provides some tools for writing multithreaded programs. These are
 *  abstractions of the threading library provided by the native operating
 *  system.
 *
 *  The three main tools are: 
 *  <ul>
 *    <li>ArASyncTask a class which can run a method in a new thread</li>
 *    <li>ArMutex, an object that can be shared by multiple threads to provide
 *        mutual exclusion from other shared data.</li>
 *    <li>ArCondition, an object that can cause a thread to block
 *        execution until signaled by another thread to continue.</li>
 *  </ul>
 *
 *  This example program shows the use of all three, with two threads
 *  interacting: the program's main thread of execution, and a new thread
 *  created using ArASyncTask.  An ArMutex object is used to keep use of
 *  some shared data safe, and then the use of ArCondition is shown.
 *
 *  Threading can be error-prone, since any (perhaps subconcious) assumptions
 *  you have about the linear execution of code may not apply to simultaneous threads.
 *  Furthermore, different computers will execute multithreaded code in
 *  different ways (especially if they have different numbers of CPUs).
 *  ARIA's threading tools can help make multiple threads work, and help make
 *  multithreaded code portable, but you must always think carefully about
 *  how code might execute (including error conditions!) to avoid deadlocks and
 *  race conditions.  
 */

#include "Aria.h"


/* A subclass of ArASyncTask, to contain a method that runs in a new thread */
class ExampleThread : public ArASyncTask
{
  ArCondition myCondition;
  ArMutex myMutex;
  int myCounter;
public:

  /* Construtor. Initialize counter. */
  ExampleThread() : myCounter(0)
  {
    myCondition.setLogName("ExampleThreadCondition");
  }


  /* This method is called in the new thread when launched. The void* parameter
   * and return value are platform implementation-specific and can be ignored.
   * This method will run in a loop, incrementing the counter each second, but 
   * locking the mutex to prevent conflicting access by other threads. 
   * If it reaches a value divisible by ten, signal our condition variable.
   */
  void* runThread(void*) 
  {
    // Run until the thread is requested to end by another thread.
    while(this->getRunningWithLock())
    {
      myMutex.lock();

      // Increment the counter. 
      myCounter++;
      ArLog::log(ArLog::Normal, "Example thread: incremented counter to %d.", myCounter);

      // If it's now divisible by 10, signal the condition variable.
      if(myCounter % 10 == 0)
      {
        ArLog::log(ArLog::Normal, "Example thread: Signalling condition.");
        myCondition.signal();
      }

      // Unlock, then sleep.  We unlock before the sleep, so that while
      // we are sleeping, other threads that try to lock the mutex won't
      // be blocked until this thread is done sleeping.
      myMutex.unlock();
      ArUtil::sleep(1000);
    }

    ArLog::log(ArLog::Normal, "Example thread: requested stop running, ending thread.");
    return NULL;
  }

  /* Other threads can call this to wait for a condition eventually
   * signalled by this thread. (So note that in this example program, this 
   * function is not executed within "Example thread", but is executed in the main thread.)
   */
  void waitOnCondition()
  {
    myCondition.wait();
    ArLog::log(ArLog::Normal, " %s ArCondition object was signalled, done waiting for it.", myCondition.getLogName()); 
  }

  /* Get the counter. Not threadsafe, you must lock the mutex during access. */
  int getCounter() { return myCounter; }

  /* Set the countner. Not threadsafe, you must lock the mutex during access. */
  void setCounter(int ctr) { myCounter = ctr; }

  /* Lock the mutex object.  */
  void lockMutex() { myMutex.lock(); }

  /* Unlock the mutex object. */
  void unlockMutex() { myMutex.unlock(); }

};

int main()
{
  Aria::init();

  ExampleThread exampleThread;

  /* Launch the new thread in the background. This thread (i.e. the main program thread, 
   * executing main()) continues immediately after the new thread is created. */
  ArLog::log(ArLog::Normal, "Main thread: Running new example thread ...");
  exampleThread.runAsync();

  /* Loop, reading the value contained in the ExampleThread object.
   * We will also use ArUtil::sleep() to make this thread sleep each iteration,
   * instead of running as fast as possible and potentially preventing other
   * threads from access to the mutex and the shared counter.
   * When the counter reaches 10, break out of the loop and then wait on the
   * condition variable.
   */
  while(true)
  {
    exampleThread.lockMutex();
    int c = exampleThread.getCounter();
    exampleThread.unlockMutex(); // we can unlock the mutex now, since we made a copy of the counter.

    printf("Main thread: Counter=%d.\n", c);

    if(c >= 10)
      break;

    ArUtil::sleep(600);
  }


  /* This shows how to block on an ArCondition object. 
   * wait() will *only* return when the condition object is 
   * signaled by the other thread.
   */
  ArLog::log(ArLog::Normal, "Main thread: Waiting on condition object...");
  exampleThread.waitOnCondition();

  ArLog::log(ArLog::Normal, "Main thread: Condition was signaled, and execution continued. Telling the other thread to stop running.");
  exampleThread.stopRunning();
  
  ArLog::log(ArLog::Normal, "Main thread: Waiting for the other thread to exit, then exiting the program.");
  do {
    ArUtil::sleep(250);
  } while(exampleThread.getRunningWithLock());

  ArLog::log(ArLog::Normal, "Main thread: Exiting program.");
  return 0;
}
