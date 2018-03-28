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
#ifndef ARDATALOGGER_H
#define ARDATALOGGER_H

#include "ariaUtil.h"
#include "ArMutex.h"
#include "ArFunctor.h"
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

class ArRobot;
class ArConfig;


/// This class will log data, but you have to use it through an ArConfig right now
/**
   This class doesn't log anything by default, but can be set up to
   log all sorts of data.  Note that if you do an addString after you
   do an addToConfig it'll automatically be enabled (since right now
   we don't want to change config after loading since the values would
   wind up wierd).

  @ingroup OptionalClasses
 **/
class ArDataLogger
{
public:
  /// Constructor
  AREXPORT ArDataLogger(ArRobot *robot, const char *fileName = NULL);
  /// Destructor
  AREXPORT ~ArDataLogger();
  /// Adds the data logger information to the config
  AREXPORT void addToConfig(ArConfig *config);
  /// Adds a string to the list of options in the raw format
  AREXPORT void addString(const char *name, ArTypes::UByte2 maxLen, 
			  ArFunctor2<char *, ArTypes::UByte2> *functor);

  /// Add data to be logged.   T is the data type to log.  This can be any type
  /// supported by ArStringFormatBufFunctor  (including any specializations) using the appropriate format string in
  /// @a format.
  /// @param name Name
  /// @param unitDesc String describing units or other attribute of data. This is appended to column header in data log file, and may be displayed after the value when showing in a user interface.
  /// @param accessor Functor to invoke to obtain the data.
  /// @param format Format string used with snprintf to format data when writing to the data log file.
  /// @param maxStrLen Maximum buffer size to store formatted value
  template<typename T> void addData(const char *name, const char *unitDesc, ArRetFunctor<T> *accessor, const char *format, size_t maxStrLen = 32)
  {
    std::stringstream ns;
    ns << name << " (" << unitDesc << ")";
    addString(ns.str().c_str(), maxStrLen, new ArStringFormatBufFunctor<T, ArTypes::UByte2>(accessor, format));
  }

  /// @copydoc addData()
  template<typename T> void addData(const char *name, ArRetFunctor<T> *accessor, const char *format, size_t maxStrLen = 32)
  {
    addString(name, maxStrLen, new ArStringFormatBufFunctor<T, ArTypes::UByte2>(accessor, format));
  }

  /// Gets the functor for adding a string (for ArStringInfoGroup)
  ArFunctor3<const char *, ArTypes::UByte2,
				    ArFunctor2<char *, ArTypes::UByte2> *> *
                     getAddStringFunctor(void) { return &myAddStringFunctor; }

  AREXPORT void startLogging(int interval);

  void startLogging()
  {
    startLogging(-1);
  }

  AREXPORT void stopLogging();
 
  bool getLogging() { return myConfigLogging; }

  int getLogInterval() { return myConfigLogInterval; }

  /// Write a comment line to the log file. (A line prefixed with ; and a
  /// timestamp, folowed by @a str). If no log file is opened, do nothing.
  AREXPORT void writeComment(const char *str);

  /// Write a comment line to the log file. (A line prefixed with ; and a
  /// timestamp, folowed by @a str). If no log file is opened, do nothing.
  AREXPORT void writeComment(ArArgumentBuilder* ab);

  ArFunctor1<ArArgumentBuilder*> *getWriteCommentFunctor() { return &myWriteCommentFunctor; }

  /// Clear the log file -- all existing data will be lost!
  AREXPORT void clearLog();
  
  ArFunctor *getClearLogFunctor() { return &myClearLogFunctor; }

  ArFunctor *getStartLogFunctor() { return &myStartLogFunctor; }
  ArFunctor *getStopLogFunctor() { return &myStopLogFunctor; }

  AREXPORT std::string getStatus();
  AREXPORT void getStatus(char *buf, ArTypes::UByte2 buflen);

