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
#include "Aria.h"
//#include <time.h>

class TestThread : public ArASyncTask
{
public:
  TestThread(int number, ArMutex &mutex);
  ~TestThread(void) {}

  virtual void * runThread(void *arg);

  int myNum;
  ArMutex &myMutex;
};

TestThread::TestThread(int number, ArMutex &mutex) :
  myNum(number),
  myMutex(mutex)
{
}

void *TestThread::runThread(void *arg)
{
  unsigned int interval;

puts("new thread running...");
  while (myRunning)
  {
    // Yield the processor here so that the other threads get a chance
    // to get the lock. calling ArThread::yield() will give sequential
    // running of the threads.
    //yield();

    // Sleeping of 1 microsecond randomizes the order of running of the
    // threads.
    ArUtil::sleep(1);

    myMutex.lock();
    interval=rand()%10000;
    printf("Thread %d: Locked. Going to sleep for %dms\n", myNum, interval);
    ArUtil::sleep(interval);
    printf("Thread %d: Unlocking...\n", myNum);
    myMutex.unlock();
  }

  return NULL;
}

int main(int argc, char **argv) 
{
puts("Aria init...");
fflush(stdout);
  Aria::init(Aria::SIGHANDLE_THREAD, false);

  ArMutex mutex;
  mutex.setLogName("mutex");

  ArMutex::setLockWarningTime(1);
  ArMutex::setUnlockWarningTime(5);
  TestThread thread1(1, mutex), thread2(2, mutex), thread3(3, mutex),
    thread4(4, mutex);

  thread1.setThreadName("thread1");
  thread2.setThreadName("thread2");
  thread3.setThreadName("thread3");
  thread4.setThreadName("thread4");
puts("srand...");
  srand(time(0));


puts("creating 3 threads...");
  thread1.create();
  thread2.create();
  thread3.create();

  printf("main thread name=\"%s\", OS handle=%lu, OS pointer=0x%x\n", ArThread::getThisThreadName(), ArThread::getThisOSThread(), (unsigned int) ArThread::getThisThread());
  printf("thread1 thread name=\"%s\", OS handle=%lu, OS pointer=0x%x\n", thread1.getThreadName(), thread1.getOSThread(), (unsigned int) thread1.getThread());
  printf("thread2 thread name=\"%s\", OS handle=%lu, OS pointer=0x%x\n", thread2.getThreadName(), thread2.getOSThread(), (unsigned int) thread2.getThread());
  printf("thread3 thread name=\"%s\", OS handle=%lu, OS pointer=0x%x\n", thread3.getThreadName(), thread3.getOSThread(), (unsigned int) thread3.getThread());
  printf("thread4 (not created yet) thread name=\"%s\", OS handle=%lu, OS pointer=0x%x\n", thread4.getThreadName(), thread4.getOSThread(), (unsigned int) thread4.getThread());

#ifndef MINGW
  if(ArThread::getThisOSThread() == thread1.getOSThread() ||
     ArThread::getThisOSThread() == thread2.getOSThread() ||
     ArThread::getThisOSThread() == thread3.getOSThread() ||
     ArThread::getThisOSThread() == thread4.getOSThread() ||
     thread1.getOSThread() == thread2.getOSThread() ||
     thread1.getOSThread() == thread3.getOSThread() ||
     thread1.getOSThread() == thread4.getOSThread() ||
     thread2.getOSThread() == thread1.getOSThread() ||
     thread2.getOSThread() == thread3.getOSThread() ||
     thread2.getOSThread() == thread4.getOSThread() ||
     thread3.getOSThread() == thread1.getOSThread() ||
     thread3.getOSThread() == thread2.getOSThread() ||
     thread3.getOSThread() == thread4.getOSThread() ||
     thread4.getOSThread() == thread1.getOSThread() ||
     thread4.getOSThread() == thread2.getOSThread() ||
     thread4.getOSThread() == thread3.getOSThread() )
  {
    puts("error, some thread IDs are the same!");
    return 5;
  }
#endif

puts("run thread 4 in main thread");
  thread4.runInThisThread();

puts("exit");
  Aria::exit(0);

  return(0);
}

