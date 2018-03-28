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
#include "ArMTXIO.h"
#include <stdint.h>

/** @example mtxAnalogIOExample.cpp  Reads and writes analog IO values.
 * (See the Pioneer LX manual and robots.mobilerobots.com for more
 * details.) 
 * The 'mtx' Linux kernel module must be loaded, and the /dev/mtx character
 * device must have been created (see mtxIODriver documentation).  You must
 * have read/write access to /dev/mtx.
 */


int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();

  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(1);
  }

  ArMTXIO mtxIO;

  if(!mtxIO.isEnabled())
  {
    ArLog::log(ArLog::Terse, "mtxIO: Error opening MTX IO device interface!");
    Aria::exit(4);
  }

  printf("Input:");
  for(unsigned int i = 0; i < 64 - strlen("Input:"); ++i)
    printf(" ");
  puts("Output:");
  for(unsigned int i = 0; i < 20; ++i)
    printf("%4d ", i);
  puts("");

  while(true)
  {
    mtxIO.lock();

    // pr inputs
    for(unsigned int i = 0; i < 16; ++i)
    {
      unsigned short val;
      mtxIO.getAnalogIOBlock1(i, &val);
      printf("%2.1f ", val/1000.0);
    }

    // print outputs
    for(unsigned int i = 0; i < 4; ++i)
    {
      unsigned short val;
      mtxIO.getAnalogIOBlock2(i, &val);
      printf("%2.1f ", val/1000.0);
    }

    puts("");

    ArUtil::sleep(500);
  }
    
    
    
  Aria::exit(0);

}
