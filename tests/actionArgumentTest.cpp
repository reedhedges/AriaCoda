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

class TestAction0 : public ArAction
{
public:
  TestAction0(const char *name) : ArAction(name) 
    {}; 
  virtual ~TestAction0(void) {}
  ArActionDesired *fire(ArActionDesired currentDesired) { return NULL; }
};

class TestAction1 : public ArAction
{
public:
  TestAction1(const char *name, const char *description = "") : 
    ArAction(name, description)
    { 
      setNextArgument(ArArg("first", &myDouble)); 
      myDouble = 1;
    }; 
  virtual ~TestAction1(void) {}
  ArActionDesired *fire(ArActionDesired currentDesired) { return NULL; }
  double myDouble;
};


class TestAction3 : public ArAction
{
public:
  TestAction3(const char *name, const char *description = "") :
    ArAction(name, description)
    { 
      setNextArgument(ArArg("first", &myDouble, "That infamous double")); 
      setNextArgument(ArArg("second", &myInt)); 
      setNextArgument(ArArg("third", myString, "An infamous string", sizeof(myString)));
      myDouble = 3;
      myInt = 3;
      strcpy(myString, "3");
    }; 
  virtual ~TestAction3(void) {}
  ArActionDesired *fire(ArActionDesired currentDesired) { return NULL; }
  double myDouble;
  int myInt; 
  char myString[512];
};




int main(void)
{
  ArArg *arg;

  printf("Test0 should have no arguments\nIt should have no description.\n----------------------------------\n");
  TestAction0 ta0("Test0");
  ta0.log();
  printf("Test1 should have one argument.\nA double equal to 1.\nIt should have no description.\n----------------------------------\n");
  TestAction1 ta1("Test1");
  ta1.log();
  printf("Test3 should have 3 arguments.\nA double, an int, and a string equal to 3.\nIt should also have a description.\n----------------------------------\n");
  TestAction3 ta3("Test3", "The real test class");
  ta3.log();

  printf("Okay, now the automated test bit:\n");
  arg = ta3.getArg(0);
  if (arg->getType() == ArArg::DOUBLE && arg->getDouble() == 3 && 
      ta3.myDouble == 3)
    printf("Double argument type and equality: passed.\n");
  else
  {
    printf("Double argument type and equality: FAILED.\n");
    exit(1);
  }
  arg->setDouble(6);
  printf("Double argument setting:  %f %f:", arg->getDouble(), ta3.myDouble);
  if (arg->getDouble() == 6 && ta3.myDouble == 6)
    printf(" passed\n");
  else
  {
    printf(" FAILED\n");
    exit(1);
  }

  arg = ta3.getArg(1);
  if (arg->getType() == ArArg::INT && arg->getInt() == 3 && 
      ta3.myInt == 3)
    printf("Int argument type and equality: passed.\n");
  else
  {
    printf("Int argument type and equality: FAILED.\n");
    exit(1);
  }
  arg->setInt(6);
  printf("Int argument setting:  %d %d:", arg->getInt(), ta3.myInt);
  if (arg->getInt() == 6 && ta3.myInt == 6)
    printf(" passed\n");
  else
  {
    printf(" FAILED\n");
    exit(1);
  }

  arg = ta3.getArg(2);
  if (arg->getType() == ArArg::STRING && strcmp(arg->getString(), "3") == 0 && 
      strcmp(ta3.myString, "3") == 0)
    printf("String argument type and equality: passed.\n");
  else
  {
    printf("String argument type and equality: FAILED.\n");
    exit(1);
  }
  
  arg->setString("6");
  printf("String argument setting:  %s %s:", arg->getString(), 
	 ta3.myString);
  if (strcmp(arg->getString(), "6") == 0 && strcmp(ta3.myString, "6") == 0)
    printf(" passed\n");
  else
  {
    printf(" FAILED\n");
    exit(1);
  }

  printf("\nAll tests PASSED\n");
}
