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

/* See also angleBetweenTest, angleFixTest, mathTests. */

bool findDifference(ArPose origin, ArPose sonar, double th, 
		    double dist)
{
  double cth, cdist;
  cth = ArMath::subAngle(origin.findAngleTo(sonar), origin.getTh());
  cdist = origin.findDistanceTo(sonar);
  if (ArMath::fabs(th - cth) < .1 && 
      ArMath::fabs(cdist - dist) < .1)
  {
    //printf("GOOD got %7.2f %6.0f\n", cth, cdist);
    return true;
  }
  else
  {
    printf("bad  wanted %.2f %.0f got ", th, dist);
    printf("%.2f %.0f origin %.0f %.0f %.0f sonar %.0f %.0f %.0f angleTo %.0f\n",
	   cth, cdist, origin.getX(), origin.getY(), 
	   origin.getTh(), sonar.getX(), sonar.getY(), sonar.getTh(),
	   origin.findAngleTo(sonar));
    /*
	   (sonar.getY() - origin.getY()), (sonar.getX() - origin.getX()),
	   ArMath::radToDeg(atan2(sonar.getY() - origin.getY(), sonar.getX() - origin.getX())));
	   */
	    
    printf("Failed tests!\n");
    return false;
  }
  
}

void testSet(double x1, double y1, double x2, double y2, double initial, 
	   double distance)
{
  double add;

  ArPose origin;
  ArPose sonar(x2, y2);

  for (add = -135; add <= 180; add += 45)
  {
    origin.setPose(x1, y1, add);
    if (!findDifference(origin, sonar, ArMath::subAngle(initial, add),
			distance))
    {
      printf("Failed that one from (%.0f, %.0f) (%.0f, %.0f) %3.0f %4.0f\n", 
	     x1, y1, x2, y2, initial, distance);
      exit(0);
    }
  }
  
}

int main(void)
{
  ArPose origin;
  ArPose sonar;
  double halfDiag = sqrt(500 * 500 + 500 * 500);
  double diag = sqrt(1000 * 1000 + 1000 * 1000);

  origin.setPose(0, 0, 0);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 90, 1000);

  origin.setPose(0, 0, 45);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 45, 1000);

  origin.setPose(0, 0, 90);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 0, 1000);

  origin.setPose(0, 0, 135);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, -45, 1000);

  origin.setPose(0, 0, 180);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, -90, 1000);

  origin.setPose(0, 0, -135);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, -135, 1000);

  origin.setPose(0, 0, -90);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 180, 1000);

  origin.setPose(0, 0, -45);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 135, 1000);
  
  //printf("\n");

  origin.setPose(500, 500, 0);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 135, halfDiag);

  origin.setPose(500, 500, 45);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 90, halfDiag);

  origin.setPose(500, 500, 90);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 45, halfDiag);

  origin.setPose(500, 500, 135);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 0, halfDiag);

  origin.setPose(500, 500, 180);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, -45, halfDiag);
  
  origin.setPose(500, 500, -135);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, -90, halfDiag);

  origin.setPose(500, 500, -90);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, -135, halfDiag);

  origin.setPose(500, 500, -45);
  sonar.setPose(0, 1000, 0);
  findDifference(origin, sonar, 180, halfDiag);


  testSet(0, 0, 0, 0, 0, 0);
  testSet(0, 0, 0, 1000, 90, 1000);
  testSet(0, 0, 1000, 1000, 45, diag);
  testSet(0, 0, 1000, 0, 0, 1000);
  testSet(0, 0, 500, 500, 45, halfDiag);

  testSet(0, 1000, 0, 0, -90, 1000);
  testSet(0, 1000, 0, 1000, 0, 0);
  testSet(0, 1000, 1000, 1000, 0, 1000);
  testSet(0, 1000, 1000, 0, -45, diag);
  testSet(0, 1000, 500, 500, -45, halfDiag);

  testSet(1000, 1000, 0, 0, -135, diag);
  testSet(1000, 1000, 0, 1000, -180, 1000);
  testSet(1000, 1000, 1000, 1000, 0, 0);
  testSet(1000, 1000, 1000, 0, -90, 1000);
  testSet(1000, 1000, 500, 500, -135, halfDiag);

  testSet(1000, 0, 0, 0, 180, 1000);
  testSet(1000, 0, 0, 1000, 135, diag);
  testSet(1000, 0, 1000, 1000, 90, 1000);
  testSet(1000, 0, 1000, 0, 0, 0);
  testSet(1000, 0, 500, 500, 135, halfDiag);

  testSet(500, 500, 0, 0, -135, halfDiag);
  testSet(500, 500, 0, 1000, 135, halfDiag);
  testSet(500, 500, 1000, 1000, 45, halfDiag);
  testSet(500, 500, 1000, 0, -45, halfDiag);
  testSet(500, 500, 500, 500, 0, 0);

  printf("Passed all tests!\n");
  return 0;
}
