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

#ifdef _MSC_VER
#include <windows.h>
#define usleep(ms) Sleep(ms)
#else
#include <unistd.h>
#endif


int main(int argc, char **argv)
{
	aria_init_with_args(argc, argv);
	if(!arrobot_connect())
	{
		puts("Could not connect to robot. Exiting.");
		usleep(2000);
		aria_exit(1);
	}

	usleep(1000);
	printf("Width=%.1f Length=%.1f Radius=%.1f NumFrontBumps=%d NumRearBumps=%d\n",
		arrobot_width(),
		arrobot_length(),
		arrobot_radius(),
		arrobot_num_front_bumpers(),
		arrobot_num_rear_bumpers()
	);

	while(1)
	{
		int i;
		printf("X=%.1f Y=%.1f Th=%.1f Vel=%.1f RotVel=%.1f Bat=%.1f NumSon=%d LeftStall=%d RightStall=%d\n",
			arrobot_getx(),
			arrobot_gety(),
			arrobot_getth(),
			arrobot_getvel(),
			arrobot_getrotvel(),
			arrobot_getbatteryvoltage(),
			arrobot_getnumsonar(),
			arrobot_isleftstalled(),
			arrobot_isrightstalled()
		);
		for(i = 0; i < arrobot_num_front_bumpers(); ++i)
			if(arrobot_get_front_bumper(i))
				printf("Front bumper %d pressed.\n", i);
		for(i = 0; i < arrobot_num_rear_bumpers(); ++i)
			if(arrobot_get_rear_bumper(i))
				printf("Rear bumper %d pressed.\n", i);

		usleep(2000);
	}

	aria_exit(0);
}
