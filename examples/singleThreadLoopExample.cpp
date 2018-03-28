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

/** @example singleThreadLoopExample.cpp example showing how to run ArRobot thread
 * from your own loop, without running a separate thread.  Only use this
 * technique if your platform/system does not support threading. 
 */

int main(int argc, char **argv)
{
  Aria::init();
  ArRobot robot;

  // Connect to the robot, get some initial data from it such as type and name,
  // and then load parameter files for this robot.
  ArRobotConnector robotConnector(NULL, &robot);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "singleThreadExample: Could not connect to the robot.");
    Aria::exit(1);
    return 1;
  }


  ArLog::log(ArLog::Normal, "singleThreadExample: Connected to robot.");

  robot.enableMotors();

  robot.setVel(300);
  robot.setRotVel(10);

  while(true)
  {
    ArTime t;
    // Run one iteration of the ArRobot loop
    robot.loopOnce();

    
    // Print out some data from the SIP.  
    bool soc = robot.hasStateOfCharge();
    float battv = 0.0;
    if(soc)
      battv = robot.getStateOfCharge();
    else
      battv = robot.getBatteryVoltage();
    ArLog::log(ArLog::Normal, "singleThreadExample: Pose=(%.2f,%.2f,%.2f), Trans.  Vel=%.2f, Battery=%.2f%c", robot.getX(), robot.getY(), robot.getTh(), robot.getVel(), battv, soc?'%':'V');




    // slow the loop down a little bit.
    
    unsigned int sleepTime = 20;
    if(t.mSecSince() > 20)
      sleepTime = 0;
    else
      sleepTime -= t.mSecSince();
    ArLog::log(ArLog::Normal, "singleThreadExample: loop took %lu ms, sleeping %lu ms", t.mSecSince(), sleepTime);

    if(t.mSecSince() > 100)
      ArLog::log(ArLog::Normal, "singleThreadExample: warning: loop took longer than 100ms");

    ArUtil::sleep(sleepTime);
  }

  return 0;
}
