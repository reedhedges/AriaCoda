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

/** @example lineFinderExample.cpp Simple example of Aria's line-finder utility,
 * which uses data from a laser rangefinder to detect a continues line of
 * points.
 *
 * This example program will constantly search for a line-like pattern in the
 * sensor readings of a laser rangefinder. If you press the 'f' key, those
 * points will be saved in 'points' and 'lines' files. Use arrow keys or
 * joystick to teleoperate the robot.
 */

int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;
  ArRobotConnector robotConnector(&parser, &robot);
  ArLaserConnector laserConnector(&parser, &robot, &robotConnector);

  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "lineFinderExample: Could not connect to the robot.");
    if(parser.checkHelpAndWarnUnparsed())
    {
        // -help not given
        Aria::logOptions();
        Aria::exit(1);
    }
  }

  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(1);
  }

  ArLog::log(ArLog::Normal, "lineFinderExample: Connected to robot.");

  robot.runAsync(true);

  // Connect to laser(s) as defined in parameter files.
  // (Some flags are available as arguments to connectLasers() to control error behavior and to control which lasers are put in the list of lasers stored by ArRobot. See docs for details.)
  if(!laserConnector.connectLasers())
  {
    ArLog::log(ArLog::Terse, "Could not connect to configured lasers. Exiting.");
    Aria::exit(3);
    return 3;
  }

  ArLog::log(ArLog::Normal, "lineFinderExample: Connected to laser");

  ArKeyHandler keyHandler;
  Aria::setKeyHandler(&keyHandler);
  robot.attachKeyHandler(&keyHandler);


  // Create the ArLineFinder object. Set it to log lots of information about its
  // processing.

  ArLaser *laser = robot.findLaser(1);
  if(!laser)
  {
    ArLog::log(ArLog::Terse, "lineFinderExample: No laser device connected, exiting.");
    Aria::exit(4);
    return 4;
  }

  ArLineFinder lineFinder(laser);
  lineFinder.setVerbose(true);

  // Add key callbacks that simply call the ArLineFinder::getLinesAndSaveThem()
  // function, which searches for lines in the current set of laser sensor
  // readings, and saves them in files with the names 'points' and 'lines'.
  ArFunctorC<ArLineFinder> findLineCB(&lineFinder, 
				  &ArLineFinder::getLinesAndSaveThem);
  keyHandler.addKeyHandler('f', &findLineCB);
  keyHandler.addKeyHandler('F', &findLineCB);

  
  printf("If you press the 'f' key the points and lines found will be saved\n");
  printf("Into the 'points' and 'lines' file in the current working directory\n");

  robot.waitForRunExit();
  Aria::exit(0);
  return 0;
}



