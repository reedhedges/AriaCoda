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
#include <stdio.h>

/* Queries a subset of the ARIA classes for help text on their command line
 * arguments, outputs nd creates files containing that text, plus two files that
 * aggregates them all, one with some HTML formatting for use in the doxygen
 * documentation, one with an explanatory header for use as a stand alone
 * text file.  It redirects the log help output be reopening stdout bound to 
 * each file and then calling the logOptions() or similar methods on objects
 * of the classes with options.
 *
 * 
 * When compiling this program, you must define either FOR_ARIA or
 * FOR_ARNETWORKING (or both) with -D, since this program can be used either 
 * for ARIA or ArNetworking.
 * 
 * The files generated are:
 *   docs/options/<class name>
 *   docs/options/all_options.dox
 *   CommandLineOptions.txt
 * 
 * If FOR_ARIA is defined, then the classes whose options are included are:
 *   ArRobotConnector
 *   ArLaserConnector with each laser type
 *   ArPTZConnector
 *   ArGPSConnector
 *   ArSonarConnector
 *   [ArDaemonizer?]
 * 
 * If FOR_ARNETWORKING is defined, then these classes are also included:
 *   ArClientSimpleConnector
 *   ArServerSimpleOpener
 *   ArClientSwitchManager
 */



/* Wrapper classes provide a standardized publically accessible logOptions()
 * method for each class that have the ability to log options in some way. 
 * (They can all be stored as Wrapper classes below, and they expose
 * logOptions() as a public method instead of private.)
 */

class Wrapper {
public:
  virtual void logOptions() = 0;
};


#ifdef FOR_ARIA
#include "ArArgumentParser.h"
#include "ArRobotConnector.h"
#include "ArGPSConnector.h"
//#include "ArTCM2.h"
#include "ArSonarConnector.h"
#include "ArPTZConnector.h"
#include "ArLMS2xx.h"

class ArRobotConnectorWrapper : 
  public ArRobotConnector,  public virtual Wrapper
{
public:
  ArRobotConnectorWrapper(ArArgumentParser *argParser) :
    ArRobotConnector(argParser, NULL)
  {
  }
  virtual void logOptions()
  {
    ArRobotConnector::logOptions();
  }
};

class ArPTZConnectorWrapper :
  public ArPTZConnector, public virtual Wrapper
{
public:
  ArPTZConnectorWrapper(ArArgumentParser *argParser) :
    ArPTZConnector(argParser, NULL)
  {
  }
  virtual void logOptions()
  {
    ArPTZConnector::logOptions();
  }
};


