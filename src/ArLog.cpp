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
#include "Aria/ariaOSDef.h"
#include "Aria/ArLog.h"
#include "Aria/ArConfig.h"
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "Aria/ariaInternal.h"


#ifdef WIN32
#include <io.h>
#else
#include <fcntl.h>
#include <errno.h>
#include <execinfo.h>
#endif

#if defined(_ATL_VER) || defined(ARIA_MSVC_ATL_VER)
#include <atlbase.h>
#define HAVEATL 1
#endif // ifdef _ATL_VER

ArMutex ArLog::ourMutex;
ArLog::LogType ArLog::ourType=ArLog::DefaultLogType;
ArLog::LogLevel ArLog::ourLevel=ArLog::Normal;
FILE * ArLog::ourFP=0;
long ArLog::ourCharsLogged = 0;
std::string ArLog::ourFileName;
bool ArLog::ourLoggingTime = false;
bool ArLog::ourAlsoPrint = false;

ArLog::LogType ArLog::ourConfigLogType = ArLog::DefaultLogType;
ArLog::LogLevel ArLog::ourConfigLogLevel = ArLog::Normal;
char ArLog::ourConfigFileName[1024] = "log.txt";
bool ArLog::ourConfigLogTime = false;
bool ArLog::ourConfigAlsoPrint = false;
ArGlobalRetFunctor<bool> ArLog::ourConfigProcessFileCB(&ArLog::processFile);


ArFunctor1<const char *> *ArLog::ourFunctor;


AREXPORT void ArLog::logPlain(LogLevel level, const char *str)
{
  log(level, str);
}

/**
   This function is used like printf(). If the supplied level is less than
   or equal to the set level, it will be printed.
   @param level level of logging
   @param str printf() like formating string
*/
AREXPORT void ArLog::log(LogLevel level, const char *str, ...)
{
  if (level > ourLevel)
    return;
  
  //printf("logging %s\n", str);

  char buf[10000];
  char *bufPtr;
  char *timeStr;
  const size_t timeLen = 26; // max size of string returned by ctime


  ourMutex.lock();
  // put our time in if we want it
  if (ourLoggingTime)
  {
    time_t now = time(NULL);
    timeStr = ctime(&now);
    // get take just the portion of the time we want
    strncpy(buf, timeStr, timeLen);
    buf[timeLen] = '\0';
    bufPtr = &buf[timeLen];
  }
  else
    bufPtr = buf;
  va_list ptr;
  va_start(ptr, str);
  
  vsnprintf(bufPtr, sizeof(buf) - timeLen - 2, str, ptr);
  bufPtr[sizeof(buf) - timeLen - 1] = '\0';
  //vsprintf(bufPtr, str, ptr);
  // can do whatever you want with the buf now
  if (ourFP)
  {
    int written;
    if ((written = fprintf(ourFP, "%s\n", buf)) > 0)
      ourCharsLogged += written;
    fflush(ourFP);
    if(ourType == File) checkFileSize();
  }
  else if (ourType != None)
  {
    printf("%s\n", buf);
    fflush(stdout);
  }
  if (ourAlsoPrint)
    printf("%s\n", buf);

  invokeFunctor(buf);


// Also send it to the VC++ debug output window...
#ifdef HAVEATL
  ATLTRACE2("%s\n", buf);
#endif
  
  va_end(ptr);
  ourMutex.unlock();
}

/**
   This function appends errorno in linux, or getLastError in windows,
   and a string indicating the problem.
   
   @param level level of logging
   @param str printf() like formating string
*/
AREXPORT void ArLog::logErrorFromOSPlain(LogLevel level, const char *str)
{
  logErrorFromOS(level, str);
}

