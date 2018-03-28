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
  ArInterpolation interp;
  ArTime t;
  ArPose p;
  int i;
  int result;

  for (i = 1; i <= 6; i++)
  {
    t.setMSec(i*100);
    p.setPose(i*100, i*100, (i - 1)*60);
    interp.addReading(t, p);
  }

  t.setMSec(250);
  result = interp.getPose(t, &p);
  printf("Result of 250 %d\n\n", result);
  if (result)
    p.log();

  t.setMSec(366);
  result = interp.getPose(t, &p);
  printf("\nResult of 366 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(455);
  result = interp.getPose(t, &p);
  printf("\nResult of 455 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(580);
  result = interp.getPose(t, &p);
  printf("\nResult of 580 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(750);
  result = interp.getPose(t, &p);
  printf("\nResult of 750 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(599);
  result = interp.getPose(t, &p);
  printf("\nResult of 599 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(600);
  result = interp.getPose(t, &p);
  printf("\nResult of 600 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(601);
  result = interp.getPose(t, &p);
  printf("\nResult of 601 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(50);
  result = interp.getPose(t, &p);
  printf("\nResult of 50 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(99);
  result = interp.getPose(t, &p);
  printf("\nResult of 99 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(100);
  result = interp.getPose(t, &p);
  printf("\nResult of 100 %d\n", result);
  if (result == 1)
    p.log();

  t.setMSec(101);
  result = interp.getPose(t, &p);
  printf("\nResult of 101 %d\n", result);
  if (result == 1)
    p.log();
  

}