class ArLaserConnectorWrapper : 
  public ArLaserConnector,  public virtual Wrapper
{

  ArLMS2xx lms2xxLaser;
  ArUrg urgLaser;
  ArUrg_2_0 urg2Laser;
  ArLMS1XX lms1xxLaser;
  ArLMS1XX lms5xxLaser;
  ArS3Series s3xxLaser;
  ArSZSeries szLaser;
  ArLMS1XX tim3xxLaser;
  ArLMS1XX tim551Laser;
  ArLMS1XX tim561Laser;
  ArLMS1XX tim571Laser;
public:
  ArLaserConnectorWrapper(ArArgumentParser *argParser) :
    ArLaserConnector(argParser, NULL, NULL),
    lms2xxLaser(1), urgLaser(1), urg2Laser(1), 
    lms1xxLaser(1, "lms1XX", ArLMS1XX::LMS1XX),
    lms5xxLaser(1, "lms5xx", ArLMS1XX::LMS5XX), 
    s3xxLaser(1), szLaser(1),
    tim3xxLaser(1, "tim3XX", ArLMS1XX::TiM3XX),
    tim551Laser(1, "tim551", ArLMS1XX::TiM551),
    tim561Laser(1, "tim561", ArLMS1XX::TiM561),
    tim571Laser(1, "tim571", ArLMS1XX::TiM571)
  {
  }
  virtual void logOptions()
  {
    puts(
"Laser types and options may also be set in the robot parameter file.  See the\n"
"ARIA reference documentation for details.\n\n"
"If a program supports multiple lasers, then options for additional lasers\n"
"after the first are given by appending the laser number (e.g. -laserType2)\n"
"To enable use of a laser, choose its type with the -laserType<N> options\n"
"(e.g.: -laserType lms2xx -laserType2 urg2.0)\n\n" 
"The default laser type for the primary laser (laser 1) is specified in the\n"
"robot type parameter file in the ARIA \"params\" directory. For many robots\n"
"it is \"lms2xx\", the SICK LMS200. For some it is \"lms1xx\", for the SICK\n"
"LMS100 or LMS111.\n\n"
"Instruct a program to connect to a laser using the -connectLaser<N> option\n"
"or by setting LaserAutoConnect to true in the robot's parameter file.\n"
"If a program requires use of a laser it usually always attempts to connect to\n"
"the primary laser, however.\n\n"
"The index number is optional in any options for the primary laser; i.e. 1 is\n"
"assumed if the index number is omitted.\n\n"
);


    // TODO loop through all laser registered with Aria class and instantiate
    // them that way.

    // lms 200
    puts("\nFor laser type \"lms2xx\" (SICK LMS-200):\n");
    addPlaceholderLaser(&lms2xxLaser, 1);
    ArLaserConnector::logLaserOptions(myLasers[1], false, false);

    // lms 100
    puts("\nFor laser type \"lms1xx\" (SICK LMS-100, LMS-111, etc.):\n");
    addPlaceholderLaser(&lms1xxLaser, 1); // replace lms2xx as  first laser
    ArLaserConnector::logLaserOptions(myLasers[1], false, false);

    // urg2.0
    puts("\nFor laser type \"urg2.0\" (URG with SCIP 2.0):\n");
    addPlaceholderLaser(&urg2Laser, 1); // replace lms1xx as first laser
    ArLaserConnector::logLaserOptions(myLasers[1], false, false);

    // urg 1.0
    puts("\nFor laser type \"urg\" (URG with old SCIP 1.0):\n");
    addPlaceholderLaser(&urgLaser, 1); // replace urg2.0 as first laser
    ArLaserConnector::logLaserOptions(myLasers[1], false, false);

    // LMS500
    puts("\nFor laser type \"lms5XX\" (SICK LMS-500):\n");
    addPlaceholderLaser(&lms5xxLaser, 1); // replace previous
    ArLaserConnector::logLaserOptions(myLasers[1], false, false);

    // SZ 
    puts("\nFor laser type \"sZseries\" (Keyence SZ):\n");
    addPlaceholderLaser(&szLaser, 1); // replace previous
    ArLaserConnector::logLaserOptions(myLasers[1], false, false);

    // S3xx
    puts("\nFor laser type \"s3series\" (SICK S-300, S-3000, etc.):\n");
    addPlaceholderLaser(&s3xxLaser, 1); // replace previous
    ArLaserConnector::logLaserOptions(myLasers[1], false, false);

    // TiM3xx
    puts("\nFor laser type \"tim510\" or \"tim3XX\" (SICK TiM310 and TiM510):\n");
    addPlaceholderLaser(&tim3xxLaser, 1); // replace previous
    ArLaserConnector::logLaserOptions(myLasers[1], false, false);

    // TiM551
    puts("\nFor laser type \"tim551\" (SICK TiM551):\n");
    addPlaceholderLaser(&tim551Laser, 1); // replace previous
    ArLaserConnector::logLaserOptions(myLasers[1], false, false);

    // TiM561
    puts("\nFor laser type \"tim561\" (SICK TiM561):\n");
    addPlaceholderLaser(&tim551Laser, 1); // replace previous
    ArLaserConnector::logLaserOptions(myLasers[1], false, false);

    // TiM571
    puts("\nFor laser type \"tim571\" (SICK TiM571):\n");
    addPlaceholderLaser(&tim571Laser, 1); // replace previous
    ArLaserConnector::logLaserOptions(myLasers[1], false, false);

  }
};

