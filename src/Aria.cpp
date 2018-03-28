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
#include "Aria.h"
#include "ArSocket.h"
#include "ArSignalHandler.h"
#include "ArKeyHandler.h"
#include "ArJoyHandler.h"
//#include "ArModuleLoader.h"
#include "ArRobotJoyHandler.h"
#ifndef __MACH__
#include "ArSystemStatus.h"
#endif

// to register PTZ types with PTZConnector:
#include "ArPTZConnector.h"
#include "ArRVisionPTZ.h"
#include "ArVCC4.h"
#include "ArDPPTU.h"
//#include "ArSonyPTZ.h"



ArGlobalFunctor1<int> Aria::ourSignalHandlerCB(&Aria::signalHandlerCB);
ArMutex Aria::ourShuttingDownMutex;
bool Aria::ourShuttingDown=false;
bool Aria::ourExiting=false;
std::string Aria::ourDirectory="";
std::list<ArFunctor*> Aria::ourInitCBs;
std::list<ArFunctor*> Aria::ourUninitCBs;
ArKeyHandler *Aria::ourKeyHandler = NULL;
ArJoyHandler *Aria::ourJoyHandler = NULL;
bool Aria::ourInited = false;
bool Aria::ourRunning = false;
ArMutex Aria::ourExitCallbacksMutex;
std::multimap<int, ArFunctor *> Aria::ourExitCallbacks;
bool Aria::ourSigHandleExitNotShutdown = true;
std::multimap<int, ArRetFunctor<bool> *> Aria::ourParseArgCBs;
ArLog::LogLevel Aria::ourParseArgsLogLevel = ArLog::Verbose;
std::multimap<int, ArFunctor *> Aria::ourLogOptionsCBs;
ArLog::LogLevel Aria::ourExitCallbacksLogLevel = ArLog::Verbose;
std::map<std::string, ArRetFunctor3<ArDeviceConnection *, const char *, const char *, const char *> *, ArStrCaseCmpOp> Aria::ourDeviceConnectionCreatorMap;
std::string Aria::ourDeviceConnectionTypes;
std::string Aria::ourDeviceConnectionChoices = "Choices:";

std::list<ArRobot*> Aria::ourRobots;
ArConfig Aria::ourConfig;
ArRobotJoyHandler *Aria::ourRobotJoyHandler = NULL;
ArStringInfoGroup Aria::ourInfoGroup;
int Aria::ourMaxNumLasers = 2;
int Aria::ourMaxNumSonarBoards = 1;
int Aria::ourMaxNumBatteries = 1;
int Aria::ourMaxNumLCDs = 1;
std::map<std::string, ArRetFunctor2<ArLaser *, int, const char *> *, ArStrCaseCmpOp> Aria::ourLaserCreatorMap;
std::string Aria::ourLaserTypes;
std::string Aria::ourLaserChoices = "Choices:";
std::map<std::string, ArRetFunctor2<ArBatteryMTX *, int, const char *> *, ArStrCaseCmpOp> Aria::ourBatteryCreatorMap;
std::string Aria::ourBatteryTypes;
std::string Aria::ourBatteryChoices = "Choices:";
std::map<std::string, ArRetFunctor2<ArLCDMTX *, int, const char *> *, ArStrCaseCmpOp> Aria::ourLCDCreatorMap;
std::string Aria::ourLCDTypes;
std::string Aria::ourLCDChoices = "Choices:";
std::map<std::string, ArRetFunctor2<ArSonarMTX *, int, const char *> *, ArStrCaseCmpOp> Aria::ourSonarCreatorMap;
std::string Aria::ourSonarTypes;
std::string Aria::ourSonarChoices = "Choices:";

size_t Aria::ourMaxNumVideoDevices = 8;
size_t Aria::ourMaxNumPTZs = 8;


std::string Aria::ourIdentifier = "generic";

/**
   This must be called first before any other Aria functions.
   It initializes the thread layer and the signal handling method. For
   Windows it iniatializes the socket layer as well. This also sets the
   directory Aria is located in using ARIA environmental variable (or Aria registry key on Windows).
   For more about Aria's directory see getDirectory() and setDirectory().

   For Linux the default signal handling method is to intercept OS signals
   in a dedicated "signal handling thread", which cleanly closes down the
   program, causes all the instances of ArRobot to stop their run loop and
   disconnect from their robot. The program will exit on the following signals:
   SIGHUP, SIGINT, SIGQUIT, and SIGTERM.

   For Windows, there is no signal handling.

   This method also adds the file /etc/Aria.args and the environment variable
   ARIAARGS as locations for ArArgumentParser to obtain default argument values
   from. 

   @param method the method in which to handle signals. Defaulted to SIGHANDLE_SINGLE, or the method indicated by the ARIA_SIGHANDLE_METHOD environment variable (NONE, SINGLE or THREAD), if it exists. 
   @param initSockets specify whether or not to initialize the socket layer. This is only meaningfull for Windows. Defaulted to true.

   @param sigHandleExitNotShutdown if this is true and a signal
   happens Aria will use Aria::exit() to quit instead of Aria::shutdown(), false will
   still use shutdown() which is the old behavior.

   @see ArSignalHandler
   @see ArSocket

 */
