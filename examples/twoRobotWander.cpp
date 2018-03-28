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

/** @example twoRobotWander.cpp Example showing how one program can connect to
   two robots using TCP network connection via ArTcpConnection objects (in simulator or to Amigobot or Pioneer with
   wireless-serial bridge device).

  To use with MobileSim, expand "More Options" and create 2 robots, and select
  a map.   Or run from the command line like this:
    MobileSim -r p3dx-sh -r p3dx-sh -map ../maps/columbia.map
 
  Normally, a program will only connect to one robot, and will only have
  one ArRobot object.   However, it is possible to connect to more than
  one robot from the same program -- for example, this may be useful for doing 
  multirobot experiments in a simulator, or to control remote robots with
  wireless-serial bridge devices from an offboard computer. This program 
  demonstrates this by connecting to two robots over TCP connections. 
  (Old systems with dedicated serial-radio links for each robot would require 
  modifying this program to use serial connections instead.)

  Specify the hostnames of each robot with -rh1 and -rh2 command line arguments
  (defaults are "localhost" for each). Specify port numbers for each with
  -rp1 and -rp2 command line arguments (defaults are 8101 for each if 
  the hostnames are different, or 8101 and 8102 if the hostnames are the same).

  This program will just have two robots wander around, avoiding obstacles
  detected by the sonar, or otherwise moving forward at constant velocity.
  First it has one robot wander for a period of time, then it has the 
  second robot wander around.
*/

int main(int argc, char** argv)
{

  // initialize ARIA
  Aria::init();

  // used to parse command-line arguments. only one instance needed.
  ArArgumentParser argParser(&argc, argv);

  // get hostnames and port numbers for connecting to the robots.
  const char* host1 = argParser.checkParameterArgument("-rh1");
  if(!host1) host1 = "localhost";
  const char* host2 = argParser.checkParameterArgument("-rh2");
  if(!host2) host2 = "localhost";
  int port1 = 8101;
  int port2 = 8101;
  if(strcmp(host1, host2) == 0 )
  {
    // same host, it must be using two ports (but can still override below with -rp2)
    port2++;
  }

  bool argSet = false;
  argParser.checkParameterArgumentInteger("-rp1", &port1, &argSet);  
  if(!argSet) argParser.checkParameterArgumentInteger("-rrtp1", &port1);
  argSet = false;
  argParser.checkParameterArgumentInteger("-rp2", &port2, &argSet);
  if(!argSet) argParser.checkParameterArgumentInteger("-rrtp2", &port2);
  
  if(!argParser.checkHelpAndWarnUnparsed())
  {
    ArLog::log(ArLog::Terse, "Usage: twoRobotWander [-rh1 <hostname1>] [-rh2 <hostname2>] [-rp1 <port1>] [-rp2 <port2>]\n"\
      "\t<hostname1> Is the network host name of the first robot."\
          " Default is localhost (for the simulator).\n"\
      "\t<hostname2> Is the network host name of the second robot."\
          " Default is localhost (for the simulator).\n"\
      "\t<port1> Is the TCP port number of the first robot. Default is 8101.\n"\
      "\t<port2> Is the TCP port number of the second robot. Default is 8102 if"\
        " both robots have the same hostname, or 8101 if they differ.\n\n");
    return 1;
  }

  

  //
  // The first robot's objects
  //
  // the first robot connection
  ArTcpConnection con1;
  // the first robot interface
  ArRobot robot1;
  // sonar interface
  ArSonarDevice sonar1;
  // the actions we'll use to cause wander behavior for the first robot
  ArActionStallRecover recover1;
  ArActionBumpers bumpers1;
  ArActionAvoidFront avoidFront1;
  ArActionConstantVelocity constantVelocity1("Constant Velocity", 400);

  //
  // The second robot's objects
  //
  // the second robot connection
  ArTcpConnection con2;
  // the second robot
  ArRobot robot2;
  // sonar
  ArSonarDevice sonar2;
  // the actions we'll use to wander for the second robot
  ArActionStallRecover recover2;
  ArActionBumpers bumpers2;
  ArActionAvoidFront avoidFront2;
  ArActionConstantVelocity constantVelocity2("Constant Velocity", 400);


  //
  // Lets get robot 1 going
  //
  int ret;
  std::string str;

  // open the connection, if this fails exit
  ArLog::log(ArLog::Normal, "Connecting to first robot at %s:%d...", host1, port1);
  if ((ret = con1.open(host1, port1)) != 0)
  {
    str = con1.getOpenMessage(ret);
    printf("Open failed to robot 1: %s\n", str.c_str());
    Aria::exit(1);
    return 1;
  }
  
  // add the sonar to the robot
  robot1.addRangeDevice(&sonar1);
  
  // set the device connection on the robot
  robot1.setDeviceConnection(&con1);
  
  // try to connect, if we fail exit
  if (!robot1.blockingConnect())
  {
    printf("Could not connect to robot 1... exiting\n");
    Aria::exit(1);
    return 1;
  }

  // turn on the motors, turn off amigobot sounds
  robot1.comInt(ArCommands::ENABLE, 1);
  robot1.comInt(ArCommands::SOUNDTOG, 0);

  // add the actions
  robot1.addAction(&recover1, 100);
  robot1.addAction(&bumpers1, 75);
  robot1.addAction(&avoidFront1, 50);
  robot1.addAction(&constantVelocity1, 25);


  //
  // Lets get robot 2 going
  //

  // open the connection, if this fails exit
  ArLog::log(ArLog::Normal, "Connecting to second robot at %s:%d...", host2, port2);
  if ((ret = con2.open(host2, port2)) != 0)
  {
    str = con2.getOpenMessage(ret);
    printf("Open failed to robot 2: %s\n", str.c_str());
    Aria::exit(1);
    return 1;
  }
  
  // add the sonar to the robot
  robot2.addRangeDevice(&sonar2);
  
  // set the device connection on the robot
  robot2.setDeviceConnection(&con2);
  
  // try to connect, if we fail exit
  if (!robot2.blockingConnect())
  {
    printf("Could not connect to robot 2... exiting\n");
    Aria::exit(1);
    return 1;
  }

  // turn on the motors, turn off amigobot sounds
  robot2.comInt(ArCommands::ENABLE, 1);
  robot2.comInt(ArCommands::SOUNDTOG, 0);

  // add the actions
  robot2.addAction(&recover2, 100);
  robot2.addAction(&bumpers2, 75);
  robot2.addAction(&avoidFront2, 50);
  robot2.addAction(&constantVelocity2, 25);
  
  // start the robots running. true so that if we lose connection to either
  // robot, the run stops.
  robot1.runAsync(true);
  robot2.runAsync(true);

  // As long as both robot loops are running, lets alternate between the
  // two wandering around every 10 sec.  mutex lock/unlock calls are neccesary since 
  // the ArRobot objects are running in background threads after runAsync()
  // calls above.
  int curRobot=1;
  while (robot1.isRunning() && robot2.isRunning())
  {
    ArUtil::sleep(10000);
    robot1.lock();
    robot2.lock();
    if (curRobot == 1)
    {
      ArLog::log(ArLog::Normal, "Switching to robot 2.");
      robot1.stop();
      robot2.clearDirectMotion();
      curRobot=2;
    }
    else
    {
      ArLog::log(ArLog::Normal, "Switching to robot 1.");
      robot2.stop();
      robot1.clearDirectMotion();
      curRobot=1;
    }
    robot2.unlock();
    robot1.unlock();
  }
  
  
  // exit program if both robots disconnect.
  ArLog::log(ArLog::Normal, "Both robots disconnected.");
  Aria::exit(0);
  return 0;
}