/**
   This function appends errorno in linux, or getLastError in windows,
   and a string indicating the problem.
   
   @param level level of logging
   @param str printf() like formating string
*/
AREXPORT void ArLog::logErrorFromOS(LogLevel level, const char *str, ...)
{
  if (level > ourLevel)
    return;

#ifndef _WIN32
  int err = errno;
#else
  DWORD err = GetLastError();
#endif 

  //printf("logging %s\n", str);

  char buf[10000];
  char *bufPtr;
  size_t timeLen = 0; 


  ourMutex.lock();
  // put our time in if we want it
  if (ourLoggingTime)
  {
    const time_t now = time(NULL);
    char *timeStr = ctime(&now);
    timeLen = 20; // this is a value based on the standard length of
                  // ctime return

    // get take just the portion of the time we want
    strncpy(buf, timeStr, timeLen);
    buf[timeLen] = '\0';
    bufPtr = &buf[timeLen];
  }
  else
  {
    bufPtr = buf;
    timeLen = 0;
  }
  va_list ptr;
  va_start(ptr, str);
  
  vsnprintf(bufPtr, sizeof(buf) - timeLen - 2, str, ptr);
  bufPtr[sizeof(buf) - timeLen - 1] = '\0';


  char bufWithError[10200];  

#ifndef _WIN32
  const char *errorString = strerror(err);
  //if (err < sys_nerr - 1)
  //  errorString = sys_errlist[err];
  snprintf(bufWithError, sizeof(bufWithError) - 1, "%s | ErrorFromOSNum: %d ErrorFromOSString: %s", buf, err, errorString);
  bufWithError[sizeof(bufWithError) - 1] = '\0';
#else
  LPVOID errorString = NULL;

  FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &errorString,
        0, NULL);
 
  snprintf(bufWithError, sizeof(bufWithError) - 1, "%s | ErrorFromOSNum: %d ErrorFromOSString: %s", buf, err, (char*)errorString);
  bufWithError[sizeof(bufWithError) - 1] = '\0';

  LocalFree(errorString);
#endif

  //vsprintf(bufPtr, str, ptr);
  // can do whatever you want with the buf now
  if (ourFP)
  {
    int written;
    if ((written = fprintf(ourFP, "%s\n", bufWithError)) > 0)
      ourCharsLogged += written;
    fflush(ourFP);
    if(ourType == File) checkFileSize();
  }
  else if (ourType != None)
  {
    printf("%s\n", bufWithError);
    fflush(stdout);
  }
  if (ourAlsoPrint)
    printf("%s\n", bufWithError);

  invokeFunctor(bufWithError);


// Also send it to the VC++ debug output window...
#ifdef HAVEATL
  ATLTRACE2("%s\n", buf);
#endif
  
  va_end(ptr);
  ourMutex.unlock();
}


/**
   This function appends errorno in linux, or getLastError in windows,
   and a string indicating the problem.
   
   @param level level of logging
   @param str printf() like formating string
*/
AREXPORT void ArLog::logErrorFromOSPlainNoLock(LogLevel level, const char *str)
{
  logErrorFromOSNoLock(level, str);
}

/**
   This function appends errorno in linux, or getLastError in windows,
   and a string indicating the problem.
   
   @param level level of logging
   @param str printf() like formating string
*/
AREXPORT void ArLog::logErrorFromOSNoLock(LogLevel level, const char *str, ...)
{
  if (level > ourLevel)
    return;

#ifndef _WIN32
  int err = errno;
#else
  DWORD err = GetLastError();
#endif 

  //printf("logging %s\n", str);

  char buf[10000];
  char *bufPtr;
  size_t timeLen = 0; 

  // put our time in if we want it
  if (ourLoggingTime)
  {
    const time_t now = time(NULL);
    char* timeStr = ctime(&now);
    timeLen = 20;      // this is a value based on the standard length of
                       // ctime return

    // get take just the portion of the time we want
    strncpy(buf, timeStr, timeLen);
    buf[timeLen] = '\0';
    bufPtr = &buf[timeLen];
  }
  else
  {
    bufPtr = buf;
    timeLen = 0;
  }

  va_list ptr;
  va_start(ptr, str);

  vsnprintf(bufPtr, sizeof(buf) - timeLen - 2, str, ptr);
  bufPtr[sizeof(buf) - timeLen - 1] = '\0';

  char bufWithError[10200];  

#ifndef _WIN32
  const char *errorString = strerror(err);
//  if (err < sys_nerr - 1)
//    errorString = sys_errlist[err];
  snprintf(bufWithError, sizeof(bufWithError) - 1, "%s | ErrorFromOSNum: %d ErrorFromOSString: %s", buf, err, errorString);
  bufWithError[sizeof(bufWithError) - 1] = '\0';
#else
  LPVOID errorString = NULL;

  FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &errorString,
        0, NULL);
 
  snprintf(bufWithError, sizeof(bufWithError) - 1, "%s | ErrorFromOSNum: %d ErrorFromOSString: %s", buf, err, (LPSTR)errorString);
  bufWithError[sizeof(bufWithError) - 1] = '\0';

  LocalFree(errorString);
#endif

  //vsprintf(bufPtr, str, ptr);
  // can do whatever you want with the buf now
  if (ourFP)
  {
    int written;
    if ((written = fprintf(ourFP, "%s\n", bufWithError)) > 0)
      ourCharsLogged += written;
    fflush(ourFP);
    if(ourType == File) checkFileSize();
  }
  else if (ourType != None)
  {
    printf("%s\n", bufWithError);
    fflush(stdout);
  }
  if (ourAlsoPrint)
    printf("%s\n", bufWithError);

  invokeFunctor(bufWithError);


