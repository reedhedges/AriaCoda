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

int main(void)
{
  ArRobot *r;

  ArRobot robot;
  printf("%s\n", robot.getName());
  if (strcmp(robot.getName(), "robot") == 0)
    printf("SUCCESS: 'robot's name successfully set\n");
  else
  {
    printf("FAILURE: 'robot's name not successfully set\n");
    exit(1);
  }
  ArRobot robot2;
  printf("%s\n", robot2.getName());
  if (strcmp(robot2.getName(), "robot2") == 0)
    printf("SUCCESS: 'robot2's name successfully set\n");
  else
  {
    printf("FAILURE: 'robot2's name not successfully set\n");
    exit(1);
  }
  ArRobot robot3;
  printf("%s\n", robot3.getName());
  if (strcmp(robot3.getName(), "robot3") == 0)
    printf("SUCCESS: 'robot3's name successfully set\n");
  else
  {
    printf("FAILURE: 'robot3's name not successfully set\n");
    exit(1);
  }

  r = Aria::findRobot("robot");
  if (r != NULL && strcmp(r->getName(), "robot") == 0)
    printf("SUCCESS: Found a the robot named 'robot' successfully.\n");
  else
  {
    printf("FAILURE: could not find the robot named 'robot'\n");
    exit(1);
  }

  r = Aria::findRobot("Bleargh");
  if (r == NULL)
    printf("SUCCESS: didn't find any robot named bleargh\n");
  else
  {
    printf("FAILURE: found a robot with the name bleargh\n");
    exit(1);
  }
  
  robot.setName(NULL);
  printf("Resetting the name of 'robot'\n");
  printf("%s\n", robot.getName());
  if (strcmp(robot.getName(), "robot") == 0)
    printf("SUCCESS: 'robot's name successfully reset\n");
  else
  {
    printf("FAILURE: 'robot's name not successfully reset\n");
    exit(1);
  }

  robot2.setName(NULL);
  printf("Resetting the name of 'robot2'\n");
  printf("%s\n", robot2.getName());
  if (strcmp(robot2.getName(), "robot2") == 0)
    printf("SUCCESS: 'robot2's name successfully reset\n");
  else
  {
    printf("FAILURE: 'robot2's name not successfully reset\n");
    exit(1);
  }

  robot.setName("BobBot");
  printf("Resetting the name of 'robot' to 'BobBot'\n");
  printf("%s\n", robot.getName());
  if (strcmp(robot.getName(), "BobBot") == 0)
    printf("SUCCESS: 'robot's name successfully changed to 'BobBot'\n");
  else
  {
    printf("FAILURE: 'robot's name not successfully changed to 'BobBot'");
    exit(1);
  }
}