AREXPORT void Aria::init(SigHandleMethod method, bool initSockets, 
			 bool sigHandleExitNotShutdown)
{
  // get this here so that the program update can be accurate
#if !defined(__MACH__) && !defined(WIN32) // getUptime() not available on Windows or MacOSX
  ArSystemStatus::getUptime();
#endif

  std::list<ArFunctor*>::iterator iter;
  std::string str;
  char buf[1024];

  // make an ArTime and set it to now so that the good clock value will be set
  ArTime timeInit;
  timeInit.setToNow();


  ourShuttingDownMutex.setLogName("Aria::ourShuttingDownMutex");
  ourExitCallbacksMutex.setLogName("Aria::ourExitCallbacksMutex");
  if (ourInited == true)
    return;

  ourRunning = true;
#ifndef WIN32
  srand48(time(NULL));
#endif

  ArThread::init();

  char* overrideSigMethod = getenv("ARIA_SIGHANDLE_METHOD");
  if(overrideSigMethod)
  {
    ArLog::log(ArLog::Terse, "Overriding signal handler method with %s from ARIA_SIGHANDLE_METHOD environment variable.", overrideSigMethod);
    if(!strcmp(overrideSigMethod, "NONE"))
      method = SIGHANDLE_NONE;
    else if(!strcmp(overrideSigMethod, "SINGLE"))
      method = SIGHANDLE_SINGLE;
    else if(!strcmp(overrideSigMethod, "THREAD"))
      method = SIGHANDLE_THREAD;
  }

  if (method != SIGHANDLE_NONE)
  {
    ArSignalHandler::addHandlerCB(&ourSignalHandlerCB, ArListPos::LAST);
    ArSignalHandler::blockCommon();
    ArSignalHandler::handle(ArSignalHandler::SigHUP);
    ArSignalHandler::handle(ArSignalHandler::SigINT);
    ArSignalHandler::handle(ArSignalHandler::SigQUIT);
    ArSignalHandler::handle(ArSignalHandler::SigTERM);
    ArSignalHandler::handle(ArSignalHandler::SigPIPE);
    if (method == SIGHANDLE_SINGLE)
        ArSignalHandler::createHandlerNonThreaded();
    else if (method == SIGHANDLE_THREAD)
    {
      ArSignalHandler::blockCommonThisThread();
      ArSignalHandler::createHandlerThreaded();
    }
  }

  if (initSockets)
    ArSocket::init();

  if (ourDirectory.length() == 0)
  {
    if (getenv("ARIA") != NULL)
    {
      setDirectory(getenv("ARIA"));
    }
    else
    {
#ifndef WIN32
      ArUtil::getStringFromFile("/etc/Aria", buf, sizeof(buf));
      str = buf;
#else // WIN32
      if (ArUtil::findFirstStringInRegistry(
          "SOFTWARE\\MobileRobots\\Aria",
          "Install Directory", buf, 1024))
        str = buf;
      else
        if (ArUtil::findFirstStringInRegistry(
            "SOFTWARE\\ActivMedia Robotics\\Aria",
            "Install Directory", buf, 1024))
          str = buf;
        else
          str = "";
        
  #endif // WIN32
        if (str.length() > 0)
        {
    setDirectory(str.c_str());
        }
        else
        {
    ArLog::log(ArLog::Terse, "NonCritical Error: ARIA could not find where it is located.");
        }
      }
    }
  ourSigHandleExitNotShutdown = sigHandleExitNotShutdown;
  
  Aria::laserAddCreator("lms2xx", ArLaserCreatorHelper::getCreateLMS2xxCB());
  Aria::laserAddCreator("urg", ArLaserCreatorHelper::getCreateUrgCB());
  Aria::laserAddCreator("lms1XX", ArLaserCreatorHelper::getCreateLMS1XXCB());
  Aria::laserAddCreator("urg2.0", ArLaserCreatorHelper::getCreateUrg_2_0CB());
  Aria::laserAddCreator("s3series", ArLaserCreatorHelper::getCreateS3SeriesCB());
  Aria::laserAddCreator("lms5XX", ArLaserCreatorHelper::getCreateLMS5XXCB());
  Aria::laserAddCreator("tim3XX", ArLaserCreatorHelper::getCreateTiM3XXCB());
  Aria::laserAddCreator("tim5XX", ArLaserCreatorHelper::getCreateTiM3XXCB());
  Aria::laserAddCreator("tim510", ArLaserCreatorHelper::getCreateTiM3XXCB());
  Aria::laserAddCreator("tim551", ArLaserCreatorHelper::getCreateTiM551CB());
  Aria::laserAddCreator("tim561", ArLaserCreatorHelper::getCreateTiM561CB());
  Aria::laserAddCreator("tim571", ArLaserCreatorHelper::getCreateTiM571CB());
  Aria::laserAddCreator("sZseries", ArLaserCreatorHelper::getCreateSZSeriesCB());
  Aria::batteryAddCreator("mtx", ArBatteryMTXCreatorHelper::getCreateBatteryMTXCB());
  Aria::lcdAddCreator("mtx", ArLCDMTXCreatorHelper::getCreateLCDMTXCB());
  Aria::sonarAddCreator("mtx", ArSonarMTXCreatorHelper::getCreateSonarMTXCB());
  //Aria::batteryAddCreator("mtxbatteryv1", ArBatteryMTXCreatorHelper::getCreateBatteryMTXCB());
  //Aria::lcdAddCreator("mtxlcdv1", ArLCDMTXCreatorHelper::getCreateLCDMTXCB());
  //Aria::sonarAddCreator("mtxsonarv1", ArSonarMTXCreatorHelper::getCreateSonarMTXCB());

  Aria::deviceConnectionAddCreator(
	  "serial", ArDeviceConnectionCreatorHelper::getCreateSerialCB());
  Aria::deviceConnectionAddCreator(
	  "serial422", ArDeviceConnectionCreatorHelper::getCreateSerial422CB());
  Aria::deviceConnectionAddCreator(
	  "tcp", ArDeviceConnectionCreatorHelper::getCreateTcpCB());

  ourInited = true;

  for (iter=ourInitCBs.begin(); iter !=  ourInitCBs.end(); ++iter)
    (*iter)->invoke();

  ArArgumentParser::addDefaultArgumentFile("/etc/Aria.args");
  ArArgumentParser::addDefaultArgumentEnv("ARIAARGS");
  
  ArVCC4::registerPTZType();
  ArRVisionPTZ::registerPTZType();
  ArDPPTU::registerPTZType();
  //ArSonyPTZ::registerPTZType();
}