class ArGPSConnectorWrapper : 
  public ArGPSConnector, 
  public virtual Wrapper
{
public:
  ArGPSConnectorWrapper(ArArgumentParser *argParser) :
    ArGPSConnector(argParser)
  {
  }
  virtual void logOptions()
  {
    ArGPSConnector::logOptions();
  }
};

/*
class ArCompassConnectorWrapper : 
  public ArCompassConnector, 
  public virtual Wrapper
{
public:
  ArCompassConnectorWrapper(ArArgumentParser *argParser) :
    ArCompassConnector(argParser)
  {
  }
  virtual void logOptions()
  {
    ArCompassConnector::logOptions();
  }
};
*/

class ArSonarConnectorWrapper: public ArSonarConnector, public virtual Wrapper
{
public:
  ArSonarConnectorWrapper(ArArgumentParser *argParser) :
    ArSonarConnector(argParser, NULL, NULL) { }
  virtual void logOptions() {
    ArSonarConnector::logOptions();
  }
};

class ArBatteryConnectorWrapper: public ArBatteryConnector, public virtual Wrapper
{
public:
  ArBatteryConnectorWrapper(ArArgumentParser *argParser) :
    ArBatteryConnector(argParser, NULL, NULL) { }
  virtual void logOptions() {
    ArBatteryConnector::logOptions();
  }
};

class ArLCDConnectorWrapper: public ArLCDConnector, public virtual Wrapper
{
public:
  ArLCDConnectorWrapper(ArArgumentParser *argParser) :
    ArLCDConnector(argParser, NULL, NULL) { }
  virtual void logOptions() {
    ArLCDConnector::logOptions();
  }
};

#endif

#ifdef FOR_ARNETWORKING
#include "ArNetworking.h"

class ArClientSimpleConnectorWrapper : public ArClientSimpleConnector, public virtual Wrapper
{
public:
  ArClientSimpleConnectorWrapper(ArArgumentParser *argParser) : ArClientSimpleConnector(argParser)
  {
  }
  virtual void logOptions()
  {
    ArClientSimpleConnector::logOptions();
  }
};

class ArServerSimpleOpenerWrapper : public ArServerSimpleOpener, public virtual Wrapper
{
public:
  ArServerSimpleOpenerWrapper(ArArgumentParser *argParser) : ArServerSimpleOpener(argParser)
  {
  }
  virtual void logOptions()
  {
    ArServerSimpleOpener::logOptions();
  }
};

class ArClientSwitchManagerWrapper : public ArClientSwitchManager, public virtual Wrapper
{
public:
  ArClientSwitchManagerWrapper(ArServerBase *server, ArArgumentParser *argParser) : ArClientSwitchManager(server, argParser)
  {
  }
  virtual void logOptions()
  {
    ArClientSwitchManager::logOptions();
  }
};
#endif

const char *EXPLANATION = 
"Some classes in ARIA and ArNetworking check a program's run time options to\n"
"specify parameters and options. These options are used to configure run time\n"
"accessory device parameters (ports, speeds, etc.) used by ARIA; host names,\n"
"port numbers, etc. used by ArNetworking; and various other run time options.\n"
"Options may be given as program arguments on the command line, or globally\n"
"saved as defaults in the file /etc/Aria.args if on Linux, or in the ARIAARGS\n"
"environment variable.  Arguments given on the command line may override some\n"
"internal defaults or values read from the robot parameter files.\n\n"
"Note, an option will be available only in programs that instantiate an\n"
"object of the class that uses it. Some programs may also check for\n"
"program-specific command line options.\n\n"
"Use the special \"-help\" command line option to cause a program to \n"
"print out its available options.\n\n"
"A list of options used by each class follows.\n\n";

/* Redirect stdout to a file. If reopening stdout for the new file fails, print
 * a message and exit the program with error code 3.
 */
void redirectStdout(const char *filename)
{
  FILE *fp = freopen(filename, "w", stdout);
  if(fp == NULL)
  {
    fprintf(stderr, "genCommandLineOptionDocs: Error opening \"%s\"!  Exiting.\n", filename);
    Aria::exit(3);
  }
}

