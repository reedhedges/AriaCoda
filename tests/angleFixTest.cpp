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

/* See also angleBetweenTest, angleTest, mathTests. */

void fixTest(double th, double sth)
{
  double cth;
  if ((cth = ArMath::fixAngle(th)) == sth)
    printf("GOOD fix of %f resulted in %f\n", th, cth);
  else
  {
    printf("bad fix of %f resulted in %f instead of %f\n", th, cth, sth);
    printf("Failed tests\n");
    exit(1);
  }
}

int main(void)
{

  printf("Some tests of fixAngle\n");
  fixTest(405, 45);
  fixTest(-405, -45);
  fixTest(450, 90);
  fixTest(-450, -90);
  fixTest(495, 135);
  fixTest(-495, -135);
  fixTest(540, 180);
  fixTest(-540, 180);
  fixTest(585, -135);
  fixTest(-585, 135);
  fixTest(630, -90);
  fixTest(-630, 90);
  fixTest(675, -45);
  fixTest(-675, 45);
  fixTest(720, 0);
  fixTest(-720, 0);
  fixTest(765, 45);
  fixTest(-765, -45);

  printf("\n");
  fixTest(225, -135);
  fixTest(-225, 135);
  fixTest(315, -45);
  fixTest(-315, 45);
  fixTest(270, -90);
  fixTest(-270, 90);
  fixTest(-180, 180);
  fixTest(180, 180);
  
  printf("\nPassed all tests!!\n");
}
