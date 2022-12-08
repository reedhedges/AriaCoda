/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016-2018 Omron Adept Technologies, Inc.

     This program is free sfor t=tware; you can redistribute it and/or modify
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

void printInterpResult(ArTime t, int r, ArPose p)
{
  printf("Result for t=%3lums: (%d) ", t.getMSec(), r);
  if(r == 0)
  {
    printf("Projected pose forward in time => {%.2f %.2f %.8f}", p.getX(), p.getY(), p.getTh());
  }
  else if(r == 1)
  {
    printf("Interpolated pose back => {%.2f %.2f %.8f}", p.getX(), p.getY(), p.getTh());
  }
  else if(r == -1)
  {
    printf("Cannot predict forward");
  }
  else if(r == -2)
  {
    printf("Cannot interpolate back, too old");
  }
  else if(r == -3)
  {
    printf("Cannot interpolate, not enough pose history yet");
  }
  else
  {
    printf("Invalid ArInterpolation::getPose() return value!");
    assert(r >= -3 && r <= 1);
  }
}

void check(ArInterpolation& interp, unsigned long t, int expectedResultFlag, ArPose expectedPose = ArPose())
{
  ArPose p;
  ArTime tm;
  tm.setMSec(t);
  int result = interp.getPose(tm, &p);
  printInterpResult(tm, result, p);
  printf("...");
  if(result != expectedResultFlag)
    printf("\t! Result flag = %d, expected %d\n", result, expectedResultFlag);
  assert(result == expectedResultFlag);
  
  if(expectedResultFlag == 0 || expectedResultFlag == 1)
  {
    if(p != expectedPose)
      printf("\t! expectedPose is {%.2f %.2f %.8f} [epsilon=%.10f]\n", expectedPose.getX(), expectedPose.getY(), expectedPose.getTh(), ArMath::epsilon());
    assert(p == expectedPose);
  }

  puts("ok");
}

int main()
{
  ArInterpolation interp;

  for (unsigned long i = 1; i <= 6; i++)
  {
    ArTime t;
    ArPose p;
    t.setMSec(i*100);
    p.setPose((double)i*100.0, (double)i*100.0, (double)(i - 1)*60.0);
    printf("Added pose {%.0f %.0f %.0f} with time %lu\n", p.getX(), p.getY(), p.getTh(), t.getMSec());
    interp.addReading(t, p);
  }


  // Check interpolated poses exactly on the timestamps used above:

  check(interp, 100, 1, ArPose(100, 100, 0));
  check(interp, 200, 1, ArPose(200, 200, 60));
  check(interp, 400, 1, ArPose(400, 400, 180));
  check(interp, 600, 1, ArPose(600, 600, -60));
  

  // Check interpolated poses for various times in between data:

  check(interp, 101, 1, ArPose(101, 101, 0.6));
  check(interp, 250, 1, ArPose(250, 250, 90.0));
  check(interp, 366, 1, ArPose(366, 366, 159.6));
  check(interp, 455, 1, ArPose(455, 455, -147.0));
  check(interp, 580, 1, ArPose(580, 580, -72.0));
  check(interp, 750, 0, ArPose(600, 600, -60.0)); // is it projecting it forward enough?
  check(interp, 599, 1, ArPose(599, 599, -60.6));
  check(interp, 600, 1, ArPose(600, 600, -60.0));
  check(interp, 601, 0, ArPose(600, 600, -60.0));
  check(interp, 50, -2);
  check(interp, 99, -2);
  // too far forward doesn't seem to work. check(interp, 20000, -1);
  

  return 0;
}


