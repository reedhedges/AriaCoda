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

/** @example dataLoggerExample.cpp
    Show how to register data values with the ArDataLogger object.  The
    ArDataLogger will obtain a new set of values from functions or pointers you provide
    (with the return or variable type specified via template parameter)
    at a regular interval, format the value as text, and write it
    to a CSV, TSV or fixed-with text file row with a timestamp.  
    ArDataLogger also contains various built-in robot-related values by default.  

    ArDataLogger will always append new data to the log file, rather than
    rewriting it.  If configuration changes or logging is re-enabled after
    being disabled, a new header row is written to the file.

    All logging, and whether to log individual data values, can be enabled or 
    disabled in program configuration (via ArConfig), and the file name, file type, and 
    interval can be configured.  When used in an ArNetworking server, users can use 
    a client such as MobileEyes to modify this configuration.  This example 
    program will use a configuration file called dataLoggerExample.cfg if 
    present.   If not present, a new config file with defaults will be created.
    Edit this file to enable logging and enable individual values to log, and
    re-run the program.

    An ArNetworkingServer can also register these same values with the StringInfo 
    feature for display in MobileEyes or other clients.  Configuration can also
    be changed by users via ArNetworking, and some extra commands can be added such
    as clearing or saving a copy of the data log.  See the ArNetworking examples
    (and ARNL or MOGS examples, if you have ARNL or MOGS).  See the 
    detailed documentation of the ArDataLogger class in the ARIA API
    reference documenttation for more details including which data types 
    are supported.
*/

#include "Aria.h"
#include "ariaUtil.h"
#include "ArDataLogger.h"

/* An example function that returns an int */
int i = 0;
int getExampleInt() { 
  if(i >= 99999) i = 0;
  return i++;
}

/* An example function that returns a float */
float f = 2;
float getExampleFloat() {
  if(f > 99999) f = 0;
  f *= 2;
  return f;
}

/* An example function that returns an ArPose object. */
ArPose p;
ArPose getExamplePose() {
  return p;
}


/* You can get values from object methods as well */
class ExampleClass {
public:
  long getRandomNumber() {
    return ArMath::randomInRange(0, 100);
  }
};

int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser argparser(&argc, argv);
  argparser.loadDefaultArguments();
  ArRobot robot;

  ArDataLogger dataLogger(&robot, "dataLoggerExample.log");


  // Make it configurable via config  file or MobileEyes: 
  // To Use config file, you must get ArConfig via
  // Aria::getConfig() and load the file.  To use a config file and also be
  // configurable remotely via MobileEyes, also use ArServerHandlerConfig. See
  // ArNetworking examples and documentation.
  dataLogger.addToConfig(Aria::getConfig()); 

  dataLogger.addData<int>("Example Int", new ArGlobalRetFunctor<int>(&getExampleInt), "%d");
  dataLogger.addData<float>("Example Float", new ArGlobalRetFunctor<float>(&getExampleFloat), "%f");
  dataLogger.addData<ArPose>("Example Pose", new ArGlobalRetFunctor<ArPose>(&getExamplePose), "(%f %f %f)");
  ExampleClass exampleObj;
  dataLogger.addData<long>("Example Random Number", new ArRetFunctorC<long, ExampleClass>(&exampleObj, &ExampleClass::getRandomNumber), "%ld");

  ArRobotConnector robotConnector(&argparser, &robot);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "dataLoggerExample: Could not connect to the robot.");
    if(argparser.checkHelpAndWarnUnparsed())
    {
        // -help not given
        Aria::logOptions();
        Aria::exit(1);
    }
  }

  if (!Aria::parseArgs() || !argparser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(1);
  }

  ArLog::log(ArLog::Normal, "dataLoggerExample: Connected to robot.");


  // todo move this into a utility function:
  Aria::getConfig()->setBaseDirectory("./");
  if(Aria::getConfig()->parseFile("dataLoggerExample.cfg", true, true))
  {
    ArLog::log(ArLog::Normal, "dataLoggerExample: Loaded config file dataLoggerExample.cfg");
  }
  else
  {
    if(ArUtil::findFile("dataLoggerExample.cfg"))
    {
      ArLog::log(ArLog::Terse, "dataLoggerExample: Warning: Error loading config file dataLoggerExample.cfg.");
    }
    else
    {
      ArLog::log(ArLog::Normal, "dataLoggerExample: Creating new config file dataLoggerExample.cfg");
      Aria::getConfig()->writeFile("dataLoggerExample.cfg");
    }
  }
  

  // Logging can be enabled/disabled in the configuration.  Or we can always
  // start logging by calling startLogging:
  dataLogger.startLogging();
  ArLog::log(ArLog::Normal, "dataLoggerExample: Starting data logging to %s at %d sec interval", dataLogger.getLogFileName(), dataLogger.getLogInterval());

  // Start the robot processing cycle running in the background.
  // True parameter means that if the connection is lost, then the 
  // run loop ends.  The robot cycle will trigger dataLogger.
  robot.run(true);

  ArLog::log(ArLog::Normal, "dataLoggerExample: Exiting.");
  Aria::exit(0);
  return 0;
}

