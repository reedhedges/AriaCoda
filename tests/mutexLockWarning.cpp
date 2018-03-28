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

int main(int argc, char **argv) 
{
  Aria::init();
  ArMutex mutex;
  mutex.setLogName("test mutex");
  ArLog::log(ArLog::Normal, "This test succeeds if three (and only three) mutex lock/unlock time warning follow.");
  puts("setting test_mutex warning time to 1 sec");
  mutex.setUnlockWarningTime(1); // 1 sec
  puts("locking and unlocking immediately, should not warn...");
  mutex.lock();
  mutex.unlock(); // should not warn
  puts("locking and unlocking after 2 sec, should warn...");
  mutex.lock();
  ArUtil::sleep(2000); // 2 sec
  mutex.unlock(); // should warn
  puts("locking and unlocking after 0.5 sec, should not warn...");
  mutex.lock();
  ArUtil::sleep(500); // 0.5 sec
  mutex.unlock();	// should not warn
  puts("setting test_mutex warning time to 0.5 sec");
  mutex.setUnlockWarningTime(0.5); // 0.5 sec
  puts("locking and unlocking after 0.6 sec, should warn...");
  mutex.lock();
  ArUtil::sleep(600); // 0.6 sec
  mutex.unlock(); // should warn
  puts("locking and unlocking after 0.2 sec, should not warn...");
  mutex.lock();
  ArUtil::sleep(200); // 0.2 sec
  mutex.unlock(); // should not warn
  puts("locking and unlocking immediately, should not warn...");
  mutex.lock();
  mutex.unlock(); // should not warn
  puts("setting test_mutex warning time to 0.1 sec");
  mutex.setUnlockWarningTime(0.1);  // 0.1 sec
  puts("locking and unlocking after 0.2 sec, should warn...");
  mutex.lock();
  ArUtil::sleep(200); // 0.2 sec
  mutex.unlock(); // should warn
  mutex.setUnlockWarningTime(0.0); // off
  mutex.lock();
  ArUtil::sleep(100); // should not warn
  mutex.unlock();

  // Create and destroy a few mutexes, locking them, etc.
  ArMutex *m1 = new ArMutex();
  m1->setLogName("m1");
  m1->lock();
  ArMutex *m2 = new ArMutex();
  m2->lock();
  m2->setLogName("m2");
  puts("unlocking m1 before destroying it...");
  m1->unlock();
  delete m1;
  puts("NOT unlocking m2 before destroying it...");
  delete m2;

  puts("exiting with Aria::exit(0)...");
  Aria::exit(0);
}

