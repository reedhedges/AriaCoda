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

ArACTS_1_2 acts;
enum {
  WIDTH = 160,
  HEIGHT = 120
};
  

int main(void)
{
  ArACTSBlob blob;
  double xRel, yRel;
  int i;

  acts.openPort(NULL);
  acts.requestPacket();
  while(1)
  {
    if (acts.receiveBlobInfo())
      {
	//acts.receiveBlobInfo();
	for (i = 0; i < 8; i++)
	  if (acts.getNumBlobs(i) >= 1 && acts.getBlob(i, 1, &blob))
	    {
	      xRel = (double)(blob.getXCG() - WIDTH/2.0) / (double)WIDTH;
	      yRel = (double)(blob.getYCG() - HEIGHT/2.0) / (double)HEIGHT;
	      printf("Chan %d xRel %.4f yRel %.4f    ", i, xRel, yRel);
	      blob.log();
	    }
	acts.requestPacket();
	ArUtil::sleep(80);
      }
    ArUtil::sleep(5);
  }

}
