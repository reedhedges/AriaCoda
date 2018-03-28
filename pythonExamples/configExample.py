"""
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
"""

from __future__ import division # For correct float division in Python 2
from AriaPy import *
import sys

"""
* @example configExample.cpp Example program demonstrating the use of ArConfig
 *
 *  This program shows how to use ArConfig to store configuration parameters
 *  and load/save them from a file.  
 *
 *  The ArNetworking library includes server classes that will let you
 *  use a remote client such as MobileEyes to view and change the configuration.
 *  See the ArNetworking documentation and examples.
 """


class ConfigExample:

  def __init__(self):

    # The global class Aria() contains an ArConfig object.  You can create
    # other instances of ArConfig, but this is how you can share one ArConfig
    # among various program modules.
    # If you want to store a config parameter in ArConfig, first you must add 
    # it to the ArConfig() object.  Parameters are stored in sections.
    # 
    # In the C++ library, ArConfigArg stores a pointer to a variable, which is
    # directly modified by ArConfig.  In Python, however, it can't do that, so 
    # you must get the value from ArConfig using findSection() and findParam().
    # Furthermore, the wrapper library can't distinguish between integers,
    # booleans, etc. so you must use special _Int, _Double, _Bool, and _String 
    # subclasses for different types.

    config = Aria_getConfig()
    config.setSectionComment("Example Section", "Contains parameters created by the configExample")

    # Add an integer which ranges from -10 to 10:
    intParam = ArConfigArg_Int("ExampleIntegerParameter", 0, "Example parameter integer.", -10, 10)
    config.addParam(intParam, "Example Section", ArPriority.NORMAL)
    
    # Add a floating point number which ranges from 0.0 to 1.0:
    doubleParam = ArConfigArg_Double("ExampleDoubleParameter", 0.5, "Example double precision floating point number.", 0.0, 1.0)
    config.addParam(doubleParam, "Example Section", ArPriority.NORMAL)

    # Essential parameters can be placed in the "Important" priority level:
    boolParam = ArConfigArg_Bool("ExampleBoolParameter", 1, "Example boolean parameter.")
    config.addParam(boolParam, "Example Section", ArPriority.IMPORTANT)

    # Unimportant parameters can be placed in the "Trivial" priority level:
    stringParam = ArConfigArg_String("ExampleStringParameter", "Hello, world", "Example string parameter.")
    config.addParam(stringParam, "Example Section", ArPriority.TRIVIAL)

    # You can set a callback to be invoked when the configuration changes, in
    # case you need to respond to any changes in the parameter values:
    config.addProcessFileCB(self.processConfigFile, 0)

  
  # Method called by config process callback when a new file is loaded.
  # It can return False (0) to indicate an error, or True (1) to indicate no error.
  def processConfigFile(self):
    config = Aria_getConfig()
    section = config.findSection("Example Section")
    if not section:
      ArLog.log(ArLog.Normal, "configExample: config file does not have section \"Example Section\".")
      return 0
    intParam = section.findParam("ExampleIntegerParameter")
    if not intParam:
      ArLog.log(ArLog.Normal, "configExample: config file does not have ExampleIntegerParameter")
      return 0
    doubleParam = section.findParam("ExampleDoubleParameter")
    if not doubleParam:
      ArLog.log(ArLog.Normal, "configExample: config file does not have ExampleDoubleParameter.")
      return 0
    stringParam = section.findParam("ExampleStringParameter")
    if not stringParam:
      ArLog.log(ArLog.Normal, "configExample: config file does not have ExampleStringParameter.")
      return 0
    boolParam = section.findParam("ExampleBoolParameter")
    if not boolParam:
      ArLog.log(ArLog.Normal, "configExample: config file does not have ExampleBoolParameter.")
      return 0
    ArLog.log(ArLog.Normal, ( "configExample: processConfigFile callback: Config changed. New values: int=%d, float=%f, bool=%s, string=\"%s\"." % (intParam.getInt(), doubleParam.getDouble(), boolParam.getBool(), stringParam.getString()) ) )
    return True
  
Aria_init()
argParser = ArArgumentParser(sys.argv)
argParser.loadDefaultArguments()

if len(sys.argv) < 2  or  not Aria_parseArgs()  or  argParser.checkArgument("-help"):
  ArLog.log(ArLog.Terse, "configExample: Error: no config file given as command line argument.");
  ArLog.log(ArLog.Terse, "configExample: usage: configExample <config file>.\nFor example, \"configExample configExample.cfg\".")
  Aria_logOptions()
  Aria_shutdown()
  sys.exit(1)

# Object containing config parameters, and responding to changes (defined above):
configExample = ConfigExample()

# Load a config file given on the command line into the global 
# ArConfig object kept by Aria_  Normally ArConfig expects config
# files to be in the main ARIA directory (i.e. /usr/local/Aria or
# a directory specified by the $ARIA environment variable)., but
# let's add "pythonExamples" to it, to load files from the pythonExamples
# directory.
filename = argParser.getArg(1)
config = Aria_getConfig()
config.setBaseDirectory(config.getBaseDirectory() + "pythonExamples/")
ArLog.log(ArLog.Normal, "configExample: loading configuration file \"%s\" from directory \"%s\"... (processConfigFile callback should be called with new values)" % (filename, config.getBaseDirectory()))
#config.setProcessFileCallbacksLogLevel(ArLog.Normal)
if (not config.parseFile(filename, False)):
  ArLog.log(ArLog.Terse, "configExample: Error loading configuration file \"%s\"." % (filename))
else:
  ArLog.log(ArLog.Normal, "configExample: Loaded configuration file \"%s\".  Setting some values; processConfigFile callback should be called each time and see the new value." % (filename))

  # Try changing some of the parameters:
  # After changing a config value, you should invoke the callbacks.
  section = config.findSection("Example Section")
  if section:
    arg = section.findParam("ExampleBoolParameter")
    if arg:
      print "configExample: setting ExampleBoolParameter to %s" % (not arg.getBool())
      arg.setBool(not arg.getBool())
    arg = section.findParam("ExampleStringParameter")
    if arg:
      print "configExample: setting ExampleStringParameter to \"%s\"" % (arg.getString() + " Modified!")
      arg.setString(arg.getString() + " Modified!")
    if not config.callProcessFileCallBacks(False):
      ArLog.log(ArLog.Terse, "configExample: Error processing modified config.")
    else:
      ArLog.log(ArLog.Normal, "configExample: Successfully modified config and invoked callbacks.")
        

# You can save the configuration as well:
ArLog.log(ArLog.Normal, "configExample: Saving configuration...")
if not config.writeFile(filename):
  ArLog.log(ArLog.Terse, "configExample: Error saving configuration to file \"%s\"not ", filename)

# end of program.
ArLog.log(ArLog.Normal, "configExample: end of program.")
Aria_shutdown()

