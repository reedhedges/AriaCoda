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

ArPose fn1(void)
{
  static ArPose pose;
  pose.setX(pose.getX() + 1);
  pose.setY(pose.getY() + 1);
  pose.setTh(pose.getTh() - 1);
  return pose;
}

void fn2(ArPose pose)
{
  pose.log();
}

int main(void)
{
  printf("Entering 100 iterations of incrementing pose X and Y and decrementing Theta...");
  for (int i = 0; i < 100; i++)
    fn2(fn1());

  printf("\nTesting ArPose::operator+(const ArPose&) and ArPose::operator-(const ArPose&)...\n");
  ArPose p1(10, 10, 90);
  ArPose p2(10, 10, 45);
  ArPose p3(0, 0, 0);
  ArPose p4(-20, 0, 360);
  ArPose p5(-20, -20, -180);
  printf("(10,10,90) + (10,10,90) => ");
  (p1 + p1).log();
  printf("(10,10,90) - (10,10,90) => ");
  (p1 - p1).log();
  printf("(10,10,90) + (10,10,45) => ");
  (p1 + p2).log();
  printf("(10,10,90) + (0,0,0) => ");
  (p1 + p3).log();
  printf("(10,10,90) - (0,0,0) => ");
  (p1 - p3).log();
  printf("(0,0,0) + (-20,0,360) => ");
  (p3 + p4).log();
  printf("(0,0,0) - (-20,0,360) => ");
  (p3 - p4).log();
  printf("(10,10,90) + (-20,0,360) => ");
  (p1 + p4).log();
  printf("(-20,0,360) + (-20,0,360) => ");
  (p4 + p4).log();
  printf("(-20,-20,-180) - (10,10,45) => ");
  (p5 - p2).log();

}
