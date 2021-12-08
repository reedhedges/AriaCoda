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
#ifndef ARTHREAD_H
#define ARTHREAD_H



#include "Aria/ariaTypedefs.h"

#if !defined(_WIN32) || defined(MINGW)
// NOTE on MinGW pthread.h must be included after ariaTypedefs.h which includes winsock2.h.
// (on MinGW, pthread.h will include winsock.h to get an errno definition, after which
// winsock2.h will assert a preprocessor error.) 
#include <pthread.h>
#endif

#include "Aria/ArMutex.h"
#include "Aria/ArFunctor.h"
#include "Aria/ArLog.h"

#ifdef MINGW
#include <vector>
#else
#include <map>
#endif

/// POSIX/WIN32 thread wrapper class. 
/**
  create() will create the thread. That thread will run the given Functor.

  A thread can either be in a detached state or a joinable state. If the
  thread is in a detached state, that thread can not be join()'ed upon. The
  thread will simply run until the program exits, or its function exits.
  A joinable thread means that another thread and call join() upon it. If
  this function is called, the caller will block until the thread exits
  its function. This gives a way to synchronize upon the lifespan of threads.

  Calling cancel() will cancel the thread.

  The static function self() will return a thread

  @sa ArASyncTask which provides a different approach with a simpler interface.

*/
class ArThread
{
public:

#if defined(_WIN32) && !defined(MINGW)
  typedef DWORD ThreadType;
#else
  typedef pthread_t ThreadType;
#endif

// pthread_t on Linux happens to be an integer or pointer that can be used in a map. On other platforms, pthread_t may be a struct or similar, and this is true on MINGW, so store them differently.  Use the access methods that follow to access the map.
#ifdef MINGW
  typedef std::vector< std::pair<ThreadType, ArThread*> > MapType;
#else
  typedef std::map<ThreadType, ArThread*> MapType;
#endif

protected:
  static ArThread* findThreadInMap(ThreadType t);
  static void removeThreadFromMap(ThreadType t); 
  static void addThreadToMap(ThreadType pt, ArThread *at);
  
public:
  typedef enum {
    STATUS_FAILED=1, ///< Failed to create the thread
    STATUS_NORESOURCE, ///< Not enough system resources to create the thread
    STATUS_NO_SUCH_THREAD, ///< The thread can no longer be found
    STATUS_INVALID, ///< Thread is detached or another thread is joining on it
    STATUS_JOIN_SELF, ///< Thread is your own thread. Can't join on self.
    STATUS_ALREADY_DETATCHED ///< Thread is already detatched
  } Status;

  /// Constructor
  AREXPORT explicit ArThread(bool blockAllSignals=true);
  /// Constructor - starts the thread
  AREXPORT explicit ArThread(ThreadType thread, bool joinable,
		    bool blockAllSignals=true);
  /// Constructor - starts the thread
  AREXPORT explicit ArThread(ArFunctor *func, bool joinable=true,
		    bool blockAllSignals=true);
  /// Destructor
  AREXPORT virtual ~ArThread();

  /// Initialize the internal book keeping structures
  AREXPORT static void init();
  /// Returns the instance of your own thread (the current one)
  AREXPORT static ArThread * self();
  /// Returns the os self of the current thread
  AREXPORT static ThreadType osSelf();
  /// Stop all threads
  AREXPORT static void stopAll();
  /// Cancel all threads
  AREXPORT static void cancelAll();
  /// Join on all threads
  AREXPORT static void joinAll();

  /// Shuts down and deletes the last remaining thread; call after joinAll
  AREXPORT static void shutdown();

  /// Yield the processor to another thread
  AREXPORT static void yieldProcessor();
  /// Gets the logging level for thread information
  static ArLog::LogLevel getLogLevel() { return ourLogLevel; }
  /// Sets the logging level for thread information
  static void setLogLevel(ArLog::LogLevel level) { ourLogLevel = level; }

  /// Create and start the thread
  /// @param lowerPriority on Windows this sets the thread priority to THREAD_PRIORITY_IDLE.  Does nothing on Linux.
  AREXPORT virtual int create(ArFunctor *func, bool joinable=true,
			      bool lowerPriority=true);
  /// Stop the thread
  virtual void stopRunning() {myRunning=false;}
  /// Join on the thread
  AREXPORT virtual int join(void **r=NULL);
  /// Detatch the thread so it cant be joined
  AREXPORT virtual int detach();
  /// Cancel the thread
  AREXPORT virtual void cancel();

