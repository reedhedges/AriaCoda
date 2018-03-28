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
#include "ariaUtil.h"

void testIntersection(ArLine *line1, ArLine *line2, double x, double y,
		      char *name)
{
  ArPose pose;

  if (!line1->intersects(line2, &pose) || fabs(pose.getX() - x) > .001 ||
      fabs(pose.getY() - y) > .001)
  {
    printf("%s didn't intersect\n", name);
    exit(1);
  }

  if (line1->intersects(line1, &pose))
  {
    printf("First line of %s intersected itself\n", name);
    exit(1);
  }

  if (line2->intersects(line2, &pose))
  {
    printf("Second line of %s intersected itself\n", name);
    exit(1);
  }
}

void testIntersection(ArLineSegment *line1, ArLine *line2, double x, double y,
		      char *name)
{
  ArPose pose;

  if (!line1->intersects(line2, &pose) || fabs(pose.getX() - x) > .001 ||
      fabs(pose.getY() - y) > .001)
  {
    printf("%s didn't intersect\n", name);
    exit(1);
  }

  if (line1->intersects(line1, &pose))
  {
    printf("First line of %s intersected itself\n", name);
    exit(1);
  }

  if (line2->intersects(line2, &pose))
  {
    printf("Second line of %s intersected itself\n", name);
    exit(1);
  }
}

void testIntersection(ArLineSegment *line1, ArLineSegment *line2, 
		      double x, double y, char *name)
{
  ArPose pose;

  if (!line1->intersects(line2, &pose) || fabs(pose.getX() - x) > .001 ||
      fabs(pose.getY() - y) > .001)
  {
    printf("%s didn't intersect\n", name);
    exit(1);
  }

  if (line1->intersects(line1, &pose))
  {
    printf("First line of %s intersected itself\n", name);
    exit(1);
  }

  if (line2->intersects(line2, &pose))
  {
    printf("Second line of %s intersected itself\n", name);
    exit(1);
  }
}

void testPerp(ArLineSegment *segment, ArPose perp, ArPose perpPoint, 
	      char *name)
{
  ArPose pose;
  if (!segment->getPerpPoint(perp, &pose) || 
      fabs(pose.getX() - perpPoint.getX()) > .001 ||
      fabs(pose.getY() - perpPoint.getY()) > .001)
  {
    printf("%s wasn't perp but should have been\n", name);
    exit(1);
  }
}

void testNotPerp(ArLineSegment *segment, ArPose perp, char *name)
{
  ArPose pose;
  if (segment->getPerpPoint(perp, &pose))
  {
    printf("%s was perp but shouldn't have been, at %.0f %.0f\n", name,
	   pose.getX(), pose.getY());
    exit(1);
  }
}

int main(void)
{
  ArPose pose;

  ArLine xLine(-2000, 0, 2000, 0);
  ArLine yLine(100, 500, 100, -500);
  ArLineSegment xLineSeg(-2000, 0, 2000, 0);
  ArLineSegment yLineSeg(100, 500, 100, -500);

  // test all our segments
  testIntersection(&xLine, &yLine, 100, 0, "xLine and yLine");
  testIntersection(&xLineSeg, &yLine, 100, 0, "xLineSeg and yLine");
  testIntersection(&yLineSeg, &xLine, 100, 0, "yLineSeg and xLine");
  testIntersection(&xLineSeg, &yLineSeg, 100, 0, "xLineSeg and yLineSeg");
  

  // test the perp on all the segments
  testPerp(&xLineSeg, ArPose(-2000, 50), ArPose(-2000, 0), "xLineSeg end1");
  testPerp(&xLineSeg, ArPose(2000, 50), ArPose(2000, 0), "xLineSeg end2");
  testPerp(&xLineSeg, ArPose(357, 50), ArPose(357, 0), "xLineSeg middle");
  testNotPerp(&xLineSeg, ArPose(2001, 0), "xLineSeg beyond end2");
  testNotPerp(&xLineSeg, ArPose(3000, 0), "xLineSeg way beyond end2");
  testNotPerp(&xLineSeg, ArPose(-2001, 0), "xLineSeg beyond end1");
  testNotPerp(&xLineSeg, ArPose(-3000, 0), "xLineSeg way beyond end1");
  
  testPerp(&xLineSeg, ArPose(1000, 0), ArPose(1000, 0), "xLineSeg point on line");

  printf("All tests completed successfully\n");

  return 0;
}