/**
   This must be called last, after all other Aria functions.
   For both Linux and Windows, it closes all the open ArModules. For Windows
   it deinitializes the socket layer as well.
*/
AREXPORT void Aria::uninit()
{
  std::list<ArFunctor*>::iterator iter;

  for (iter=ourUninitCBs.begin(); iter != ourUninitCBs.end(); ++iter)
    (*iter)->invoke();

//  ArModuleLoader::closeAll();
  ArSocket::shutdown();
  ArThread::shutdown();
}

/**
   This will add a callback to the list of callbacks to call when Aria
   has been initialized. It can be called before anything else.
*/
AREXPORT void Aria::addInitCallBack(ArFunctor *cb, ArListPos::Pos position)
{
  if (position == ArListPos::FIRST)
    ourInitCBs.push_front(cb);
  else
    ourInitCBs.push_back(cb);
}

/**
   This will add a callback to the list of callbacks to call right before Aria
   is un-initialized. It can be called before anything else. This facilitates
   code that in operating system signal handlers simply calls Aria::uninit()
   and packages that are based on Aria are unitited as well. It simplifies
   the entire uninit process.
*/
AREXPORT void Aria::addUninitCallBack(ArFunctor *cb, ArListPos::Pos position)
{
  if (position == ArListPos::FIRST)
    ourUninitCBs.push_front(cb);
  else
    ourUninitCBs.push_back(cb);
}

/**
   Use this function to clean up or uninitialize Aria, in particular,
   to stop background threads.
   (Note: If you want to shutdown ARIA cleanly at the
   end of your program or when exiting the program due to error, use 
   Aria::exit() instead.)
   This calls stop() on all ArThread's and ArASyncTask's. It will
   block until all ArThread's and ArASyncTask's exit. It is expected
   that all the tasks will obey the ArThread::myRunning variable and
   exit when it is false. Note, this only stop Aria's background threads,
   it does not exit the program. 
   If you want to shutdown ARIA cleanly at the
   end of your program or when exiting the program due to error, use 
   Aria::exit() instead. 
   @sa Aria::exit()
*/
AREXPORT void Aria::shutdown()
{
  bool doExit=false;

  ourShuttingDownMutex.lock();
  ourRunning = false;
  if (ourShuttingDown)
    doExit=true;
  else
    ourShuttingDown=true;
  ourShuttingDownMutex.unlock();

  if (doExit)
    return;

  ArThread::stopAll();
  ArThread::joinAll();
  uninit();
}

/**
   This will call the list of Aria exit callbacks (added by addExitCallback())
   and then exit the program with
   the given exit code.   This method may be used as a replacement for the
   standard system ::exit() call (which is  used by this method
   to finally exit the program with the given @a exitCode after finishing Aria
   shutdown steps.)

   Note that this could be called from anywhere,
   mutexes may be locked when called-- all exit
   callbacks MUST return and cannot wait for a lock, since this could result 
   in the program hanging due to double-lock (deadlock).

   @sa addExitCallback()
**/
AREXPORT void Aria::exit(int exitCode)
{
  bool doReturn = false;
  ourShuttingDownMutex.lock();
  ourRunning = false;
  if (ourExiting)
    doReturn=true;
  else
    ourExiting=true;
  ourShuttingDownMutex.unlock();

  if (doReturn)
    return;

  callExitCallbacks();
  ::exit(exitCode);
}

/** @internal */
AREXPORT void Aria::callExitCallbacks(void)
{
  ourRunning = false;

  ourShuttingDownMutex.lock();
  ourExiting = true;
  ourShuttingDownMutex.unlock();

  std::multimap<int, ArFunctor *>::reverse_iterator it;

  
 
  ourExitCallbacksMutex.lock();
  ArLog::log(ourExitCallbacksLogLevel, "Aria::exit: Starting exit callbacks");
  for (it = ourExitCallbacks.rbegin(); it != ourExitCallbacks.rend(); it++)
  {
    ArLog::log(ourExitCallbacksLogLevel,
	       "Aria::exit: Calling callback at position %d with name '%s'",
	       (*it).first, (*it).second->getName());
    (*it).second->invoke();
    ArLog::log(ourExitCallbacksLogLevel,
	       "Aria::exit: Called callback at position %d with name '%s'",
	       (*it).first, (*it).second->getName());

  }
  ArLog::log(ourExitCallbacksLogLevel, "Aria::exit: Finished exit callbacks");
  ourExitCallbacksMutex.unlock();
} 
 
/**
 * Note, an exit callback may be called at any time; it should not
 * block or wait on any locked mutexes or similar.
 */
