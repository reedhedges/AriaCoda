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
#ifndef ARRANGEDEVICETHREADED_H
#define ARRANGEDEVICETHREADED_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ArRangeDevice.h"
#include "Aria/ArFunctorASyncTask.h"

/// A range device which can run in its own thread
/** 
    This is a range device thats threaded, it doesn't do
    multipleInheritance from both ArASyncTask and ArRangeDevice any
    more since JAVA doesn't support this and the wrapper software
    can't deal with it.  Its still functionally the same however.
 **/
class ArRangeDeviceThreaded : public ArRangeDevice
{
public:
  /// Constructor
  AREXPORT ArRangeDeviceThreaded(size_t currentBufferSize, 
				 size_t cumulativeBufferSize,
				 const char *name, unsigned int maxRange,
				 int maxSecondsToKeepCurrent = 0,
				 int maxSecondsToKeepCumulative = 0,
				 double maxDistToKeepCumulative = 0,
				 bool locationDependent = false);
  /// Destructor
  virtual ~ArRangeDeviceThreaded() = default;
  /// The functor you need to implement that will be the one executed by the thread
  AREXPORT virtual void * runThread(void *arg) = 0;
  /// Run in this thread
  AREXPORT virtual void run() { myTask.run(); }
  /// Run in its own thread
  AREXPORT virtual void runAsync() { myTask.runAsync(); }
  /// Stop the thread
  AREXPORT virtual void stopRunning() { myTask.stopRunning(); }
  /// Get the running status of the thread
  AREXPORT virtual bool getRunning() { return myTask.getRunning();}
  /// Get the running status of the thread, locking around the variable
  AREXPORT virtual bool getRunningWithLock() 
    { return myTask.getRunningWithLock(); }

  AREXPORT virtual int lockDevice() { return myTask.lock(); }
  AREXPORT virtual int tryLockDevice() { return myTask.tryLock(); }
  AREXPORT virtual int unlockDevice() { return myTask.unlock(); }
protected:
  ArRetFunctor1C<void *, ArRangeDeviceThreaded, void *> myRunThreadCB;
  ArFunctorASyncTask myTask;

};

#endif // ARRANGEDEVICETHREADED_H
