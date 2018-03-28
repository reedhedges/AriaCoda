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

/** @example armExample.cpp Example program showing use of the original 5DOF P2 arm accessory.
 *
 *  Demonstrates how to connect with the Pioneer 2 controller
 *  and set up the P2Arm class, including the ARMpac handler.
 *  It simply queries and prints the status of the arm, moving it to a position, then exits.
 *  Note, The P2Arm class is only for use with the microcontroller-integrated original 5DOF 
 *  Pioneer 2 arm (Robotica arm).  The newer Energid Cyton 7DOF arm is accessed through 
 *  separate cyton libraries.
 *
*/

int main(int argc, char **argv)
{
  Aria::init();
  ArSimpleConnector con(&argc, argv);
  ArRobot robot;
  ArP2Arm arm;


  if(!Aria::parseArgs()) 
  {
    Aria::logOptions();
    Aria::exit(1);
    return 1;
  }

  ArLog::log(ArLog::Normal, "armExample: Connecting to the robot.");
  if(!con.connectRobot(&robot))
  {
    ArLog::log(ArLog::Terse, "armExample: Could not connect to the robot. Exiting.");
    Aria::exit(1);
    return 1;
  }
  robot.runAsync(true);

  // turn off sonar
  robot.comInt(28, 0);

  // Set up and initialize the arm
  arm.setRobot(&robot);
  if (arm.init() != ArP2Arm::SUCCESS)
  {
    ArLog::log(ArLog::Terse, "armExample: Error initializing the P2 Arm!");
    return 1;
  }

  // Print out some of the settings
  P2ArmJoint *joint;
  printf("Current joint info:\nJoint   Vel  Home  Center\n");
  for (int i=1; i<=ArP2Arm::NumJoints; i++)
  {
    joint = arm.getJoint(i);
    printf("  %2i:  %5i %5i   %5i\n", i, joint->myVel, joint->myHome, joint->myCenter);
  }
  printf("\n");

  // Put the arm to work
  printf("Powering on  (takes a couple seconds to stabilize)\n");
  arm.powerOn();

  // Request one status packet and print out the arm's status
  printf("Current arm status:\n");
  arm.requestStatus(ArP2Arm::StatusSingle);
  ArUtil::sleep(200);  // Give time to get the packet
  printf("Arm Status: ");
  if (arm.getStatus() & ArP2Arm::ArmGood)
    printf("Good=1 ");
  else
    printf("Good=0 ");
  if (arm.getStatus() & ArP2Arm::ArmInited)
    printf("Inited=1 ");
  else
    printf("Inited=0 ");
  if (arm.getStatus() & ArP2Arm::ArmPower)
    printf("Power=1 ");
  else
    printf("Power=0 ");
  if (arm.getStatus() & ArP2Arm::ArmHoming)
    printf("Homing=1 ");
  else
    printf("Homing=0 ");
  printf("\n\n");

  // Move the arm joints to specific positions
  printf("Moving Arm...\n");
  int deploy_offset[] = {0, -100, 10, 40, 80, -55, 20};
  for (int i=1; i<=ArP2Arm::NumJoints; i++)
  {
    joint = arm.getJoint(i);
    arm.moveToTicks(i, joint->myCenter + deploy_offset[i]);
  }

  // Wait for arm to achieve new position, printing joint positions and M for
  // moving, NM for not moving.
  arm.requestStatus(ArP2Arm::StatusContinuous);
  ArUtil::sleep(300); // wait a moment for arm status packet update with joints moving
  bool moving = true;
  while (moving)
  {
      moving = false;
      printf("Joints: ");
      for (int i=1; i<=ArP2Arm::NumJoints; i++)
      {
        printf("[%d] %.0f, ", i, arm.getJointPos(i));
        if (arm.getMoving(i))
        {
	        printf("M;  ");
	        moving = true;
	      }
        else
        {
          printf("NM; ");
        }
	    }
      printf("\r");
  }
  printf("\n\n");

  // Return arm to park, wait, and disconnect. (Though the arm will automatically park
  // on client disconnect)
  printf("Parking arm.\n");
  arm.park();

  // Wait 5 seconds or until arm shuts off
  for(int i = 5; (i > 0) && (arm.getStatus() & ArP2Arm::ArmPower); i--)
  {
    ArUtil::sleep(1000);
  }

  // Disconnect from robot, etc., and exit.
  printf("Shutting down ARIA and exiting.\n");
  Aria::exit(0);
  return(0);
}