  /// Get the running status of the thread
  virtual bool getRunning() const {return(myRunning);}
  /// Get the running status of the thread, locking around the variable
  virtual bool getRunningWithLock() 
    { bool running; lock(); running = myRunning; unlock(); return running; }
  /// Get the joinable status of the thread
  virtual bool getJoinable() const {return(myJoinable);}
  /// Get the underlying thread type
  virtual const ThreadType * getThread() const {return(&myThread);}
  /// Get the underlying os thread type
  virtual ThreadType getOSThread() const {return(myThread);}
  /// Get the functor that the thread runs
  virtual ArFunctor * getFunc() const {return(myFunc);}

  /// Set the running value on the thread
  virtual void setRunning(bool running) {myRunning=running;}

#ifndef SWIG
  /** Lock the thread instance
      @swigomit
  */
  int lock() {return(myMutex.lock());}
  /** Try to lock the thread instance without blocking
      @swigomit
  */
  int tryLock() {return(myMutex.tryLock());}
  /** Unlock the thread instance
      @swigomit
  */
  int unlock() {return(myMutex.unlock());}
#endif

  /// Do we block all process signals at startup?
  bool getBlockAllSignals() {return(myBlockAllSignals);}

  /// Gets the name of the thread
  virtual const char *getThreadName() { return myName.c_str();  }

  /// Sets the name of the thread
  AREXPORT virtual void setThreadName(const char *name);

  /// Gets a string that describes the thread status.
  /// (e.g. for debug logging or UI diagnostic display)
  virtual std::string getThreadActivity() { 
    if(getRunning())
      return myName + " (running)";
    else
      return myName + " (not running)";
   }

  /// Marks the thread as started and logs useful debugging information.
  /**
     If you call this function in your functor (ie runThread) it'll
     then call some things for logging (to make debugging easier)
     This method should be called before the main thread loop begins.
   **/
  AREXPORT virtual void threadStarted();

  /// Marks the thread as finished and logs useful debugging information.
  /**
     This method should be called after the main thread loop ends.  It
     enables the creator of the thread to determine that the thread has
     actually been completed and can be deleted.
   **/
  AREXPORT virtual void threadFinished();

  /// Returns whether the thread has been started.
  /**
   * This is dependent on the thread implementation calling the 
   * threadStarted() method.
  **/
  AREXPORT virtual bool isThreadStarted() const;

  /// Returns whether the thread has been completed and can be deleted.
  /**
   * This is dependent on the thread implementation calling the 
   * threadFinished() method.
  **/
  AREXPORT virtual bool isThreadFinished() const;


  /// Logs the information about this thread
  AREXPORT virtual void logThreadInfo();

#ifndef _WIN32
  pid_t getPID() { return myPID; }
  pid_t getTID() { return myTID; }
#endif

  /// Gets the name of the this thread
  AREXPORT static const char *getThisThreadName();
  /// Get the underlying thread type of this thread
  AREXPORT static const ThreadType * getThisThread();
  /// Get the underlying os thread type of this thread
  AREXPORT static ThreadType getThisOSThread();

protected:
  static ArMutex ourThreadsMutex;
  static MapType ourThreads;
#if defined(_WIN32) && !defined(MINGW)
  static std::map<HANDLE, ArThread *> ourThreadHandles;
#endif 
  AREXPORT static ArLog::LogLevel ourLogLevel; 

  AREXPORT virtual int doJoin(void **ret=NULL);

  std::string myName;

  ArMutex myMutex;
  /// State variable to denote when the thread should continue or exit
  bool myRunning;
  bool myJoinable;
  bool myBlockAllSignals;

  bool myStarted;
  bool myFinished;

  ArStrMap myStrMap;
  ArFunctor *myFunc;
  ThreadType myThread;
#if defined(_WIN32) && !defined(MINGW)
  HANDLE myThreadHandle;
#endif

  
#if !defined(_WIN32) || defined(MINGW)
  pid_t myPID;
  pid_t myTID;
#endif

  static std::string ourUnknownThreadName;
};


#endif // ARTHREAD_H
