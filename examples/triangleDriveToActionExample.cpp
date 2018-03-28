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

/** @example triangleDriveToActionExample.cpp Example/demonstration of
 * ArActionTriangleDriveTo, which drives the robot towards a specially shaped
 * triangular target. The triangular target by default is a convex triangle
 * target consisting of two lines of at least 250mm each meeting at a 135 degree
 * angle.  See the documentation for ArActionTriangleDriveTo for more
 * information about the triangle locating and approach behavior, and how to
 * customize it.
 *
   Press g or G to use ArActionTriangleDriveTo to detect and drive towards
   a triangular target shape.  Press s or S to stop.
   See ArActionTriangleDriveTo for more information about the triangular target and what the action does and its parameters.
 **/

#include "Aria.h"


int main(int argc, char **argv)
{
  Aria::init();
  ArRobot robot;
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobotConnector robotConnector(&parser, &robot);
  ArLaserConnector laserConnector(&parser, &robot, &robotConnector);

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "triangleDriveToActionExample: Could not connect to the robot.");
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

  ArLog::log(ArLog::Normal, "triangleDriveToActionExample: Connected to robot.");


  if(!laserConnector.connectLasers())
  {
    ArLog::log(ArLog::Terse, "triangleDriveToActionExample: Could not connect to laser(s).  Exiting.");
    Aria::exit(3);
    return 3;
  }
  ArLog::log(ArLog::Normal, "Connected to laser.");

  ArKeyHandler keyHandler;
  Aria::setKeyHandler(&keyHandler);
  robot.attachKeyHandler(&keyHandler);

  
  ArActionTriangleDriveTo triangleDriveTo;
  ArFunctorC<ArActionTriangleDriveTo> lineGoCB(&triangleDriveTo, 
				      &ArActionTriangleDriveTo::activate);
  keyHandler.addKeyHandler('g', &lineGoCB);
  keyHandler.addKeyHandler('G', &lineGoCB);
  ArFunctorC<ArActionTriangleDriveTo> lineStopCB(&triangleDriveTo, 
					&ArActionTriangleDriveTo::deactivate);
  keyHandler.addKeyHandler('s', &lineStopCB);
  keyHandler.addKeyHandler('S', &lineStopCB);

  ArActionLimiterForwards limiter("limiter", 150, 0, 0, 1.3);
  robot.addAction(&limiter, 70);
  ArActionLimiterBackwards limiterBackwards;
  robot.addAction(&limiterBackwards, 69);

  robot.addAction(&triangleDriveTo, 60);

  ArActionKeydrive keydrive;
  robot.addAction(&keydrive, 55);


  ArActionStop stopAction;
  robot.addAction(&stopAction, 50);
  
  robot.comInt(ArCommands::SONAR, 1);
  robot.comInt(ArCommands::ENABLE, 1);
  
  // start the robot running, true so that if we lose connection the run stops
  robot.runAsync(true);


  printf("press the 'g' key to locate a triangle shape, press 's' to stop teh robot.\n");

  robot.waitForRunExit();
  Aria::exit(0);
  return 0;
}



