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
#ifndef ARARGUMENTPARSER_H
#define ARARGUMENTPARSER_H

#include "ariaTypedefs.h"
#include "ArArgumentBuilder.h"

/// Parse and store program command-line arguments for use by other ARIA classes.
/**
   This class is made for parsing arguments from the argv and argc variables
   passed into a program's main() function by the operating system, from
   an ArArgumentBuilder object, or just from a string (e.g. provided by WinMain() 
   in a Windows MFC program.)

   It will also load default argument values if you call
   loadDefaultArguments().   Aria::init() adds the file /etc/Aria.args and the 
   environment variable ARIAARGS as locations for argument defaults, so 
   loadDefaultArguments() will always search those. You can
   use this mechanism to avoid needing to always supply command line parameters
   to all programs. For example, if you use different serial ports than the defaults
   for the robot and laser, you can put a -robotPort or -laserPort argument in 
   /etc/Aria.args for all programs that call loadDefaultArguments() to use.
   You can add other files or environment variables
   to the list of default argument locations with
   addDefaultArgumentFile() and addDefaultArgumentEnv().

   Note that the wasReallySet behavior has changed.  It used to set
   the variable to false or to true, now it will only set it to false
   if 'setWasReallySetOnlyTrue' is set to false.  By default that is
   set to false, but you can just set it to true after you make the
   parser with no ill effects since all of the built in Aria parsers
   will play nicely with this value, setting it to true or false while
   they parse but then setting it to whatever it was when they
   started.  This change in behavior is so that checking for two
   things (like -robotPort and -rp) can work without a lot of extra
   work.

   @ingroup ImportantClasses
**/
class ArArgumentParser
{
public:
  /// Constructor, takes the argc argv
  AREXPORT ArArgumentParser(int *argc, char **argv);
  /// Constructor, takes an argument builder
  AREXPORT ArArgumentParser(ArArgumentBuilder *builder);
  /// Destructor
  AREXPORT ~ArArgumentParser();
  /// If we should only set wasReallySet to true
  AREXPORT void setWasReallySetOnlyTrue(bool wasReallySetOnlyTrue);
  /// If we only set wasReallySet to true
  AREXPORT bool getWasReallySetOnlyTrue(void);
  /// Returns true if the argument was found
  AREXPORT bool checkArgument(const char *argument);
  /// Returns the word/argument after given argument 
  AREXPORT char *checkParameterArgument(const char *argument, 
					bool returnFirst = false);
  /// Returns the word/argument after given argument 
  AREXPORT bool checkParameterArgumentString(const char *argument, 
					     const char **dest, 
					     bool *wasReallySet = NULL,
					     bool returnFirst = false);
  /// Returns the integer after given argument 
  AREXPORT bool checkParameterArgumentInteger(const char *argument, int *dest,
					      bool *wasReallySet = NULL, 
					      bool returnFirst = false);
  /// Returns the word/argument after given argument 
  AREXPORT bool checkParameterArgumentBool(const char *argument, bool *dest,
					   bool *wasReallySet = NULL,
					   bool returnFirst = false);
  /// Returns the floating point number after given argument 
  AREXPORT bool checkParameterArgumentFloat(const char *argument, float *dest, 
                bool *wasReallySet = NULL, bool returnFirst = false);
  /// Returns the floating point number after given argument 
  AREXPORT bool checkParameterArgumentDouble(const char *argument, double *dest, 
                bool *wasReallySet = NULL, bool returnFirst = false);
  /// Adds a string as a default argument
  AREXPORT void addDefaultArgument(const char *argument, int position = -1);
  /// Adds a string as a default argument as is (without touching
  /// spaces or what not)
  AREXPORT void addDefaultArgumentAsIs(const char *argument, 
				       int position = -1);
  /// Adds args from default files and environmental variables
  AREXPORT void loadDefaultArguments(int positon = 1);
  /// Checks for the help strings and warns about unparsed arguments
  AREXPORT bool checkHelpAndWarnUnparsed(unsigned int numArgsOkay = 0);
  /// Checks for the help argument  
  AREXPORT bool checkHelp();
  /// Gets how many arguments are left in this parser
  AREXPORT size_t getArgc(void) const;
  /// Gets the argv
  AREXPORT char** getArgv(void) const;
  /// Gets the argument builder, if one is being used (may be NULL)
  AREXPORT const ArArgumentBuilder *getArgumentBuilder(void) const 
    { return myBuilder; }
  /// Gets a specific argument
  AREXPORT const char* getArg(size_t whichArg) const;
  /// Prints out the arguments left in this parser
  AREXPORT void log(void) const;
  /// Gets the arguments this parser started with (if possible, NULL otherwise)
  AREXPORT const char *getStartingArguments(void) const;
  /// Internal function to remove an argument that was parsed
  AREXPORT void removeArg(size_t which);
  /// Adds another file or environmental variable to the list of defaults
  AREXPORT static void addDefaultArgumentFile(const char *file);
  /// Adds another file or environmental variable to the list of defaults
  AREXPORT static void addDefaultArgumentEnv(const char *env);
  /// Logs the default argument locations
  AREXPORT static void logDefaultArgumentLocations(void);
#ifndef SWIG
  /** @brief Returns true if the argument was found
   *  @swigomit
   */
  AREXPORT bool checkArgumentVar(const char *argument, ...);
  /** @brief Returns the word/argument after given argument 
   *  @swigomit
   */
  AREXPORT char *checkParameterArgumentVar(const char *argument, ...);
  /** @brief Returns the word/argument after given argument 
   *  @swigomit
   */
  AREXPORT bool checkParameterArgumentStringVar(bool *wasReallySet, 
						const char **dest, 
						const char *argument, ...);
  /** @brief Returns the word/argument after given argument 
   *  @swigomit
   */
  AREXPORT bool checkParameterArgumentBoolVar(bool *wasReallySet, bool *dest,
					      const char *argument, ...);
  /** @brief Returns the integer after given argument 
   *  @swigomit
   */
  AREXPORT bool checkParameterArgumentIntegerVar(bool *wasReallySet, int *dest,
						 const char *argument, ...);
  /** @brief Returns the float after given argument 
   *  @swigomit
   */
  AREXPORT bool checkParameterArgumentFloatVar(bool *wasReallySet, 
						float *dest,
						 const char *argument, ...);
  /** @brief Returns the double after given argument 
   *  @swigomit
   */
  AREXPORT bool checkParameterArgumentDoubleVar(bool *wasReallySet, 
						double *dest,
						const char *argument, ...);
#endif
protected:
  static std::list<std::string> ourDefaultArgumentLocs;
  static std::list<bool> ourDefaultArgumentLocIsFile;
  bool myOwnBuilder;
  ArArgumentBuilder *myBuilder;
  bool myUsingBuilder;
  char **myArgv;
  int *myArgc;
  bool myHelp;
  char myEmptyArg[1];
  bool myReallySetOnlyTrue;
};


#endif // ARARGUMENTPARSER_H
