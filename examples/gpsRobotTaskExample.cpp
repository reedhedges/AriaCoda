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
#include "ArGPS.h"
#include "ArGPSConnector.h"

#include <assert.h>


/** @example gpsRobotTaskExample.cpp Connects to both robot and GPS, allows
 * teleoperation, and prints robot position and GPS data.
 */


/*  
 *  This class encapsulates an ArRobot sensor interpretation task that prints the
 *  last set of GPS data along with a local timestamp and current robot pose to 
 *  standard output.
 *
 *  This class also contains a mutex, which it locks during the task (while
 *  accessing the ArGPS object).  If another thread is also accessing the GPS,
 *  you can lock this mutex.
 */
class GPSLogTask {

public:
  GPSLogTask(ArRobot *robot, ArGPS *gps, ArJoyHandler *joy = NULL) :
      myRobot(robot), 
      myGPS(gps),
      myTaskFunctor(this, &GPSLogTask::doTask),
      myJoyHandler(joy),
      myButtonDown(false)
  {
    myRobot->addSensorInterpTask("GPS", ArListPos::LAST, &myTaskFunctor);   
    puts("RobotX\tRobotY\tRobotTh\tRobotVel\tRobotRotVel\tRobotLatVel\tLatitude\tLongitude\tAltitude\tSpeed\tGPSTimeSec\tGPSTimeMSec\tFixType\tNumSats\tPDOP\tHDOP\tVDOP\tGPSDataReceived");
  }

  void lock() { myMutex.lock(); }
  void unlock() { myMutex.unlock(); }

protected:

  void doTask()
  {
    // print a mark if a joystick button is pressed (other than 1, which is
    // needed to drive)
    if(myJoyHandler)
    {
      for(unsigned int b = 2; b <= myJoyHandler->getNumButtons(); ++b)
        if(myJoyHandler->getButton(b)) {
          if(!myButtonDown)
            printf("--------------- Joystick button %d pressed.\n", b);
          myButtonDown = true;
        }
        else
          myButtonDown = false;
    }

    lock();
    int f = myGPS->read(50);
    printf("%.4f\t%.4f\t%.4f\t%.4f\t%.4f\t%.4f"
           "\t%2.8f\t%2.8f\t%4.4f\t%4.4f"
           "\t%lu\t%lu\t%s"
           "\t%u\t%2.4f\t%2.4f\t%2.4f"
           "\t%s\n",
      myRobot->getX(), myRobot->getY(), myRobot->getTh(), myRobot->getVel(), myRobot->getRotVel(), (myRobot->hasLatVel())?(myRobot->getLatVel()):0,
      myGPS->getLatitude(), myGPS->getLongitude(), myGPS->getAltitude(), myGPS->getSpeed(),
      myGPS->getGPSPositionTimestamp().getSec(), myGPS->getGPSPositionTimestamp().getMSec(), myGPS->getFixTypeName(),
      myGPS->getNumSatellitesTracked(), myGPS->getPDOP(), myGPS->getHDOP(), myGPS->getVDOP(),
      ((f&ArGPS::ReadUpdated)?"yes":"no")
    );
    unlock();
  }

private:
  ArRobot *myRobot;
  ArGPS *myGPS;
  ArFunctorC<GPSLogTask> myTaskFunctor;
  ArMutex myMutex;
  ArJoyHandler *myJoyHandler;
  bool myButtonDown;
};



int main(int argc, char** argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;
  ArRobotConnector robotConnector(&parser, &robot);
  ArGPSConnector gpsConnector(&parser);
  ArLaserConnector laserConnector(&parser, &robot, &robotConnector);

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "gpsRobotTaskExample: Could not connect to the robot.");
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

  ArLog::log(ArLog::Normal, "gpsRobotTaskExample: Connected to robot.");

  // Start the robot  running
  robot.runAsync(true);

  // Connect to laser(s) as defined in parameter files.
  if(!laserConnector.connectLasers())
  {
    ArLog::log(ArLog::Terse, "Warning, Could not connect to configured lasers. ");
  }
  

  ArActionLimiterForwards nearLimitAction("limit near", 300, 600, 250);
  ArActionLimiterForwards farLimitAction("limit far", 300, 1100, 400);
  ArActionLimiterBackwards limitBackwardsAction;
  ArActionJoydrive joydriveAction;
  ArActionKeydrive keydriveAction;


  // Connect to GPS
  ArLog::log(ArLog::Normal, "gpsRobotTaskExample: Connecting to GPS, it may take a few seconds...");
  ArGPS *gps = gpsConnector.createGPS(&robot);
  assert(gps);
  if(!gps || !(gps->connect()))
  {
    ArLog::log(ArLog::Terse, "gpsRobotTaskExample: Error connecting to GPS device.  Try -gpsType, -gpsPort, and/or -gpsBaud command-line arguments. Use -help for help. Exiting.");
    return -3;
  }


  // Create an GPSLogTask which will register a task with the robot.
  GPSLogTask gpsTask(&robot, gps, joydriveAction.getJoyHandler()->haveJoystick() ? joydriveAction.getJoyHandler() : NULL);


  // Add actions
  robot.addAction(&nearLimitAction, 100);
  robot.addAction(&farLimitAction, 90);
  robot.addAction(&limitBackwardsAction, 80);
  robot.addAction(&joydriveAction, 50);
  robot.addAction(&keydriveAction, 40);

  // allow keydrive action to drive robot even if joystick button isn't pressed
  joydriveAction.setStopIfNoButtonPressed(false);


  robot.lock();

  robot.enableMotors();
  robot.comInt(47, 1);  // enable joystick driving on some robots

  // Add exit callback to reset/unwrap steering wheels on seekur (critical if the robot doesn't have sliprings); does nothing for other robots 
  Aria::addExitCallback(new ArRetFunctor1C<bool, ArRobot, unsigned char>(&robot, &ArRobot::com, (unsigned char)120));
  Aria::addExitCallback(new ArRetFunctor1C<bool, ArRobot, unsigned char>(&robot, &ArRobot::com, (unsigned char)120));

  robot.unlock();

  ArLog::log(ArLog::Normal, "gpsRobotTaskExample: Running... (drive robot with joystick or arrow keys)");
  robot.waitForRunExit();


  return 0;
}