  AREXPORT void saveCopyAs(const char *name);
  void saveCopy() { saveCopyAs(NULL); }
  AREXPORT void saveCopyArgs(ArArgumentBuilder *ab);
  ArFunctor1<ArArgumentBuilder*> *getSaveCopyWithArgsFunctor() { return &mySaveCopyWithArgsFunctor; }
  ArFunctor *getSaveCopyFunctor() { return &mySaveCopyFunctor; }

  const char *getOpenLogFileName()
  {
    if(myFile)
      return myOpenedFileName;
    else
      return NULL;
  }

  const char *getLogFileName() { return getOpenLogFileName(); }

  unsigned long getAvailableDiskSpaceMB() {
    return ArUtil::availableDiskSpaceMB(getLogFileName());
  }

protected:
  void connectCallback(void);
  bool processFile(char *errorBuffer, size_t errorBufferLen);
  void writeHeader();
  void userTask(void);
  ArRobot *myRobot;
  ArTime myLastLogged;
  ArConfig *myConfig;
  bool myAddToConfigAtConnect;
  bool myAddedToConfig;

  FILE *myFile;
  bool myConfigLogging;
  int myConfigLogInterval;
  char myOpenedFileName[512];
  char myConfigFileName[512];
  std::string myPermanentFileName;

  // for what we're logging
  bool myLogVoltage;
  bool myLogStateOfCharge;
  bool myLogLeftVel;
  bool myLogRightVel;
  bool myLogTransVel;
  bool myLogRotVel;
  bool myLogLatVel;
  bool myLogLeftStalled;
  bool myLogRightStalled;
  bool myLogStallBits;
  bool myLogFlags;
  bool myLogFaultFlags;
  int myAnalogCount;
  bool *myAnalogEnabled;
  int myAnalogVoltageCount;
  bool *myAnalogVoltageEnabled;
  int myDigInCount;
  bool *myDigInEnabled;
  int myDigOutCount;
  bool *myDigOutEnabled;
  bool myLogPose;
  bool myLogEncoderPose;
  bool myLogCorrectedEncoderPose;
  bool myLogEncoders;
  bool myLogChargeState;
  bool myLogBatteryInfo;
  bool myLogMovementSent;
  int myStringsCount;
  std::vector<bool *> myStringsEnabled;
  std::vector<bool> myStringsAddedToConfig;

  ArMutex myMutex;

  std::vector<ArStringInfoHolder *> myStrings;
  ArTypes::UByte2 myMaxMaxLength;
  ArFunctor3C<ArDataLogger, const char *, ArTypes::UByte2,
		    ArFunctor2<char *, ArTypes::UByte2> *> myAddStringFunctor;

  ArFunctor1C<ArDataLogger, ArArgumentBuilder*> myWriteCommentFunctor;
  

  ArFunctorC<ArDataLogger> myConnectCB;  
  ArRetFunctor2C<bool, ArDataLogger, char *, size_t> myProcessFileCB;
  ArFunctorC<ArDataLogger> myUserTaskCB;


  ArFunctor2C<ArDataLogger, unsigned char, short int> myCommandMonitorCB;
  void robotCommandMonitor(unsigned char, short int);

  struct Cmd {
    unsigned char cmd;
    std::string name;
    bool sent;
    short int arg;
    Cmd(unsigned char _cmd = -1, const std::string& _name = "") : cmd(_cmd), name(_name), sent(false), arg(-1) {}
  };

  typedef std::map<unsigned char, Cmd> CmdMap;
  CmdMap myMovementCommands;
  ArMutex myMovementCommandsMutex;

  ArFunctorC<ArDataLogger> myClearLogFunctor;
  ArFunctorC<ArDataLogger> myStartLogFunctor;
  ArFunctorC<ArDataLogger> myStopLogFunctor;
  ArFunctor1C<ArDataLogger, ArArgumentBuilder*> mySaveCopyWithArgsFunctor;
  ArFunctorC<ArDataLogger> mySaveCopyFunctor;

  std::string myConfigLogFormat;
  enum { TSV, CSV, Fixed } myLogFormat;
  char myLogSep;
};

#endif // ARDATALOGGER_H