// Also send it to the VC++ debug output window...
#ifdef HAVEATL
  ATLTRACE2("%s\n", buf);
#endif
  
  va_end(ptr);
}

/**
   Change logging settings from defaults, and check for external configuration
   such as environment variable.
   @param type Destination type of log messages. 
   @param level level of logging.
   @param fileName the name of the file for File type of logging. Must be provided if @a type is File.
   @param logTime if this is true then the time a message is given will be logged
   @param alsoPrint if this is true then in addition to whatever other logging (to a file for instance) the results will also be printed to stdout
   @param printThisCall if this is true the new settings will be printed otherwise they won't
   @bug Crashes, most programs don't need this but should fix.
*/
AREXPORT bool ArLog::init(LogType type, LogLevel level, const char *fileName,
			  bool logTime, bool alsoPrint, bool printThisCall)
{
  ourMutex.setLogName("ArLog::ourMutex");

  ourMutex.lock();
  
  // if we weren't or won't be doing a file then close any old file
  if (ourType != File || type != File)
  {
    close();
  }
  
  if (type == StdOut)
    ourFP=stdout;
  else if (type == StdErr)
    ourFP=stderr;
  else if (type == File)
  {
    if (fileName != NULL)
    {
      if (strcmp(ourFileName.c_str(), fileName) == 0)
      {
	ArLog::logNoLock(ArLog::Terse, "ArLog::init: Continuing to log to the same file.");
      }
      else
      {
	close();
	if ((ourFP = ArUtil::fopen(fileName, "w")) == NULL)
	{
	  ArLog::logNoLock(ArLog::Terse, "ArLog::init: Could not open file %s for logging.", fileName);
	  ourMutex.unlock();
	  return(false);
	}
	ourFileName=fileName;
      }
    }
  }
  else if (type == None)
  {

  }
  ourType=type;
  ourLevel=level;

  // environment variables to override log level 
  {
    char* lev = getenv("ARLOG_LEVEL");
    if(lev)
    {
      switch(toupper(lev[0]))
      {
        case 'N':
          ourLevel = Normal;
          break;
        case 'T':
          ourLevel = Terse;
          break;
        case 'V':
          ourLevel = Verbose;
          break;
       }
       ArLog::log(ArLog::Normal, "ArLog: Set log level to %s from ARLOG_LEVEL environment variable.", logLevelName(ourLevel).c_str());
    }
  }

  ourLoggingTime = logTime;
  ourAlsoPrint = alsoPrint;

  if(getenv("ARLOG_TIME") != NULL)
  {
    ourLoggingTime = true;
    ArLog::log(ArLog::Normal, "ArLog: Enabled log timestamps from ARLOG_TIME environment variable.");
  }

  if (printThisCall)
  {
    printf("ArLog::init: %s\t %s\t", logTypeName(ourType).c_str(), logLevelName(ourLevel).c_str());

    if (ourLoggingTime)
      printf(" Logging Time\t");
    else
      printf(" Not logging time\t");

    if (ourAlsoPrint)
      printf(" Also printing\n");
    else
      printf(" Not also printing\n");
  }
  ourMutex.unlock();
  return(true);
}


std::string ArLog::logTypeName(ArLog::LogType type)
{
  switch(type)
  {
    case StdOut:
      return "StdOut";
    case StdErr:
      return "StdErr";
    case File:
      return std::string{"File("} + ourFileName + ")";
    case None:
      return "None";
  }
  assert(false);
  return "BadType";
}

std::string ArLog::logLevelName(ArLog::LogLevel level)
{
  switch(level)
  {
    
    case Terse:
      return "Terse";
    case Normal:
      return "Normal";
    case Verbose:
      return "Verbose";
  }
  assert(false);
  return "BadLevel";
}


AREXPORT void ArLog::close()
{
  // if logging to File and have a valid FP, close it.
  // don't try closing stderr or stdout when ourType is not File.
  if (ourFP && (ourType == File))
  {
    fclose(ourFP);
    ourFP=0;
    ourFileName="";
  }
}

AREXPORT void ArLog::beginWrite(LogLevel level)
{
  ourMutex.lock();
  if(level > ourLevel)
  {
    return;
  }
  if(ourLoggingTime)
  {
    time_t now = time(NULL);
    char *timeStr = ctime(&now);
    timeStr[strlen(timeStr)-1] = ' '; // replace newline with space
    int r = 0;
    if(ourFP)
      r = fputs(timeStr, ourFP);
    else if(ourType != None)
      r = fputs(timeStr, stdout);
    if(r > 0)
      ourCharsLogged += r;
    if(ourAlsoPrint)
      fputs(timeStr, stdout);
  }
}


