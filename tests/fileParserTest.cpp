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
  This file tests the fileParser class and is a rudimentary example
*/
   
bool boolPrinter(ArArgumentBuilder *builder)
{
  if (!builder->isArgBool(0))
  {
    printf("Bad boolean value\n");
    return false;
  }
  if (builder->getArgBool(0))
    printf("bool(%s): true\n", builder->getExtraString());
  else 
    printf("bool(%s): false\n", builder->getExtraString());

  return true;
}

bool intPrinter(ArArgumentBuilder *builder)
{
  if (!builder->isArgInt(0))
  {
    printf("Bad integer value\n");
    return false;
  }
  printf("int: %d\n", builder->getArgInt(0));
  return true;
}

bool doublePrinter(ArArgumentBuilder *builder)
{
  if (!builder->isArgDouble(0))
  {
    printf("Bad double value\n");
    return false;
  }
  printf("double: %g\n", builder->getArgDouble(0));
  return true;
}

bool stringPrinter(ArArgumentBuilder *builder)
{
  printf("string: %s\n", builder->getFullString());
  return true;
}

bool argPrinter(ArArgumentBuilder *builder)
{
  printf("Logging builder:\n");
  builder->log();
  return true;
}

int main(int argc, char **argv)
{
  ArGlobalRetFunctor1<bool, ArArgumentBuilder *> boolFunctor(&boolPrinter);
  ArGlobalRetFunctor1<bool, ArArgumentBuilder *> intFunctor(&intPrinter);
  ArGlobalRetFunctor1<bool, ArArgumentBuilder *> doubleFunctor(&doublePrinter);
  ArGlobalRetFunctor1<bool, ArArgumentBuilder *> stringFunctor(&stringPrinter);
  ArGlobalRetFunctor1<bool, ArArgumentBuilder *> argFunctor(&argPrinter);
  ArFileParser parser(Aria::getDirectory());
  //ArLog::init(ArLog::StdOut, ArLog::Verbose);

  parser.addHandler("keywordbool", &boolFunctor);
  // make sure it won't let us add another one
  parser.addHandler("keywordbool", &boolFunctor);
  parser.addHandler("keywordint", &intFunctor);
  // was testing the remHandler
  //parser.remHandler("keywordint");
  //parser.remHandler(&intFunctor);
  parser.addHandler("keyworddouble", &doubleFunctor);
  parser.addHandler("keywordstring", &stringFunctor);
  parser.addHandler(NULL, &stringFunctor);
  parser.addHandler("keywordbuilder", &argFunctor);
  // either parse the bad file 
  //parser.parseFile("tests/fileParserTestBad.txt");
  // or parse the good file 
  parser.parseFile("tests/fileParserTestGood.txt", false);
}



