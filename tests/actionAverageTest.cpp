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

ArPriorityResolver Resolver; 
ArResolver::ActionMap Actions;

void addAction(ArAction *action, int priority)
{
  Actions.insert(std::pair<int, ArAction *>(priority, action));
}

int main(void)
{
  ArActionDesired *des;

  ArActionConstantVelocity cv("", 400);
  ArActionConstantVelocity cv2("", 200);
  ArActionConstantVelocity cv3("", 500);

  addAction(&cv, 50);
  addAction(&cv2, 50);
  
  des = Resolver.resolve(&Actions, NULL);
  printf("des %.2f, should be 300\n", des->getVel());

  addAction(&cv3, 50);
  des = Resolver.resolve(&Actions, NULL);
  printf("des %.2f, should be 366.67\n", des->getVel());
}
