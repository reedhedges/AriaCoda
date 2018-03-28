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

/** @example lasers.cpp example showing how to connect to laser rangefinders attached to the robot and retrieve data from them.  What kinds of laser or lasers are connected to the robot, and their parameters, are loaded from the robot's parameter file in the params directory. First the default for this type of mobile robot is loaded (e.g. p3dx-sh.p for a Pioneer 3 DX), followed by a parameter file specific to this individual robot (based on its 'name'), if such a parameter file exists.  Laser parameters may also be given as command line options, use the -help option to list them. This program will only connect to any lasers that have LaserAutoConnect true in the parameter file, or if the -connectLaser or -cl command-line options are given. You may need to edit the parameter file for your robot to set LaserAutoConnect to true.
 
   TODO: show how to force laser connect even if autoconnect is false.
 *
 * This program will work either with the MobileSim simulator or on a real
 * robot's onboard computer.  (Or use -remoteHost to connect to a wireless
 * ethernet-serial bridge.)
 */

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
    ArLog::log(ArLog::Terse, "lasersExample: Could not connect to the robot.");
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
    Aria::exit(2);
    return 2;
  }
  
  ArLog::log(ArLog::Normal, "lasersExample: Connected to robot.");

  // Start the robot processing cycle running in the background.
  // True parameter means that if the connection is lost, then the 
  // run loop ends.
  robot.runAsync(true);

  // Connect to laser(s) defined in parameter files, if LaseAutoConnect is true
  // for the laser. 
  // (Some flags are available as arguments to connectLasers() to control error behavior and to control which lasers are put in the list of lasers stored by ArRobot. See docs for details.)
  ArLog::log(ArLog::Normal, "lasersExample: Connecting to any lasers enabled for connect...");
  if(!laserConnector.connectLasers())
  {
    ArLog::log(ArLog::Terse, "lasersExample: Could not connect to configured lasers. Exiting.");
    Aria::exit(3);
    return 3;
  }
  ArLog::log(ArLog::Normal, "lasersExample: Connected to all lasers.\n");

  // Log parameters related to the lasers
  ArLog::log(ArLog::Normal, "\nlasersExample: Robot parameters include information for %d lasers:", robot.getNumLasers());
  const ArRobotParams *params = robot.getRobotParams();
  for(size_t i = 1; i <= robot.getNumLasers(); ++i)
  {
    if(!robot.findLaser(i))
      continue;
    ArLog::log(ArLog::Normal, "\tlaser #%d: pose=(x:%d, y:%d, z:%d, th:%0.2f), powerOutput=%s",
      i,
      params->getLaserX(i), params->getLaserY(i), params->getLaserZ(i), params->getLaserTh(i),
      params->getLaserPowerOutput(i)
    );
  }
  ArLog::log(ArLog::Normal, "lasersExample: ArLaserConnector also using the following parameters (if values given, otherwise defaults for this laser type will be used):");
  laserConnector.logLaserData();
  ArLog::log(ArLog::Normal, "");

  // Allow some time to read laser data
  ArUtil::sleep(500);

  // Print out some data from each connected laser.
  while(robot.isConnected())
  {
    int numLasers = 0;

 	  // Get a pointer to ArRobot's list of connected lasers. We will lock the robot while using it to prevent changes by tasks in the robot's background task thread or any other threads. Each laser has an index. You can also store the laser's index or name (laser->getName()) and use that to get a reference (pointer) to the laser object using ArRobot::findLaser().
	  robot.lock();
	  std::map<int, ArLaser*> *lasers = robot.getLaserMap();

    ArLog::log(ArLog::Normal, "lasersExample: ArRobot provided a set of %d ArLaser objects.", lasers->size());

	  for(std::map<int, ArLaser*>::const_iterator i = lasers->begin(); i != lasers->end(); ++i)
	  {
      int laserIndex = (*i).first;
      ArLaser* laser = (*i).second;
      if(!laser)
        continue;
      ++numLasers;
      laser->lockDevice();

      // The current readings are a set of obstacle readings (with X,Y positions as well as other attributes) that are the most recent set from teh laser.
      std::list<ArPoseWithTime*> *currentReadings = laser->getCurrentBuffer(); // see ArRangeDevice interface doc

      // The raw readings are just range or other data supplied by the sensor. It may also include some device-specific extra values associated with each reading as well. (e.g. Reflectance for LMS200)
      const std::list<ArSensorReading*> *rawReadings = laser->getRawReadings();
        

      // There is a utility to find the closest reading wthin a range of degrees around the laser, here we use this laser's full field of view (start to end)
      // If there are no valid closest readings within the given range, dist will be greater than laser->getMaxRange().
      double angle = 0;
      double dist = laser->currentReadingPolar(laser->getStartDegrees(), laser->getEndDegrees(), &angle);

      ArLog::log(ArLog::Normal, "lasersExample: Laser #%d (%s): %s.\n\tHave %d 'current' readings.\n\tHave %d 'raw' readings.\n\tClosest reading is at %3.0f degrees and is %2.4f meters away.", 
        laserIndex, laser->getName(), (laser->isConnected() ? "connected" : "NOT CONNECTED"), 
        currentReadings->size(), 
        rawReadings->size(),
        angle, dist/1000.0);
                  laser->unlockDevice();
    }
    if(numLasers == 0)
      ArLog::log(ArLog::Normal, "lasersExample: No lasers.");
    else
      ArLog::log(ArLog::Normal, "");

    // Unlock robot and sleep for a few seconds before next loop.
    robot.unlock();
    ArUtil::sleep(3000);
  }

  ArLog::log(ArLog::Normal, "lasersExample: exiting.");
  Aria::exit(0);
  return 0;
}