AREXPORT void Aria::addExitCallback(ArFunctor *functor, int position)
{
  if (ourExiting)
  {
    ArLog::log(ArLog::Normal, 
	       "Aria::addExitCallback: Could not add exit callback '%s' since in the middle of exiting", functor->getName());
    return;
  }

  ourExitCallbacksMutex.lock();
  ArLog::log(ourExitCallbacksLogLevel,
	"Aria::addExitCallback: Adding callback at position %d with name '%s'",
	     position, functor->getName());
  ourExitCallbacks.insert(std::pair<int, ArFunctor *>(position, functor));
  ourExitCallbacksMutex.unlock();
}

/**
 Note, an exit callback may be called at any time; it should not *
 block or wait on any locked mutexes or similar.
 
 Also note that this will not actually happen if it is called while
 aria is exiting (ie it is walking through the list).

 @since 2.7.0
 */
AREXPORT void Aria::remExitCallback(ArFunctor *functor)
{
  if (ourExiting)
  {
    ArLog::log(ArLog::Normal, 
	       "Aria::remExitCallback: Could not remove exit callback '%s' since in the middle of exiting", functor->getName());
    return;
  }

  std::multimap<int, ArFunctor *>::iterator it;
  
  ourExitCallbacksMutex.lock();
  for (it = ourExitCallbacks.begin(); it != ourExitCallbacks.end(); it++)
  {
    if ((*it).second == functor)
    {
      ourExitCallbacks.erase(it);
      ourExitCallbacksMutex.unlock();
      return remExitCallback(functor);
    }
  }
  ourExitCallbacksMutex.unlock();
}

/**
   This method is roughly obsolete, it simply forces all the threads to die
   immediately and then exits... The new method Aria::exit() is preferred.

   This calls cancel() on all AtThread's and ArASyncTask's. It forces
   each thread to exit and should only be used in the case of a thread
   hanging or getting stuck in an infinite loop. This works fine in Linux.
   In Windows it is not recommended at all that this function be called.
   Windows can not handle cleanly killing off a thread. See the help in
   the VC++ compiler on the WIN32 function TerminateThread. The biggest
   problem is that the state of DLL's can be destroyed.  

   @deprecated
**/

AREXPORT void Aria::exitOld(int exitCode)
{
  ourRunning = false;
  ArThread::cancelAll();
  uninit();
  ::exit(exitCode);
}

AREXPORT void Aria::addRobot(ArRobot *robot)
{
  ourRobots.push_back(robot);
}

AREXPORT void Aria::delRobot(ArRobot *robot)
{
  ourRobots.remove(robot);
}

/**
   @param name the name of the robot you want to find
   @return NULL if there is no robot of that name, otherwise the robot with 
   that name
*/
AREXPORT ArRobot *Aria::findRobot(char *name)
{
  std::string rname;
  std::list<ArRobot *>::iterator it;
  if (name == NULL)
    return NULL;

  rname = name;
  for (it = ourRobots.begin(); it != ourRobots.end(); it++)
  {
    if ((*it)->getName() == rname)
      return (*it);
  }
  return NULL;

}

AREXPORT std::list<ArRobot*> * Aria::getRobotList()
{
  return(&ourRobots);
}


/** @internal */
AREXPORT void Aria::signalHandlerCB(int sig)
{

  // if we want to exit instead of shutdown then do that ( call never returns)
  if (ourSigHandleExitNotShutdown)
  {
    ArLog::log(ArLog::Normal, "Aria: Received signal '%s'. Exiting.",
	       ArSignalHandler::nameSignal(sig));
    Aria::exit(0);
    // we shouldn't need this here, since the program should already
    // exited... but just in case
    ::exit(0);
  }


  ourShuttingDownMutex.lock();
  if (!ourRunning)
  {
    ourShuttingDownMutex.unlock();
    return;
  }
  ourShuttingDownMutex.unlock();


  ArLog::log(ArLog::Normal, "Aria: Received signal '%s'. Shutting down.",
	     ArSignalHandler::nameSignal(sig));

  std::list<ArRobot*>::iterator iter;
  if ((sig == ArSignalHandler::SigINT) || (sig == ArSignalHandler::SigHUP) ||
      (sig == ArSignalHandler::SigTERM))
  {
    for (iter=ourRobots.begin(); iter != ourRobots.end(); ++iter)
      (*iter)->stopRunning();
  }

  // I'm disregarding this advice below since I can't seem to get
  // anything else to work well and haven't seen problems from it

  // dont do an Aria::shutdown() here because we want the main()
  // function to do the ArThread::joinAll(). Otherwise variables on
  // the stack frame of main() may get destructed if main() happens to
  // exit before other threads.  And some of those variables may be
  // used by those threads.
  shutdown();
}

/**
   This sets the directory that ARIA is located in, so ARIA can find param
   files and the like.  This can also be controlled by the environment variable
   ARIA, which this is set to (if it exists) when Aria::init is done.  So 
   for setDirectory to be effective, it must be done after the Aria::init.
   @param directory the directory Aria is located in
   @see getDirectory
*/
AREXPORT void Aria::setDirectory(const char *directory)
{
  int ind;
  if (directory != NULL)
  {
    ourDirectory = directory;
    ind = strlen(directory) - 1;
    if (ind < 0)
      ind = 0;
    if (directory[ind] != '/' && directory[ind] != '\\')
    {
#ifdef WIN32
      ourDirectory += "\\";
#else // win32
      ourDirectory += "/";
#endif // win32
    }
    ourConfig.setBaseDirectory(ourDirectory.c_str());
  }
}

