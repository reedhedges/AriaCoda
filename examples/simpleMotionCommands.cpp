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

/** @example simpleMotionCommands.cpp example showing how to connect and send
 * basic motion commands to the robot
 *
 * ARIA provides two levels of robot motion control, direct motion commands, and
 * actions. This example shows direct motion commands. See actionExample.cpp,
 * actionGroupExample.cpp, and others for examples on how to use actions.
 * Actions provide a more modular way of performing more complex motion
 * behaviors than the simple imperitive style used here.  
 *
 * See the ArRobot class documentation, as well as the overview of robot motion,
 * for more information.
 *
 * WARNING: this program does no sensing or avoiding of obstacles, the robot WILL
 * collide with any objects in the way!   Make sure the robot has about 2-3
 * meters of free space around it before starting the program.
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

  ArLog::log(ArLog::Terse, "WARNING: this program does no sensing or avoiding of obstacles, the robot WILL collide with any objects in the way! Make sure the robot has approximately 3 meters of free space on all sides.");

  // ArRobotConnector connects to the robot, get some initial data from it such as type and name,
  // and then loads parameter files for this robot.
  ArRobotConnector robotConnector(&parser, &robot);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "simpleMotionCommands: Could not connect to the robot.");
    if(parser.checkHelpAndWarnUnparsed())
    {
        Aria::logOptions();
        Aria::exit(1);
        return 1;
    }
  }
  if (!Aria::parseArgs())
  {
    Aria::logOptions();
    Aria::exit(1);
    return 1;
  }
  
  ArLog::log(ArLog::Normal, "simpleMotionCommands: Connected.");

  // Start the robot processing cycle running in the background.
  // True parameter means that if the connection is lost, then the 
  // run loop ends.
  robot.runAsync(true);

  // Print out some data from the SIP.  

  // We must "lock" the ArRobot object
  // before calling its methods, and "unlock" when done, to prevent conflicts
  // with the background thread started by the call to robot.runAsync() above.
  // See the section on threading in the manual for more about this.
  // Make sure you unlock before any sleep() call or any other code that will
  // take some time; if the robot remains locked during that time, then
  // ArRobot's background thread will be blocked and unable to communicate with
  // the robot, call tasks, etc.
  
  robot.lock();
  ArLog::log(ArLog::Normal, "simpleMotionCommands: Pose=(%.2f,%.2f,%.2f), Trans. Vel=%.2f, Rot. Vel=%.2f, Battery=%.2fV",
    robot.getX(), robot.getY(), robot.getTh(), robot.getVel(), robot.getRotVel(), robot.getBatteryVoltage());
  robot.unlock();

  // Sleep for 3 seconds.
  ArLog::log(ArLog::Normal, "simpleMotionCommands: Will start driving in 3 seconds...");
  ArUtil::sleep(3000);

  // Set forward velocity to 50 mm/s
  ArLog::log(ArLog::Normal, "simpleMotionCommands: Driving forward at 250 mm/s for 5 sec...");
  robot.lock();
  robot.enableMotors();
  robot.setVel(250);
  robot.unlock();
  ArUtil::sleep(5000);

  ArLog::log(ArLog::Normal, "simpleMotionCommands: Stopping.");
  robot.lock();
  robot.stop();
  robot.unlock();
  ArUtil::sleep(1000);

  ArLog::log(ArLog::Normal, "simpleMotionCommands: Rotating at 10 deg/s for 5 sec...");
  robot.lock();
  robot.setRotVel(10);
  robot.unlock();
  ArUtil::sleep(5000);

  ArLog::log(ArLog::Normal, "simpleMotionCommands: Rotating at -10 deg/s for 10 sec...");
  robot.lock();
  robot.setRotVel(-10);
  robot.unlock();
  ArUtil::sleep(10000);

  ArLog::log(ArLog::Normal, "simpleMotionCommands: Driving forward at 150 mm/s for 5 sec...");
  robot.lock();
  robot.setRotVel(0);
  robot.setVel(150);
  robot.unlock();
  ArUtil::sleep(5000);

  ArLog::log(ArLog::Normal, "simpleMotionCommands: Stopping.");
  robot.lock();
  robot.stop();
  robot.unlock();
  ArUtil::sleep(1000);


  // Other motion command functions include move(), setHeading(),
  // setDeltaHeading().  You can also adjust acceleration and deceleration
  // values used by the robot with setAccel(), setDecel(), setRotAccel(),
  // setRotDecel().  See the ArRobot class documentation for more.

  
  robot.lock();
  ArLog::log(ArLog::Normal, "simpleMotionCommands: Pose=(%.2f,%.2f,%.2f), Trans. Vel=%.2f, Rot. Vel=%.2f, Battery=%.2fV",
    robot.getX(), robot.getY(), robot.getTh(), robot.getVel(), robot.getRotVel(), robot.getBatteryVoltage());
  robot.unlock();

  
  ArLog::log(ArLog::Normal, "simpleMotionCommands: Ending robot thread...");
  robot.stopRunning();

  // wait for the thread to stop
  robot.waitForRunExit();

  // exit
  ArLog::log(ArLog::Normal, "simpleMotionCommands: Exiting.");
  Aria::exit(0);
  return 0;
}
