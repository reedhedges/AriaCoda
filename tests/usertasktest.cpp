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


class UserTaskTest
{
public:
  UserTaskTest(void) {};
  ~UserTaskTest(void) {};
  void userTaskOne(void) { printf("One\n"); }
  void userTaskTwo(void) { printf("Two\n"); }
  void userTaskThree(void) { printf("Three\n"); }
  void userTaskFour(void) { printf("Four\n"); }
  ArTaskState::State myTaskFourState;
};

int main()
{
  ArSyncTask *task;
  UserTaskTest test;
  ArRobot robot;

  ArFunctorC<UserTaskTest> userTaskOne(&test, &UserTaskTest::userTaskOne);
  ArFunctorC<UserTaskTest> userTaskTwo(&test, &UserTaskTest::userTaskTwo);
  ArFunctorC<UserTaskTest> userTaskThree(&test, &UserTaskTest::userTaskThree);
  ArFunctorC<UserTaskTest> userTaskFour(&test, &UserTaskTest::userTaskFour);


  printf("Before tasks added:\n");
  robot.logUserTasks();
  printf("---------------------------------------------------------------\n");

  // the order you add tasks doesn't matter, it goes off of the integer
  // you give the function call to add them 
  // Caveat: if you give the function call the same number it goes off of order
  robot.addUserTask("procTwo", 20, &userTaskTwo);
  robot.addUserTask("procFour", 40, &userTaskFour, &test.myTaskFourState);
  robot.addUserTask("procThree", 30, &userTaskThree);
  robot.addUserTask("procOne", 10, &userTaskOne);

  printf("After tasks added:\n");
  robot.logUserTasks();
  printf("---------------------------------------------------------------\n");
  printf("What happens when these are run:\n");
  robot.loopOnce();

  printf("---------------------------------------------------------------\n");

  printf("After tasks removed by name:\n");
  printf("---------------------------------------------------------------\n");
  robot.remUserTask("procOne");
  robot.remUserTask("procTwo");
  robot.remUserTask("procThree");
  robot.remUserTask("procFour");
  robot.logUserTasks();


  printf("\nAfter they are added again, procThree is found by name and set to SUSPEND and procFour is found by functor and set to FAILURE:\n");
  printf("---------------------------------------------------------------\n");

  printf("---------------------------------------------------------------\n");
  robot.addUserTask("procTwo", 20, &userTaskTwo);
  robot.addUserTask("procFour", 40, &userTaskFour, &test.myTaskFourState);
  robot.addUserTask("procThree", 30, &userTaskThree);
  robot.addUserTask("procOne", 10, &userTaskOne);
  task = robot.findUserTask("procThree");
  if (task != NULL)
    task->setState(ArTaskState::SUSPEND);

  task = robot.findUserTask(&userTaskFour);
  if (task != NULL) 
    task->setState(ArTaskState::FAILURE);

  robot.logUserTasks();

  task = robot.findUserTask(&userTaskFour);
  if (task != NULL)
  {
    printf("---------------------------------------------------------------\n");

    printf("Did the real state on procFourState get set:\n");
    printf("getState: %d realState: %d\n", task->getState(), 
	   test.myTaskFourState);
  }


  printf("---------------------------------------------------------------\n");
  printf("What happens when these are run:\n");
  robot.loopOnce();

  printf("---------------------------------------------------------------\n");


  printf("After tasks removed by functor:\n");
  printf("---------------------------------------------------------------\n");
  robot.remUserTask(&userTaskOne);
  robot.remUserTask(&userTaskTwo);
  robot.remUserTask(&userTaskThree);
  robot.remUserTask(&userTaskFour);
  robot.logUserTasks();
  
  
}