/**
   This gets the directory that ARIA is located in, this is so ARIA can find 
   param files and the like.  
   @return the directory ARIA is located in
   @see setDirectory
*/
AREXPORT const char *Aria::getDirectory(void)
{
  return ourDirectory.c_str();
}

/// Sets the key handler, so that other classes can find it
AREXPORT void Aria::setKeyHandler(ArKeyHandler *keyHandler)
{
  ourKeyHandler = keyHandler;
}

/// Gets the key handler if one has been set
AREXPORT ArKeyHandler *Aria::getKeyHandler(void)
{
  return ourKeyHandler;
}

/// Sets the joy handler, so that other classes can find it
AREXPORT void Aria::setJoyHandler(ArJoyHandler *joyHandler)
{
  ourJoyHandler = joyHandler;
}

/// Gets the joy handler if one has been set
AREXPORT ArJoyHandler *Aria::getJoyHandler(void)
{
  return ourJoyHandler;
}

/// Sets the robot joy handler, so that other classes can find it
AREXPORT void Aria::setRobotJoyHandler(ArRobotJoyHandler *robotJoyHandler)
{
  ourRobotJoyHandler = robotJoyHandler;
}

/// Gets the robot joy handler if one has been set
AREXPORT ArRobotJoyHandler *Aria::getRobotJoyHandler(void)
{
  return ourRobotJoyHandler;
}

/**
   This gets the global config aria uses.
 **/
AREXPORT ArConfig *Aria::getConfig(void)
{
  return &ourConfig;
}

/**
   This gets the global string group aria uses.
 **/
AREXPORT ArStringInfoGroup *Aria::getInfoGroup(void)
{
  return &ourInfoGroup;
}

/**
   Gets the maximum number of lasers to check for and use
**/
AREXPORT int Aria::getMaxNumLasers(void)
{
  return ourMaxNumLasers;
}

/**
   Sets the maximum number of lasers to check for and use, if you are
   going to set this you should do it after the Aria::init before you
   start making laser connectors or robots or robot params.
**/
AREXPORT void Aria::setMaxNumLasers(int maxNumLasers)
{
  ourMaxNumLasers = maxNumLasers;
}

/**
   Gets the maximum number of sonar boards to check for and use
**/
AREXPORT int Aria::getMaxNumSonarBoards(void)
{
  return ourMaxNumSonarBoards;
}

/**
   Sets the maximum number of sonars to check for and use, if you are
   going to set this you should do it after the Aria::init
**/
AREXPORT void Aria::setMaxNumSonarBoards(int maxNumSonarBoards)
{
  ourMaxNumSonarBoards = maxNumSonarBoards;
}

/**
   Gets the maximum number of batteries to check for and use
**/
AREXPORT int Aria::getMaxNumBatteries(void)
{
  return ourMaxNumBatteries;
}

/**
   Sets the maximum number of sonars to check for and use, if you are
   going to set this you should do it after the Aria::init
**/
AREXPORT void Aria::setMaxNumBatteries(int maxNumBatteries)
{
  ourMaxNumBatteries = maxNumBatteries;
}

/**
   Gets the maximum number of lcds to check for and use
**/
AREXPORT int Aria::getMaxNumLCDs(void)
{
  return ourMaxNumLCDs;
}

/**
   Sets the maximum number of sonars to check for and use, if you are
   going to set this you should do it after the Aria::init
**/
AREXPORT void Aria::setMaxNumLCDs(int maxNumLCDs)
{
  ourMaxNumLCDs = maxNumLCDs;
}



/**
   returns true if ARIA is initialized (Aria::init() has been called) and has not been shut down by
   a call to Aria::shutdown() or Aria::exit() and an operating system signal has not occured (e.g. 
   external KILL signal)
**/
AREXPORT bool Aria::getRunning(void)
{
  return ourRunning;
}

/** @sa addParseArgsCB() */
AREXPORT bool Aria::parseArgs(void)
{
  std::multimap<int, ArRetFunctor<bool> *>::reverse_iterator it;
  ArRetFunctor<bool> *callback;

  ArLog::log(ourParseArgsLogLevel, "Aria: Parsing arguments");
  for (it = ourParseArgCBs.rbegin(); it != ourParseArgCBs.rend(); it++)
  {
    callback = (*it).second;
    if (callback->getName() != NULL && callback->getName()[0] != '\0')
      ArLog::log(ourParseArgsLogLevel, 
		 "Aria: Calling parse arg functor '%s' (%d)", 
		 callback->getName(), (*it).first);
    else
      ArLog::log(ourParseArgsLogLevel, 
		 "Aria: Calling unnamed parse arg functor (%d)", 
		 (*it).first);

    if (!callback->invokeR())
    {
      if (callback->getName() != NULL && callback->getName()[0] != '\0')
	ArLog::log(ArLog::Terse,
		   "Aria::parseArgs: Failed, parse arg functor '%s' (%d) returned false", 
		   callback->getName(), (*it).first);
      else
	ArLog::log(ArLog::Terse,
		   "Aria::parseArgs: Failed unnamed parse arg functor (%d) returned false", 
		   (*it).first);
      return false;
    }
  }
  return true;
}

