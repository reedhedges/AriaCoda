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

#include "ariaUtil.h"
#include <stdio.h>
#include <assert.h>
#include <cmath>

/* Also see angleBetweenTest.cpp, angleFixTest.cpp and anglesTest.cpp for the angle comparison functions. */

int main(int argc, char ** argv)
{
  // Check that ArMath::roundInt is a correct replacement for rint():
  assert(rint(0.001) == ArMath::roundInt(0.001));
  assert(rint(0.5) == ArMath::roundInt(0.5));
  assert(rint(0.9999) == ArMath::roundInt(0.999));
  assert(rint(0.449) == ArMath::roundInt(0.449));
  assert(rint(999999.9999) == ArMath::roundInt(999999.999));
  assert(ArMath::roundInt(INT_MAX) == INT_MAX);
  assert(ArMath::roundInt(INT_MIN) == INT_MIN);
  assert(rint(0) == ArMath::roundInt(0));
  assert(rint(0.000000001) == ArMath::roundInt(0.000000001));
  assert(ArMath::isFinite( (float)23.0 ));
  assert(ArMath::isFinite( (double)42.0 ));
  assert(!ArMath::isFinite(sqrt(-1.0)));
  assert(ArMath::isNan(nan("")));
  assert(ArMath::isNan(nanf("")));
  assert(!ArMath::isNan( 123.2 ));
  assert(ArMath::compareFloats(0.01, 0.02, 0.05));
  ArLog::log(ArLog::Normal, "All tests completed.");
  return 0; 
}
