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
#ifndef ARLOG_H
#define ARLOG_H

#ifndef WIN32
#include <stdio.h>
#endif
#include <string>
#include "ariaTypedefs.h"
#include "ArMutex.h"
#include "ArFunctor.h"

class ArConfig;

/// Logging utility class
/**
   ArLog is a utility class to log all messages from Aria to a choosen
   destintation. Messages can be logged to stdout, stderr, a file, and
   turned off completely. Logging by default is set to stdout. The level
   of logging can be changed as well. Allowed levels are Terse, Normal,
   and Verbose. By default the level is set to Normal.

   @ingroup ImportantClasses
   @ingroup easy
*/
class ArLog
{
public:

  typedef enum {
    StdOut, ///< Use stdout for logging
    StdErr, ///< Use stderr for logging
    File, ///< Use a file for logging
    Colbert, ///< @deprecated
    None ///< Disable logging
  } LogType;
  typedef enum {
    Terse, ///< Use terse logging
    Normal, ///< Use normal logging
    Verbose ///< Use verbose logging
  } LogLevel;

#ifndef SWIG
  /** @brief Log a message, with formatting and variable number of arguments
   *  @swignote In Java and Python, this function only takes one 
   *    string argument. Use Java or Python's native facities
   *    for constructing a formatted string, e.g. the % and + string
   *    operators in Python, and the methods of the Java String class.
   */
  AREXPORT static void log(LogLevel level, const char *str, ...);
#endif
  /// Log a message containing just a plain string
  AREXPORT static void logPlain(LogLevel level, const char *str);
  /// Initialize the logging utility with options
  AREXPORT static bool init(LogType type, LogLevel level,
			    const char *fileName="",
			    bool logTime = false, bool alsoPrint = false, 
			    bool printThisCall = true);
  /// Close the logging utility
  AREXPORT static void close();

  /// Logs an error, adding the error and string the error mean at the
  /// end of this message
  AREXPORT static void logErrorFromOS(LogLevel level, const char *str, ...);
  /// Logs an error, adding the error and string the error mean at the
  /// end of this message
  AREXPORT static void logErrorFromOSPlain(LogLevel level, const char *str);
#ifndef SWIG // these is internal we don't need to wrap it
  /// Logs an error, adding the error and string the error mean at the
  /// end of this message... internal version, don't use it
  AREXPORT static void logErrorFromOSNoLock(LogLevel level, const char *str, ...);
  /// Logs an error, adding the error and string the error mean at the
  /// end of this message... internal version, dont' use it
  AREXPORT static void logErrorFromOSPlainNoLock(LogLevel level, const char *str);
  // Do not use this unless you know what you are doing...
  /** @internal
   * @swigomit */
  AREXPORT static void logNoLock(LogLevel level, const char *str, ...);
#endif 
  /// Log function call backtrace for debugging 
  /// @linuxonly
  AREXPORT static void logBacktrace(LogLevel level);
  /// Read the contents of @a fileName and print a log message for each line. File should be plain text.
  AREXPORT static bool logFileContents(LogLevel level, const char *fileName);

  /// Use an ArConfig object to control ArLog's options
  AREXPORT static void addToConfig(ArConfig *config);

  /// Set log level
  AREXPORT static void setLogLevel(LogLevel level);
  
  /// Set a functor to be called when a log message is made 
  /// Call clearFunctor() to unset.
  AREXPORT static void setFunctor(ArFunctor1<const char *> *functor);
  /// Clear functor set by setFunctor().
  AREXPORT static void clearFunctor();

#ifndef SWIG
  /// Internal function to force a lockup, only for debugging
  /// @internal
  static void internalForceLockup(void);
#endif

  /// Convenience function to log a message at Terse log level with "Warning: " prepended
  AREXPORT static void warning(const char *str, ...);
  /// Convenience function to log a message at Terse log level with "Error: " prepended
  AREXPORT static void error(const char *str, ...);
  /// Convenience function to log a message at Normal log level 
  AREXPORT static void info(const char *str, ...);
  /// Convenience function to log a message at Verbose log level 
  AREXPORT static void debug(const char *str, ...);

#ifndef SWIG
  /// @internal
  AREXPORT static void log_v(LogLevel level, const char *prefix, const char *format, va_list vaptr);
#endif


  AREXPORT static unsigned long getAvailableDiskSpaceMB();

protected:
  static bool processFile(void);
  static void invokeFunctor(const char *message);
  static void checkFileSize(void);

  static ArLog *ourLog;
  static ArMutex ourMutex;
  static LogType ourType;
  static LogLevel ourLevel;
  static bool ourLoggingTime;
  static FILE *ourFP;
  static std::string ourFileName;
  static bool ourAlsoPrint;
  static int ourCharsLogged;
  
  static LogType ourConfigLogType;
  static LogLevel ourConfigLogLevel;
  static char ourConfigFileName[1024];
  static bool ourConfigLogTime;
  static bool ourConfigAlsoPrint;
  static ArGlobalRetFunctor<bool> ourConfigProcessFileCB;
  
  static ArFunctor1<const char *> *ourFunctor;

};


#endif
