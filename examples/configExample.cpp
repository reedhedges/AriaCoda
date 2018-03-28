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

/** @example configExample.cpp Example program demonstrating the use of ArConfig
 *
 *  This program shows how to use ArConfig to store configuration parameters
 *  and load/save them from a file.  
 *
 *  The ArNetworking library includes server classes that will let you
 *  use a remote client such as MobileEyes to view and change the configuration.
 *  See ArNetworking documentation and examples.
 */

class ConfigExample
{
  ArConfig* myConfig;
  int myIntParam;
  double myDoubleParam;
  bool myBoolParam;
  char myStringParam[256];
  ArRetFunctorC<bool, ConfigExample> myProcessConfigCB;

public:
  ConfigExample():
    myIntParam(0),
    myDoubleParam(0.5),
    myBoolParam(false),
    myProcessConfigCB(this, &ConfigExample::processConfigFile)
  {
    // The global Aria class contains an ArConfig object.  You can create
    // other instances of ArConfig, but this is how you can share one ArConfig
    // among various program modules.
    // If you want to store a config parameter in ArConfig, first you must add 
    // it to the ArConfig object.  Parameters are stored in sections, and
    // they affect a variable via a pointer provided in an ArConfigArg
    // object:
    ArConfig* config = Aria::getConfig();
    config->setSectionComment("Example Section", "Contains parameters created by the configExample");

    // Add an integer which ranges from -10 to 10:
    config->addParam( ArConfigArg("ExampleIntegerParameter", &myIntParam, "Example parameter integer.", -10, 10), "Example Section", ArPriority::NORMAL);
    
    // Add a floating point number which ranges from 0.0 to 1.0:
    config->addParam( ArConfigArg("ExampleDoubleParameter", &myDoubleParam, "Example double precision floating point number.", 0.0, 1.0), "Example Section", ArPriority::NORMAL);

    // Essential parameters can be placed in the "Important" priority level:
    config->addParam( ArConfigArg("ExampleBoolParameter", &myBoolParam, "Example boolean parameter."), "Example Section", ArPriority::IMPORTANT);

    // Unimportant parameters can be placed in the "Trivial" priority level:
    myStringParam[0] = '\0';  // make string empty
    config->addParam( ArConfigArg("ExampleStringParameter", myStringParam, "Example string parameter.", 256), "Example Section", ArPriority::TRIVIAL);

    // You can set a callback to be invoked when the configuration changes, in
    // case you need to respond to any changes in the parameter values:
    config->addProcessFileCB(&myProcessConfigCB, 0);
  }

  
  // Method called by config process callback when a new file is loaded.
  // It can return false to indicate an error, or true to indicate no error.
  bool processConfigFile() 
  {
    ArLog::log(ArLog::Normal, "configExample: Config changed. New values: int=%d, float=%f, bool=%s, string=\"%s\".", myIntParam, myDoubleParam, myBoolParam?"true":"false", myStringParam);
    return true;
  }
};
  
int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser argParser(&argc, argv);
  argParser.loadDefaultArguments();
  if (argc < 2 || !Aria::parseArgs() || argParser.checkArgument("-help"))
  {
    ArLog::log(ArLog::Terse, "configExample usage: configExample <config file>.\nFor example, \"configExample examples/configExample.cfg\".");
    Aria::logOptions();
    Aria::exit(1);
    return 1;
  }
  
  // Object containing config parameters, and responding to changes:
  ConfigExample configExample;

  // Load a config file given on the command line into the global 
  // ArConfig object kept by Aria.  Normally ArConfig expects config
  // files to be in the main ARIA directory (i.e. /usr/local/Aria or
  // a directory specified by the $ARIA environment variable).
  char error[512];
  const char* filename = argParser.getArg(1);
  ArConfig* config = Aria::getConfig();
  ArLog::log(ArLog::Normal, "configExample: loading configuration file \"%s\"...", filename);
  if (! config->parseFile(filename, true, false, error, 512) )
  {
    ArLog::log(ArLog::Terse, "configExample: Error loading configuration file \"%s\" %s. Try \"examples/configExample.cfg\".", filename, error);
    Aria::exit(-1);
    return -1;
  }

  ArLog::log(ArLog::Normal, "configExample: Loaded configuration file \"%s\".", filename);
  
  ArConfigSection* section = config->findSection("Example Section");
  if (section)
  {
    ArConfigArg* arg = section->findParam("ExampleBoolParameter");
    if (arg)
    {
      arg->setBool(!arg->getBool());
      // After changing a config value, you should invoke the callbacks:
      if (! config->callProcessFileCallBacks(false, error, 512) )
      {
        ArLog::log(ArLog::Terse, "configExample: Error processing modified config: %s.", error);
      }
      else
      {
        ArLog::log(ArLog::Normal, "configExample: Successfully modified config and invoked callbacks.");
      }
    }
  }

  // You can save the configuration as well:
  ArLog::log(ArLog::Normal, "configExample: Saving configuration...");
  if(!config->writeFile(filename))
  {
    ArLog::log(ArLog::Terse, "configExample: Error saving configuration to file \"%s\"!", filename);
  }

  // end of program.
  ArLog::log(ArLog::Normal, "configExample: end of program.");
  Aria::exit(0);
  return 0;
}
