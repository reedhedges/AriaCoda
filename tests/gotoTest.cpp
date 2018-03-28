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

/*
  This program will just have the robot drive a couple of meters forwards.

  You can press escape while it was running to cause the program to
  close up and exit.
*/

int main(void)
{
  // the serial connection (robot)
  ArSerialConnection serConn;
  // tcp connection (sim)
  ArTcpConnection tcpConn;
  
  // robot
  ArRobot robot;
  // sonar, must be added to the robot
  ArSonarDevice sonar;

  // limiter for close obstacles
  ArActionLimiterForwards limiter("speed limiter near", 300, 600, 250);
  // limiter for far away obstacles
  ArActionLimiterForwards limiterFar("speed limiter far", 300, 1100, 400);
  // limiter so we don't bump things backwards
  // limiter for far away obstacles
  ArActionLimiterTableSensor tableLimiter;
  // goto the goal
  ArActionGoto gotoPose("goto", ArPose(2000, 0));
  // Make a key handler, so that escape will shut down the program
  // cleanly
  ArKeyHandler keyHandler;

  // mandatory init
  Aria::init();
  
  // Add the key handler to Aria so other things can find it
  Aria::setKeyHandler(&keyHandler);

  // Attach the key handler to a robot now, so that it actually gets
  // some processing time so it can work, this will also make escape
  // exit
  robot.attachKeyHandler(&keyHandler);


  // First we see if we can open the tcp connection, if we can we'll
  // assume we're connecting to the sim, and just go on...  if we
  // can't open the tcp it means the sim isn't there, so just try the
  // robot

  // modify this next line if you're not using default tcp connection
  tcpConn.setPort();

  // see if we can get to the simulator  (true is success)
  if (tcpConn.openSimple())
  {
    // we could get to the sim, so set the robots device connection to the sim
    printf("Connecting to simulator through tcp.\n");
    robot.setDeviceConnection(&tcpConn);
  }
  else
  {
    // we couldn't get to the sim, so set the port on the serial
    // connection and then set the serial connection as the robots
    // device

    // modify the next line if you're not using the first serial port
    // to talk to your robot
    serConn.setPort();
    printf(
      "Could not connect to simulator, connecting to robot through serial.\n");
    robot.setDeviceConnection(&serConn);
  }
  
  
  // add the sonar to the robot
  robot.addRangeDevice(&sonar);
  
  // try to connect, if we fail exit
  if (!robot.blockingConnect())
  {
    printf("Could not connect to robot... exiting\n");
    Aria::shutdown();
    return 1;
  }

  // turn on the motors, turn off amigobot sounds
  robot.comInt(ArCommands::ENABLE, 1);
  robot.comInt(ArCommands::SOUNDTOG, 0);

  // add the actions
  robot.addAction(&tableLimiter, 100);
  robot.addAction(&limiter, 95);
  robot.addAction(&limiterFar, 90);
  robot.addAction(&gotoPose, 50);
  
  // start the robot running, true so that if we lose connection the run stops
  robot.run(true);
  
  // now exit
  Aria::shutdown();
  return 0;
}
