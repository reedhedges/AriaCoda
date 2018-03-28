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

/** @example mtxDigitalIOExample.cpp  Sets patterns on digital outputs and acts on digital
 * inputs.   Attach LEDs to the outputs and buttons to the digital inputs to
 * use. (See the Pioneer LX manual and robots.mobilerobots.com for more
 * details.) 
 * The 'mtx' Linux kernel module must be loaded, and the /dev/mtx character
 * device must have been created (see mtxIODriver documentation).  You must
 * have read/write access to /dev/mtx.
 */

void printBits(unsigned char byte) 
{
  int bit; 
  int ch;
  for(bit = 7, ch = 0; bit >= 0; bit--,ch++)
  {
    if(ch == 4)
       printf(" ");
    if ((byte>>bit)&1)
       printf("1");
    else
       printf("0");
  }
  //printf(" (0x%04x) ", byte);
}


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

  unsigned char out = 1;
  while(true)
  {
    mtxIO.lock();

    // print state of inputs
    unsigned char inp;
    mtxIO.getDigitalIOInputMon1(&inp);
    printf("Input 1: ");
    printBits(inp);
    printf(" | ");
    mtxIO.getDigitalIOInputMon2(&inp);
    printf("Input 2: ");
    printBits(inp);
    printf(" | ");

    

/* shouldn't be neccesary
    unsigned char cur;

    // get current state
    if(!mtxIO.getDigitalOutputControl1(&cur))
    {
      ArLog::log(ArLog::Terse, "mtxIO: Error getting current state of output control 1");
      mtxIO.unlock();
      Aria::exit(2);
    }
*/

    // set new state on both output banks

    printf("Output 1: ");
    printBits(out);
    printf(" | ");
    if(!mtxIO.setDigitalOutputControl1(&out))
    {
      ArLog::log(ArLog::Terse, "mtxIO: Error setting state of output control 1");
      mtxIO.unlock();
      Aria::exit(3);
    }

    printf("Output 2: ");
    printBits(out);
    printf(" | ");
    if(!mtxIO.setDigitalOutputControl2(&out))
    {
      ArLog::log(ArLog::Terse, "mtxIO: Error setting state of output control 2");
      mtxIO.unlock();
      Aria::exit(3);
    }
  
    // wait
    mtxIO.unlock();
    ArUtil::sleep(500);
 
    // shift 
    
    out = out << 1;
    if(out == 0) out = 1;

    puts("");
  }
    
    
    
  Aria::exit(0);

}
