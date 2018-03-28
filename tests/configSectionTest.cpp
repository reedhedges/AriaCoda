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

class ConfigTester 
{
public:
  ConfigTester(const char *section);
  virtual ~ConfigTester();
  bool processFile(void);

  std::string mySection;
  int myInt;
  double myDouble;
  bool myBool;
  ArPose myPose;
  char myString[512];

  // stuff for testing the functor ones
  std::list<std::string> myList;
  std::list<ArArgumentBuilder *> myArgList;
  bool listAdder(ArArgumentBuilder *builder) 
    { myList.push_front(builder->getFullString()); 
    printf("Added %s\n", builder->getFullString()); return true; }
  const std::list<ArArgumentBuilder *> *getList(void) 
    {
      std::list<ArArgumentBuilder *>::iterator argIt;
      std::list<std::string>::iterator listIt;
      ArArgumentBuilder *builder;

      if (myArgList.size() != 0)
      {
	while ((argIt = myArgList.begin()) != myArgList.end())
	{
	  delete (*argIt);
	  myArgList.pop_front();
	}
      }
      for (listIt = myList.begin(); listIt != myList.end(); listIt++)
      {
	builder = new ArArgumentBuilder;
	builder->add((*listIt).c_str());
	myArgList.push_front(builder);
      }
      return &myArgList;
    }
  ArRetFunctor1C<bool, ConfigTester, ArArgumentBuilder *> mySetFunctor;
  ArRetFunctorC<const std::list<ArArgumentBuilder *> *, ConfigTester> myGetFunctor;

};

ConfigTester::ConfigTester(const char *section) : 
  mySetFunctor(this, &ConfigTester::listAdder),
  myGetFunctor(this, &ConfigTester::getList)
{
  myInt = 42;
  myDouble = 42;
  myBool = true;
  myPose.setPose(42, -42.3, 21.21);
  strcpy(myString, "42");
  mySection = section;

  Aria::getConfig()->addParam(
	  ArConfigArg("int", &myInt, "fun things!"), section);//, 0, 300));
  Aria::getConfig()->addParam(
	  ArConfigArg("double", &myDouble, "fun things double!"), section);//, 0, 2300));
  Aria::getConfig()->addParam(
	  ArConfigArg("bool", &myBool, "fun things bool!"), section);
  Aria::getConfig()->addParam(
	  ArConfigArg("string", myString, "fun things string!", sizeof(myString)), section);
  Aria::getConfig()->addParam(
	  ArConfigArg("functor", &mySetFunctor, &myGetFunctor, "fun functor thing!"), section);
}

ConfigTester::~ConfigTester()
{
  std::list<ArArgumentBuilder *>::iterator argIt;
  if (myArgList.size() != 0)
  {
    while ((argIt = myArgList.begin()) != myArgList.end())
    {
      delete (*argIt);
      myArgList.pop_front();
    }
  }
}

bool ConfigTester::processFile(void)
{
  printf("%s: int %d double %g bool %s string '%s'\n", 
	 mySection.c_str(), myInt, myDouble,
	 ArUtil::convertBool(myBool),
	 myString);
  return true;
}

bool func100(void)
{
  printf("100\n");
  return true;
}

bool func90a(void)
{
  printf("90a\n");
  return true;
}

bool func90b(void)
{
  printf("90b\n");
  return true;
}

bool func50(void)
{
  printf("50\n");
  return true;
}

int main(int argc, char **argv)
{
  Aria::init();
  ArLog::init(ArLog::StdOut, ArLog::Normal);
  
  ArArgumentParser parser(&argc, argv);
  ConfigTester tester1("one");
  ConfigTester tester2("two");
  ConfigTester tester3("three");
  bool ret;
  Aria::getConfig()->writeFile("configBefore.txt");
  char errorBuffer[512];
  errorBuffer[0] = '\0';


  ArGlobalRetFunctor<bool> func100cb(&func100);
  ArGlobalRetFunctor<bool> func90acb(&func90a);
  ArGlobalRetFunctor<bool> func90bcb(&func90b);
  ArGlobalRetFunctor<bool> func50cb(&func50);

  func100cb.setName("100cb");
  func90bcb.setName("bcb");
  func50cb.setName("50cb");

  Aria::getConfig()->addProcessFileCB(&func100cb, 100);
  Aria::getConfig()->addProcessFileCB(&func90acb, 90);
  Aria::getConfig()->addProcessFileCB(&func90bcb, 90);
  Aria::getConfig()->addProcessFileCB(&func50cb, 50);

  std::list<std::string> sectionsToParse;
  sectionsToParse.push_back("two");
  sectionsToParse.push_back("three");

  ArLog::init(ArLog::StdOut, ArLog::Verbose);
  Aria::getConfig()->useArgumentParser(&parser);
  ret = Aria::getConfig()->parseFile(
	  "configTest.txt", false, true, errorBuffer, 
	  sizeof(errorBuffer), &sectionsToParse);
  if (ret)
  {
    printf("\nSucceeded test\n");
  }
  else
  {
    printf("\nFailed config test because '%s'\n\n", errorBuffer);
  }

  Aria::getConfig()->writeFile("configAfter.txt");
  exit(0);
}