/** @sa addLogOptionsCB() */
AREXPORT void Aria::logOptions(void)
{
  std::multimap<int, ArFunctor *>::reverse_iterator it;

  ArLog::log(ArLog::Terse, "");
  ArLog::log(ArLog::Terse, "");
  for (it = ourLogOptionsCBs.rbegin(); it != ourLogOptionsCBs.rend(); it++)
  {
    (*it).second->invoke();
    ArLog::log(ArLog::Terse, "");
    ArLog::log(ArLog::Terse, "");
  }
  ArLog::log(ArLog::Terse, "");
  ArLog::log(ArLog::Terse, "");
}

AREXPORT void Aria::addParseArgsCB(ArRetFunctor<bool> *functor, 
					  int position)
{
  ourParseArgCBs.insert(std::pair<int, ArRetFunctor<bool> *>(position, 
							     functor));
}

AREXPORT void Aria::setParseArgLogLevel(ArLog::LogLevel level)
{
  ourParseArgsLogLevel = level;
}

AREXPORT void Aria::addLogOptionsCB(ArFunctor *functor, int position)
{
  ourLogOptionsCBs.insert(std::pair<int, ArFunctor *>(position, functor));
}

AREXPORT void Aria::setExitCallbacksLogLevel(ArLog::LogLevel level)
{
  ourExitCallbacksLogLevel = level;
}

/**
   This adds a functor which can create a laser of a given type. 

   @param laserType The laser type that the creator will make (these
   are always checked case insensitively).  If
   there is already a creator for this type, then the old one is
   replaced and the new one is used.

   @param creator A functor which takes an int (laser number) and a
   const char * (logPrefix) and returns a new laser of the laserType
**/
AREXPORT bool Aria::laserAddCreator(
	const char *laserType, 
	ArRetFunctor2<ArLaser *, int, const char *> *creator)
{
  if (ourLaserCreatorMap.find(laserType) != ourLaserCreatorMap.end())
  {
    ArLog::log(ArLog::Normal, "Aria::laserAddCreator: There is already a laser creator for %s, replacing it", laserType);
    ourLaserCreatorMap.erase(laserType);
  }
  else
  {
    // if we haven't added any types add to the choices (there's an
    // intro string in choices, so it's checking the other variable)
    if (!ourLaserTypes.empty())
      ourLaserChoices += ";;";
    ourLaserChoices += laserType;

    if (!ourLaserTypes.empty())
      ourLaserTypes += "|";
    ourLaserTypes += laserType;
  }

  ourLaserCreatorMap[laserType] = creator;
  return true;
}

/**
   Gets a string that is the types of lasers that can be created
   separated by | characters.  Mostly for internal use by ArLaserConnector.
**/
AREXPORT const char *Aria::laserGetTypes(void)
{
  return ourLaserTypes.c_str();
}

/**
   Gets a string that is the types of lasers that can be created
   separated by ;; characters.  Mostly for internal use by the config
**/
AREXPORT const char *Aria::laserGetChoices(void)
{
  return ourLaserChoices.c_str();
}

/**
   Creates a laser of a given type, with the given laserNumber, and
   uses the logPrefix for logging what happens.  This is mostly for
   internal use by ArLaserConnector.

   @param laserType The type of laser to create

   @param laserNumber The laser number to use for the created laser
   
   @param logPrefix The prefix to use when logging 
*/

AREXPORT ArLaser *Aria::laserCreate(const char *laserType, int laserNumber,
				    const char *logPrefix)
{
  std::map<std::string, ArRetFunctor2<ArLaser *, int, const char *> *, ArStrCaseCmpOp>::iterator it;
  if ((it = ourLaserCreatorMap.find(laserType)) == ourLaserCreatorMap.end())
  {
    ArLog::log(ArLog::Normal, "%sCannot create a laser of type %s options are <%s>", logPrefix, laserType, laserGetTypes());
    return NULL;
  }
  
  return (*it).second->invokeR(laserNumber, logPrefix);
}


/**
   This adds a functor which can create a battery of a given type. 

   @param batteryType The battery type that the creator will make (these
   are always checked case insensitively).  If
   there is already a creator for this type, then the old one is
   replaced and the new one is used.

   @param creator A functor which takes an int (battery number) and a
   const char * (logPrefix) and returns a new battery of the batteryType
**/
AREXPORT bool Aria::batteryAddCreator(
	const char *batteryType, 
	ArRetFunctor2<ArBatteryMTX *, int, const char *> *creator)
{
  if (ourBatteryCreatorMap.find(batteryType) != ourBatteryCreatorMap.end())
  {
    ArLog::log(ArLog::Normal, "Aria::batteryAddCreator: There is already a battery creator for %s, replacing it", batteryType);
    ourBatteryCreatorMap.erase(batteryType);
  }
  else
  {
    // if we haven't added any types add to the choices (there's an
    // intro string in choices, so it's checking the other variable)
    if (!ourBatteryTypes.empty())
      ourBatteryChoices += ";;";
    ourBatteryChoices += batteryType;

    if (!ourBatteryTypes.empty())
      ourBatteryTypes += "|";
    ourBatteryTypes += batteryType;
  }

  ourBatteryCreatorMap[batteryType] = creator;
  return true;
}

/**
   Gets a string that is the types of batteries that can be created
   separated by | characters.  Mostly for internal use by ArBatteryConnector.
**/
AREXPORT const char *Aria::batteryGetTypes(void)
{
  return ourBatteryTypes.c_str();
}

/**
   Gets a string that is the types of batteries that can be created
   separated by ;; characters.  Mostly for internal use by the config.
**/
AREXPORT const char *Aria::batteryGetChoices(void)
{
  return ourBatteryChoices.c_str();
}