AREXPORT void ArLog::write(LogLevel level, const char *str, ...)
{
  if(level > ourLevel)
    return;
  va_list args;
  va_start(args, str);
  int r = 0;
  if(ourFP)
    r = vfprintf(ourFP, str, args);
  else if(ourType != None)
    vprintf(str, args);
  if(ourAlsoPrint)
    vprintf(str, args);
  va_end(args);
  if(r > 0)
    ourCharsLogged += r;
}


AREXPORT void ArLog::endWrite()
{
  if(ourFP)
  {
    int r = fputc('\n', ourFP);
    if(r > 0)
      ourCharsLogged += r;
    fflush(ourFP);
    if(ourType == File) checkFileSize();
  }
  else if(ourType != None)
    putchar('\n');

  if(ourAlsoPrint)
    putchar('\n');

  // XXX bug: we are not invoking user supplied functor with a string of line written!

  ourMutex.unlock();
}



AREXPORT void ArLog::logNoLock(LogLevel level, const char *str, ...)
{
  if (level > ourLevel)
    return;

  char buf[2048];
  char *bufPtr;

  
  // put our time in if we want it
  if (ourLoggingTime)
  {
    const time_t now = time(NULL);
    char *timeStr = ctime(&now);
    size_t timeLen = 20; // this is a value based on the standard length of ctime return
    // get take just the portion of the time we want
    strncpy(buf, timeStr, timeLen);
    buf[timeLen] = '\0';
    bufPtr = &buf[timeLen];
  }
  else
  {
    bufPtr = buf;
    //timeLen = 0;
  }
  va_list ptr;
  va_start(ptr, str);
  //vsnprintf(bufPtr, sizeof(buf) - timeLen - 1, str, ptr);
  vsprintf(bufPtr, str, ptr);
  // can do whatever you want with the buf now
  if (ourFP)
  {
    int written;
    if ((written = fprintf(ourFP, "%s\n", buf)) > 0)
      ourCharsLogged += written;
    fflush(ourFP);
    if (ourType == File)
      checkFileSize();
  }
  else if (ourType != None)
    printf("%s\n", buf);
  if (ourAlsoPrint)
    printf("%s\n", buf);
  
  invokeFunctor(buf);

  va_end(ptr);
}

AREXPORT void ArLog::logBacktrace(LogLevel level)
{
#ifndef _WIN32
  const int size = 100;
  int numEntries;
  void *buffer[size];
  char **names;
  
  numEntries = backtrace(buffer, size);
  ArLog::log(ArLog::Normal, "Backtrace %d levels", numEntries);
  
  names = backtrace_symbols(buffer, numEntries);
  if (names == NULL)
    return;
  
  int i;
  for (i = 0; i < numEntries; i++)
    ArLog::log(level, "%s", names[i]);
  
  free(names);
#endif
}

/// Log a file if it exists
AREXPORT bool ArLog::logFileContents(LogLevel level, const char *fileName)
{
  FILE *strFile;
  unsigned int i;
  char str[100000];
  
  str[0] = '\0';
  
  if ((strFile = ArUtil::fopen(fileName, "r")) != NULL)
  {
    while (fgets(str, sizeof(str), strFile) != NULL)
    {
      bool endedLine = false;
      for (i = 0; i < sizeof(str) && !endedLine; i++)
      {
	if (str[i] == '\r' || str[i] == '\n' || str[i] == '\0')
	{
	  str[i] = '\0';
	  ArLog::log(level, str);
	  endedLine = true;
	}
      }
    }
    fclose(strFile);
    return true;
  }
  else
  {
    return false;
  }
}

