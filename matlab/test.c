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

#include "ariac.h"
#include <stdio.h>
#include <unistd.h> // for sleep()

int main(int argc, char **argv)
{
  aria_init_with_args(argc, argv);
  while(1)
  {
    if(!arrobot_connect())
      return 1 ;
    printf("radius=%.2f, length=%.2fmm, width=%.2fmm\n", 
      arrobot_radius(),
      arrobot_length(),
      arrobot_width()
    );
    arrobot_setvel(200);
    arrobot_setrotvel(10);
    for(int c = 0; c < 10; ++c)
    {
      arrobot_connect(); // should skip connection if already connected.
      printf("x=%.1f y=%.1f th=%.1f vel=%.1f rotvel=%.1f left=%.1f right=%.1f stall=%d battv=%.1f\n",
        arrobot_getx(),
        arrobot_gety(),
        arrobot_getth(),
        arrobot_getvel(),
        arrobot_getrotvel(),
        arrobot_getleftvel(),
        arrobot_getrightvel(),
        arrobot_isstalled(),
	arrobot_getbatteryvoltage()
      );
      printf("sonar");
      for(int i = 0; i < arrobot_getnumsonar(); ++i)
      {
        printf(" %d=%f", i, arrobot_getsonarrange(i));
      }
      puts("");
      sleep(1);
    }
    arrobot_disconnect();
    puts("disconnected, reconnecting in 5 sec...");
    sleep(5);
  }
  return 0;
}
