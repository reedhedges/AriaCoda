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

// This tests standalone use of the LMS2xx class.
// It first tries connecting to a robot and sending it the command
// to switch on power to the primary laser.  Then it connects
// to the SICK LMS2xx using the ArLMS2xx class directly, not using
// ArLaserConnector or ArRobot.

#include "Aria.h"

int main(int argc, char **argv)
{
  Aria::init();

  /* Connect to robot just so we can turn on laser power if neccesary */
  ArArgumentParser parser(&argc, argv);
  ArRobot robot;
  ArRobotConnector robotConnector(&parser, &robot);
  if(robotConnector.connectRobot())
  {
    robot.runAsync(true);
    ArLog::log(ArLog::Normal, "Connected to robot, issusing LRF_POWER command...");
    robot.comInt(ArCommands::POWER_LRF, 1);
    robot.disableSonar();
  }
  else
  {
    ArLog::log(ArLog::Normal, "No robot or unable to connect.");
  }


  /* Now connect to laser. ArRobot is not used. */

  ArSerialConnection con;
  ArLMS2xx laser(0);
  con.setPort(ArUtil::COM3);
  if(con.open(ArUtil::COM3) != 0) {
    ArLog::log(ArLog::Terse, "Error opening serial port %s", con.getPort());
    Aria::exit(1);
  }
  laser.setDeviceConnection(&con);
  laser.setPowerControlled(true);
  laser.chooseAutoBaud("38400");
  laser.chooseDegrees("180");
  laser.chooseIncrement("one");
  laser.runAsync();
  if(!laser.blockingConnect()) {
    ArLog::log(ArLog::Terse, "Error connecting to SICK LMS2xx on %s", con.getPort());
    Aria::exit(2);
  }
  while(laser.isConnected()) {
    ArUtil::sleep(2000);
    ArLog::log(ArLog::Normal, "%d readings/sec.", laser.getReadingCount());
  }
  ArLog::log(ArLog::Normal, "Laser disconnected.");
  Aria::exit(0);
}