AREXPORT void ArLog::addToConfig(ArConfig *config)
{
  std::string section = "LogConfig";
  config->addParam(
	  ArConfigArg("LogType", (int *)&ourConfigLogType,
		      "The type of log we'll be using, 0 for StdOut, 1 for StdErr, 2 for File (and give it a file name), 4 for None", 
		      ArLog::StdOut, ArLog::None), 
	  section.c_str(), ArPriority::TRIVIAL);
  config->addParam(
	  ArConfigArg("LogLevel", (int *)&ourConfigLogLevel,
		      "The level of logging to do, 0 for Terse, 1 for Normal, and 2 for Verbose", 
		      ArLog::Terse, ArLog::Verbose), 
	  section.c_str(), ArPriority::TRIVIAL);
  config->addParam(
	  ArConfigArg("LogFileName", ourConfigFileName,
		      "File to log to", sizeof(ourConfigFileName)),
	  section.c_str(), ArPriority::TRIVIAL);
  config->addParam(
	  ArConfigArg("LogTime", &ourConfigLogTime,
		      "True to prefix log messages with time and date, false not to"),
	  section.c_str(), ArPriority::TRIVIAL);
  config->addParam(
	  ArConfigArg("LogAlsoPrint", &ourConfigAlsoPrint,
		      "If true, also print messages as program output when logging to a file.s"),	  
	  section.c_str(), ArPriority::TRIVIAL);
  ourConfigProcessFileCB.setName("ArLog");
  config->addProcessFileCB(&ourConfigProcessFileCB, 200);
}

bool ArLog::processFile()
{
  if (ourConfigLogType != ourType || ourConfigLogLevel != ourLevel ||
      strcmp(ourConfigFileName, ourFileName.c_str()) != 0 || 
      ourConfigLogTime != ourLoggingTime || ourConfigAlsoPrint != ourAlsoPrint)
  {
    ArLog::logNoLock(ArLog::Normal, "Initializing log from config");
    return ArLog::init(ourConfigLogType, ourConfigLogLevel, ourConfigFileName, 
		       ourConfigLogTime, ourConfigAlsoPrint, true);
  }
  return true;
}

AREXPORT void ArLog::setFunctor(ArFunctor1<const char *> *functor)
{
  ourFunctor = functor;
}

AREXPORT void ArLog::clearFunctor()
{
  ourFunctor = NULL;
}

void ArLog::invokeFunctor(const char *message)
{
  ArFunctor1<const char *> *functor;
  functor = ourFunctor;
  if (functor != NULL)
    functor->invoke(message);
}

void ArLog::checkFileSize()
{
  if(ourType != File) return;
  const long size = sizeFile(ourFileName);
  if(size < 0)
  {
    ArLog::log(ArLog::Normal, "Warning: Error checking current size of log file %s", ourFileName.c_str());
    return;
  }
  if (size > ourCharsLogged)
  {
    ourCharsLogged = size;
  }
}

long ArLog::sizeFile(const std::string& filename)
{
  struct stat buf;
  if(ArUtil::filestat(filename, &buf) < 0)
  {
    return -1;
  }
#ifdef WIN32
  if(!(buf.st_mode | _S_IFREG))
    return -1;
#else
  if(!S_ISREG(buf.st_mode))
    return -1;
#endif
  return buf.st_size;
}

void ArLog::internalForceLockup()
{
  ArLog::log(ArLog::Terse, "ArLog: forcing internal lockup");
  ourMutex.lock();
}

AREXPORT void ArLog::log_v(LogLevel level, const char *prefix, const char *str, va_list ptr)
{
  char buf[1024];
  strncpy(buf, prefix, sizeof(buf)-1);
  const size_t prefixSize = strlen(prefix);
  vsnprintf(buf+prefixSize, sizeof(buf)-prefixSize-1, str, ptr);
  buf[sizeof(buf) - 1] = '\0';
  logNoLock(level, buf);
}


AREXPORT void ArLog::info(const char *str, ...)
{
  ourMutex.lock();
  va_list ptr;
  va_start(ptr, str);
  log_v(Normal, "", str, ptr);
  va_end(ptr);
  ourMutex.unlock();
}

AREXPORT void ArLog::warning(const char *str, ...)
{
  ourMutex.lock();
  va_list ptr;
  va_start(ptr, str);
  log_v(Terse, "Warning: ", str, ptr);
  va_end(ptr);
  ourMutex.unlock();
}

AREXPORT void ArLog::error(const char *str, ...)
{
  ourMutex.lock();
  va_list ptr;
  va_start(ptr, str);
  log_v(Terse, "Error: ", str, ptr);
  va_end(ptr);
  ourMutex.unlock();
}

AREXPORT void ArLog::debug(const char *str, ...)
{
  ourMutex.lock();
  va_list ptr;
  va_start(ptr, str);
  log_v(Terse, "[debug] ", str, ptr);
  va_end(ptr);
  ourMutex.unlock();
}

AREXPORT void ArLog::setLogLevel(LogLevel level) {
	ourMutex.lock();
	ourLevel = level;
	ourMutex.unlock();
}

AREXPORT unsigned long ArLog::getAvailableDiskSpaceMB() 
{
  if(ourType == File)
    return ArUtil::availableDiskSpaceMB(ourFileName.c_str());
  else
    return ULONG_MAX;
}

