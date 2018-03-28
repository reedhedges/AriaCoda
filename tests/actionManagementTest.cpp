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

class TestAction : public ArAction
{
public:
  TestAction(const char * name, const char * description = "") :
    ArAction(name, description) {}
  virtual ~TestAction(void) {}
  ArActionDesired *fire(ArActionDesired currentDesired) { return NULL; }
};

int main(void)
{
  Aria::init();

  TestAction act0("0");
  TestAction act1("1");
  TestAction act2("2");
  
  TestAction act3("3");
  TestAction act4("4");
  TestAction act5("5");
  
  ArAction *act;
  ArRobot robot;
  

  robot.addAction(&act2, 100);
  robot.addAction(&act1, 100);
  robot.addAction(&act0, 100);
  robot.addAction(&act4, 50);
  robot.addAction(&act3, 75);
  robot.addAction(&act5, -30);
  
  printf("###First the action list should print with 6\n");
  printf("--------------------------------------------------------------\n");
  robot.logActions();
  
  printf("###Then the action list should print with 5 (lacking number 4)\n");
  printf("--------------------------------------------------------------\n");
  robot.remAction(&act4);
  robot.logActions();

  printf("###Then the action list should print with 4 (lacking number 1)\n");
  printf("--------------------------------------------------------------\n");
  robot.remAction("1");
  robot.logActions();

  printf("###Then the action list should again print with 4\n");
  printf("--------------------------------------------------------------\n");
  robot.logActions();

  printf("###Now action 0 should be printed.\n");
  if ((act = robot.findAction("0")) != NULL)
    act->log();

  printf("###Now action 3 should be printed.\n");
  if ((act = robot.findAction("3")) != NULL)
    act->log();

  printf("end of program.\n");
  Aria::exit(0);
}
