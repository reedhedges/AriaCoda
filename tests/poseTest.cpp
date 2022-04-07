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


*/
#include "Aria/Aria.h"
#include <cassert>

ArPose fn1()
{
  static ArPose pose;
  pose.setX(pose.getX() + 1);
  pose.setY(pose.getY() + 1);
  pose.setTh(pose.getTh() - 1);
  return pose;
}

ArPose fn2(ArPose pose)
{
  pose.log();
  return pose;
}

int main()
{
  printf("Entering 100 iterations of incrementing pose X and Y and decrementing Theta...");
  ArPose p0;
  for (int i = 0; i < 100; i++)
    p0 = fn2(fn1());
  assert(p0.getX() == 100);
  assert(p0.getY() == 100);
  assert(p0.getTh() == -100);

  printf("\nTesting ArPose::operator+(const ArPose&) and ArPose::operator-(const ArPose&)...\n");
  ArPose p1(10, 10, 90);
  ArPose p2(10, 10, 45);
  ArPose p3(0, 0, 0);
  ArPose p4(-20, 0, 360);
  ArPose p5(-20, -20, -180);

  printf("(10,10,90) + (10,10,90) => ");
  ArPose s1 = p1 + p1;
  s1.log();
  assert(s1.getX() == 20);
  assert(s1.getY() == 20);
  assert(s1.getTh() == 180);

  printf("(10,10,90) - (10,10,90) => ");
  ArPose d1 = p1 - p1;
  d1.log();
  assert(d1.getX() == 0);
  assert(d1.getY() == 0);
  assert(d1.getTh() == 0);

  printf("(10,10,90) + (10,10,45) => ");
  ArPose s2 = p1 + p2;
  s2.log();
  assert(s2.getX() == 20);
  assert(s2.getY() == 20);
  assert(s2.getTh() == 135);

  printf("(10,10,90) + (0,0,0) => ");
  ArPose s3 = p1 + p3;
  s3.log();
  assert(s3.getX() == 10);
  assert(s3.getY() == 10);
  assert(s3.getTh() == 90);

  printf("(10,10,90) - (0,0,0) => ");
  ArPose d2 = p1 - p3;
  d2.log();
  assert(d2.getX() == 10);
  assert(d2.getY() == 10);
  assert(d2.getTh() == 90);

  printf("(0,0,0) + (-20,0,360) => ");
  ArPose s4 = p3 + p4;
  s4.log();
  assert(s4.getX() == -20);
  assert(s4.getY() == 0);
  assert(s4.getTh() == 0);

  printf("(0,0,0) - (-20,0,360) => ");
  ArPose d3 = p3 - p4;
  d3.log();
  assert(d3.getX() == 20);
  assert(d3.getY() == 0);
  assert(d3.getTh() == 0);

  printf("(10,10,90) + (-20,0,360) => ");
  ArPose s5 = p1 + p4;
  s5.log();
  assert(s5.getX() == -10);
  assert(s5.getY() == 10);
  assert(s5.getTh() == 90);

  printf("(-20,0,360) + (-20,0,360) => ");
  ArPose s6 = p4 + p4;
  s6.log();
  assert(s6.getX() == -40);
  assert(s6.getY() == 0);
  assert(s6.getTh() == 0);

  printf("(-20,-20,-180) - (10,10,45) => ");
  ArPose d6 = p5 - p2;
  d6.log();
  assert(d6.getX() == -30);
  assert(d6.getY() == -30);
  assert(d6.getTh() == 135);

  return 0;
}
