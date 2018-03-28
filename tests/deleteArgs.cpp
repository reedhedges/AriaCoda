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


// Pass command-line arguments to this program to really test it

//#define DO_DELETE 1
//#define PARSER 1
//#define BUILDER 1
#define BUILDER_ARGV 1
//#define DELETE_BEFORE_PARSE 1 // defining this might make it crash (but only if the OS NULLS out the argument builder or parser's internal pointers, Windows might do this in debug mode for instance)

int main(int argc, char **argv)
{
  ArRobot robot;
  Aria::init();
#if defined(PARSER)
  ArArgumentParser *parser = new ArArgumentParser(&argc, argv);
  puts("Creating ArSimpleConnector using parser.");
  ArSimpleConnector connector(parser);
#elif defined(BUILDER)
  ArArgumentBuilder *builder = new ArArgumentBuilder();
  builder->addStrings(argc, argv);
  puts("Creating ArSimpleConnector using builder.");
  ArSimpleConnector connector(builder);
#elif defined(BUILDER_ARGV)
  ArArgumentBuilder *builder = new ArArgumentBuilder();
  builder->addStrings(argc, argv);
  int c = builder->getArgc();
  puts("Creating ArSimpleConnector using (int *argc, char **argv) from builder.");
  printf("Got argv pointer 0x%x from builder.\n", builder->getArgv());
  ArSimpleConnector connector(&c, builder->getArgv());
#else
  #error Must define one of PARSER, BUILDER, BUILDER_ARGV
#endif

#ifndef DELETE_BEFORE_PARSE
  puts("Parsing arguments (before deletion)...");
  if (!Aria::parseArgs())
  {
    Aria::logOptions();
    Aria::shutdown();
    return 1;
  }
#endif
  
#ifdef DO_DELETE
  puts("Deleting argument object(s)...");
#if defined(PARSER)
  ArArgumentParser *p = parser;
  printf("Deleting ArArgumentParser 0x%x and writing %d NULL-bytes to that address (0x%x)\n", parser, sizeof(ArArgumentParser), p);
  delete parser;
  memset(p, 0, sizeof(ArArgumentParser));
#elif defined(BUILDER) || defined(BUILDER_ARGV)
  ArArgumentBuilder *b = builder;
  printf("Deleting ArArgumentBuilder 0x%x and writing %d NULL-bytes to that address (0x%x)\n", builder, sizeof(ArArgumentBuilder), b);
  delete builder;
  memset(b, 0, sizeof(ArArgumentBuilder));
#endif
#else
  puts("Not deleteing objects.");
#endif

#ifdef DELETE_BEFORE_PARSE
  puts("Parsing arguments (after deletion)...");
  if (!Aria::parseArgs())
  {
    Aria::logOptions();
    Aria::shutdown();
    return 1;
  }
#endif
  
  if (!connector.connectRobot(&robot))
  {
    ArLog::log(ArLog::Normal, "simpleConnect: Could not connect to robot... exiting");
    return 2;
  }

  ArLog::log(ArLog::Normal, "simpleConnect: Connected.");

  // Start the robot processing cycle running in the background.
  // True parameter means that if the connection is lost, then the 
  // run loop ends.
  robot.runAsync(true);

  // Print out some data from the SIP.  We must "lock" the ArRobot object
  // before calling its methods, and "unlock" when done, to prevent conflicts
  // with the background thread started by the call to robot.runAsync() above.
  // See the section on threading in the manual for more about this.
  robot.lock();
  ArLog::log(ArLog::Normal, "simpleConnect: Pose=(%.2f,%.2f,%.2f), Trans. Vel=%.2f, Battery=%.2fV",
    robot.getX(), robot.getY(), robot.getTh(), robot.getVel(), robot.getBatteryVoltage());
  robot.unlock();

  // Sleep for 3 seconds.
  ArLog::log(ArLog::Normal, "simpleConnect: Sleeping for 3 seconds...");
  ArUtil::sleep(3000);

  
  ArLog::log(ArLog::Normal, "simpleConnect: Ending robot thread...");
  robot.stopRunning();

  ArLog::log(ArLog::Normal, "simpleConnect: Exiting.");
  return 0;
}