/**
   Creates a battery of a given type, with the given batteryNumber, and
   uses the logPrefix for logging what happens.  This is mostly for
   internal use by ArBatteryConnector.

   @param batteryType The type of battery to create

   @param batteryNumber The battery number to use for the created battery
   
   @param logPrefix The prefix to use when logging 
*/

AREXPORT ArBatteryMTX *Aria::batteryCreate(const char *batteryType, int batteryNumber,
				    const char *logPrefix)
{
  std::map<std::string, ArRetFunctor2<ArBatteryMTX *, int, const char *> *, ArStrCaseCmpOp>::iterator it;
  if ((it = ourBatteryCreatorMap.find(batteryType)) == ourBatteryCreatorMap.end())
  {
    ArLog::log(ArLog::Normal, "%sCannot create a battery of type %s options are <%s>", logPrefix, batteryType, batteryGetTypes());
    return NULL;
  }
  
  return (*it).second->invokeR(batteryNumber, logPrefix);
}


/**
   This adds a functor which can create a lcd of a given type. 

   @param lcdType The lcd type that the creator will make (these
   are always checked case insensitively).  If
   there is already a creator for this type, then the old one is
   replaced and the new one is used.

   @param creator A functor which takes an int (lcd number) and a
   const char * (logPrefix) and returns a new lcd of the lcdType
**/
AREXPORT bool Aria::lcdAddCreator(
	const char *lcdType, 
	ArRetFunctor2<ArLCDMTX *, int, const char *> *creator)
{
  if (ourLCDCreatorMap.find(lcdType) != ourLCDCreatorMap.end())
  {
    ArLog::log(ArLog::Normal, "Aria::lcdAddCreator: There is already a lcd creator for %s, replacing it", lcdType);
    ourLCDCreatorMap.erase(lcdType);
  }
  else
  {
    // if we haven't added any types add to the choices (there's an
    // intro string in choices, so it's checking the other variable)

    if (!ourLCDTypes.empty())
      ourLCDChoices += ";;";
    ourLCDChoices += lcdType;

    if (!ourLCDTypes.empty())
      ourLCDTypes += "|";
    ourLCDTypes += lcdType;

  }

  ourLCDCreatorMap[lcdType] = creator;
  return true;
}

/**
   Gets a string that is the types of batteries that can be created
   separated by | characters.  Mostly for internal use by ArLCDConnector.
**/
AREXPORT const char *Aria::lcdGetTypes(void)
{
  return ourLCDTypes.c_str();
}

/**
   Gets a string that is the types of batteries that can be created
   separated by ;; characters.  Mostly for internal use by ArLCDConnector.
**/
AREXPORT const char *Aria::lcdGetChoices(void)
{
  return ourLCDChoices.c_str();
}

/**
   Creates a lcd of a given type, with the given lcdNumber, and
   uses the logPrefix for logging what happens.  This is mostly for
   internal use by ArLCDConnector.

   @param lcdType The type of lcd to create

   @param lcdNumber The lcd number to use for the created lcd
   
   @param logPrefix The prefix to use when logging 
*/

AREXPORT ArLCDMTX *Aria::lcdCreate(const char *lcdType, int lcdNumber,
				    const char *logPrefix)
{
  std::map<std::string, ArRetFunctor2<ArLCDMTX *, int, const char *> *, ArStrCaseCmpOp>::iterator it;
  if ((it = ourLCDCreatorMap.find(lcdType)) == ourLCDCreatorMap.end())
  {
    ArLog::log(ArLog::Normal, "%sCannot create a lcd of type %s options are <%s>", logPrefix, lcdType, lcdGetTypes());
    return NULL;
  }
  
  return (*it).second->invokeR(lcdNumber, logPrefix);
}


/**
   This adds a functor which can create a sonar of a given type. 

   @param sonarType The sonar type that the creator will make (these
   are always checked case insensitively).  If
   there is already a creator for this type, then the old one is
   replaced and the new one is used.

   @param creator A functor which takes an int (sonar number) and a
   const char * (logPrefix) and returns a new sonar of the sonarType
**/
AREXPORT bool Aria::sonarAddCreator(
	const char *sonarType, 
	ArRetFunctor2<ArSonarMTX *, int, const char *> *creator)
{
  if (ourSonarCreatorMap.find(sonarType) != ourSonarCreatorMap.end())
  {
    ArLog::log(ArLog::Normal, "Aria::sonarAddCreator: There is already a sonar creator for %s, replacing it", sonarType);
    ourSonarCreatorMap.erase(sonarType);
  }
  else
  {
    // if we haven't added any types add to the choices (there's an
    // intro string in choices, so it's checking the other variable)
    if (!ourSonarTypes.empty())
      ourSonarChoices += ";;";
    ourSonarChoices += sonarType;

    if (!ourSonarTypes.empty())
      ourSonarTypes += "|";
    ourSonarTypes += sonarType;
  }

  ourSonarCreatorMap[sonarType] = creator;
  return true;
}

/**
   Gets a string that is the types of sonars that can be created
   separated by | characters.  Mostly for internal use by ArSonarConnector.
**/
AREXPORT const char *Aria::sonarGetTypes(void)
{
  return ourSonarTypes.c_str();
}

/**
   Gets a string that is the types of sonars that can be created
   separated by ;; characters.  Mostly for internal use by the config.
**/
AREXPORT const char *Aria::sonarGetChoices(void)
{
  return ourSonarChoices.c_str();
}