typedef std::pair<std::string, Wrapper*> WrapPair;
typedef std::vector<WrapPair> WrapList;

int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser argParser(&argc, argv);

  WrapList wrappers;

#ifdef FOR_ARIA
  wrappers.push_back(WrapPair("ArRobotConnector", new ArRobotConnectorWrapper(&argParser)));
  wrappers.push_back(WrapPair("ArLaserConnector", new ArLaserConnectorWrapper(&argParser)));
  wrappers.push_back(WrapPair("ArPTZConnector", new ArPTZConnectorWrapper(&argParser)));
  wrappers.push_back(WrapPair("ArGPSConnector", new ArGPSConnectorWrapper(&argParser)));
//  wrappers.push_back(WrapPair("ArCompassConnector", new ArCompassConnectorWrapper(&argParser)));
  wrappers.push_back(WrapPair("ArSonarConnector", new ArSonarConnectorWrapper(&argParser)));
  wrappers.push_back(WrapPair("ArBatteryConnector", new ArBatteryConnectorWrapper(&argParser)));
  wrappers.push_back(WrapPair("ArLCDConnector", new ArLCDConnectorWrapper(&argParser)));
#endif

#ifdef FOR_ARNETWORKING
  ArServerBase server;
  wrappers.push_back(WrapPair("ArClientSimpleConnector", new ArClientSimpleConnectorWrapper(&argParser)));
  wrappers.push_back(WrapPair("ArServerSimpleOpener", new ArServerSimpleOpenerWrapper(&argParser)));
  wrappers.push_back(WrapPair("ArClientSwitchManager", new ArClientSwitchManagerWrapper(&server, &argParser)));
#endif

  /* Write docs/options/all_options.dox */
  // TODO process text to replace HTML characters with entities or formatting
  // (especially < and >)
  redirectStdout("docs/options/all_options.dox");
  printf("/* This file was automatically generated by utils/genCommandLineOptionDocs.cpp. Do not modify or changes will be lost.*/\n\n"\
    "/** @page CommandLineOptions Command Line Option Summary\n\n%s\n\n", EXPLANATION);
  // TODO process text by turning it into a <dl> or similar: start new <dt> at
  // beginning of line, add </dt><dd> at first \t, then add </dt> at end.
  for(WrapList::const_iterator i = wrappers.begin(); i != wrappers.end(); ++i)
  {
    printf("@section %s\n\n(See %s for class documentation)\n\n@verbatim\n", (*i).first.c_str(), (*i).first.c_str());
    (*i).second->logOptions();
    puts("@endverbatim\n");
    fprintf(stderr, "genCommandLineOptionDocs: Added %s to docs/options/all_options.dox\n", (*i).first.c_str());
  }
  puts("*/");
  fputs("genCommandLineOptionDocs: Wrote docs/options/all_options.dox\n", stderr);


  /* Write docs/options/<class> */
  for(WrapList::const_iterator i = wrappers.begin(); i != wrappers.end(); ++i)
  {
    std::string filename("docs/options/");
    filename += (*i).first + "_options";
    redirectStdout(filename.c_str());
    (*i).second->logOptions();
    fprintf(stderr, "genCommandLineOptionDocs: Wrote %s\n", filename.c_str());
  }

  /* Write CommandLineOptions.txt */
  redirectStdout("CommandLineOptions.txt");
#ifdef FOR_ARIA
  puts("\nARIA\n");
#elif defined(FOR_ARNETWORKING)
  puts("\nArNetworking\n");
#endif
  printf("Summary of command line options\n\n%s", EXPLANATION);

  for(WrapList::const_iterator i = wrappers.begin(); i != wrappers.end(); ++i)
  {
    puts("");
    puts((*i).first.c_str());
    for(std::string::size_type c = 0; c < (*i).first.size(); ++c)
      fputc('-', stdout);
    puts("");
    (*i).second->logOptions();
  }
  puts("\n");
  fputs("genCommandLineOptionDocs: Wrote CommandLineOptions.txt\n", stderr);

  Aria::exit(0);
}
