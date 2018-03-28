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
  This program just outputs the values from the joystick
*/
int main(void)
{
  Aria::init();
  ArLog::init(ArLog::StdOut, ArLog::Verbose);
  ArJoyHandler joy;
  joy.init();
  unsigned int i;
  double x, y, z;
  int xi, yi, zi;
  if (!joy.haveJoystick())
  {
    printf("Did not definitely detect a joystick, it may not work.\n");
  }
  printf("Num. Buttons: %d\n", joy.getNumButtons());
  printf("Num. Axes: %d\n", joy.getNumAxes());
  if (joy.haveZAxis())
    printf("The Z axis (Axis 3, the throttle) works\n");
  else
    printf("The Z axis (Axis 3, the throttle) may or may not work (it'll show up below if it does)\n");
	 
  puts("Press button 1 to set speeds to 100,100,100, button 2 for 1000,100,50");

  while (Aria::getRunning())
  {
    printf("\rButton ");
    for (i = 1; i <= joy.getNumButtons(); i++)
      printf(" %d:%d", i, joy.getButton(i));

    joy.getDoubles(&x, &y, &z);
    joy.getAdjusted(&xi, &yi, &zi);
    printf(" Axis x:%6.3f y:%6.3f (adj x:%d y:%d)", x, y, xi, yi);
    if (joy.haveZAxis())
      printf(" z:%6.3f (adj %d)", z, zi);
    for (i = 4; i < joy.getNumAxes(); i++)
      printf(" %d:%6.3f", i, joy.getAxis(i));

    if(joy.getNumButtons() >= 1 && joy.getButton(1))
    {
      printf(" ->setSpeeds(100,100,100);");
      joy.setSpeeds(100,100,100);
    }
    else if (joy.getNumButtons() >= 2 && joy.getButton(2))
    {
      printf(" ->setSpeeds(1000,100,50);");
      joy.setSpeeds(1000,100,50);
    }
    else
      printf("                                 ");
    fflush(stdout);
    ArUtil::sleep(1);
  }

}