/**
   Creates a sonar of a given type, with the given sonarNumber, and
   uses the logPrefix for logging what happens.  This is mostly for
   internal use by ArSonarConnector.

   @param sonarType The type of sonar to create

   @param sonarNumber The sonar number to use for the created sonar
   
   @param logPrefix The prefix to use when logging 
*/

AREXPORT ArSonarMTX *Aria::sonarCreate(const char *sonarType, int sonarNumber,
				    const char *logPrefix)
{
  std::map<std::string, ArRetFunctor2<ArSonarMTX *, int, const char *> *, ArStrCaseCmpOp>::iterator it;
  if ((it = ourSonarCreatorMap.find(sonarType)) == ourSonarCreatorMap.end())
  {
    ArLog::log(ArLog::Normal, "%sCannot create a sonar of type %s options are <%s>", logPrefix, sonarType, sonarGetTypes());
    return NULL;
  }
  
  return (*it).second->invokeR(sonarNumber, logPrefix);
}


/**
   This adds a functor which can create a deviceConnection of a given
   type. 

   @param deviceConnectionType The deviceConnection type that the
   creator will make (these are always checked case insensitively).  If
   there is already a creator for this type, then the old one is
   replaced and the new one is used.

   @param creator A functor which takes a const char * (port name), a
   const char * (defaultInfo, for example default tcp port), a const
   char * (logPrefix) and returns a new deviceConnection of the
   deviceConnectionType if it can create the desired connection.  If
   the connection failes then it should return NULL.
**/
AREXPORT bool Aria::deviceConnectionAddCreator(
	const char *deviceConnectionType, 
	ArRetFunctor3<ArDeviceConnection *, const char *, const char *, const char *> *creator)
{
  if (ourDeviceConnectionCreatorMap.find(deviceConnectionType) != ourDeviceConnectionCreatorMap.end())
  {
    ArLog::log(ArLog::Normal, "Aria::deviceConnectionAddCreator: There is already a deviceConnection creator for %s, replacing it", deviceConnectionType);
    ourDeviceConnectionCreatorMap.erase(deviceConnectionType);
  }
  else
  {
    // if we haven't added any types add to the choices (there's an
    // intro string in choices, so it's checking the other variable)
    if (!ourDeviceConnectionTypes.empty())
      ourDeviceConnectionChoices += ";;";
    ourDeviceConnectionChoices += deviceConnectionType;

    if (!ourDeviceConnectionTypes.empty())
      ourDeviceConnectionTypes += "|";
    ourDeviceConnectionTypes += deviceConnectionType;
  }

  ourDeviceConnectionCreatorMap[deviceConnectionType] = creator;
  return true;
}

/**
   Gets a string that is the types of device connections that can be created
   separated by | characters.  Mostly for internal use by ArLaserConnector.
**/
AREXPORT const char *Aria::deviceConnectionGetTypes(void)
{
    return ourDeviceConnectionTypes.c_str();
}

/**
   Gets a string that is the types of device connections that can be created
   separated by ;; characters.  Mostly for internal use by the config.
**/
AREXPORT const char *Aria::deviceConnectionGetChoices(void)
{
    return ourDeviceConnectionChoices.c_str();
}

/** Creates a device connection of a given type, connecting to a given
    port, with the given defaultInfo, logging messages with the given
    prefix.  This is mostly for internal use by ArLaserConnector

   @param deviceConnectionType The type of device connection to create

   @param port The port to connect to (for serial, an example is COM1
   or /dev/ttyS0, for tcp an example is p3dx:8101)

   @param defaultInfo Default info to use, the only use for this right
   now is for a default port for a tcp connection (so that if it
   doesn't have hostname:port format but only hostname then it will
   use this port)
   
   @param logPrefix The prefix to use when logging 
*/
AREXPORT ArDeviceConnection *Aria::deviceConnectionCreate(
	const char *deviceConnectionType, const char *port, 
	const char *defaultInfo, const char *logPrefix)
{
  std::map<std::string, ArRetFunctor3<ArDeviceConnection *, const char *, const char *, const char *> *, ArStrCaseCmpOp>::iterator it;

  if ((it = ourDeviceConnectionCreatorMap.find(deviceConnectionType)) == ourDeviceConnectionCreatorMap.end())
  {
    ArLog::log(ArLog::Normal, "%sCannot create a deviceConnection of type %s options are <%s>", logPrefix, deviceConnectionType, deviceConnectionGetTypes());
    return NULL;
  }
  
  return (*it).second->invokeR(port, defaultInfo, logPrefix);
}


AREXPORT  void Aria::setMaxNumVideoDevices(size_t n) { ourMaxNumVideoDevices = n; }
AREXPORT  size_t Aria::getMaxNumVideoDevices() { return ourMaxNumVideoDevices; }
AREXPORT  void Aria::setMaxNumPTZs(size_t n) { ourMaxNumVideoDevices = n; }
AREXPORT  size_t Aria::getMaxNumPTZs() { return ourMaxNumVideoDevices; }

AREXPORT const char *Aria::getIdentifier(void)
{
  return ourIdentifier.c_str();
}

AREXPORT void Aria::setIdentifier(const char *identifier)
{
  ourIdentifier = identifier;

  // MPL fixing the problem caused by whitespace or bad chars in the
  // identifier (bug 14486).
  ArUtil::fixBadCharacters(&ourIdentifier, true);
}
