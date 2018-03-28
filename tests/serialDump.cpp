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

int main(int argc, char **argv)
{
  int ret;
  char bufWrite[1024];
  char bufRead[1024];
  bool verbose = true;
  int i, n;

  const char *port = ArUtil::COM1;
  if (argc > 0)
	  port = argv[1];

  /*
  for (i = 0; i < 1024; i++)
    bufWrite[i] = 0x66;
  */

  
  int bytes1 = 0;
  int bytes2 = 0;
  //int numToWrite = 1;

  ArTime lastPrint;

  ArSerialConnection ser1;
  ser1.setPort(port);
  //ser1.setBaud(115200);
  printf("opening %s...\n", port);
  if (!ser1.openSimple())
  {
    printf("open failed\n");
    exit(0);
  }
  printf("Port opened\n");
  lastPrint.setToNow();
  while (1)
  {
    ArUtil::sleep(1);
    //ArUtil::sleep(500);
/*
    bufWrite[0] = 0xfa;
    bufWrite[1] = 0xfb;
    bufWrite[2] = 0x3;
    bufWrite[3] = 0x0;
    bufWrite[4] = 0x0;
    bufWrite[5] = 0x0;
    ser1.write(bufWrite, 6);
*/
    ////ser1.write("a", 1);
    if ((ret = ser1.read(bufRead, sizeof(bufRead))) < 0)
      printf("Failed read\n");
    else if (ret > 0)
    {
      bufRead[ret] = '\0';
      if (verbose)
      {
	printf("%3d: ", ret);
	for (i = 0; i < ret; i++)
	  printf("%c(%x) ", bufRead[i], (unsigned char)bufRead[i]);
	printf("\n");
      }
      else
	printf("%s", bufRead);
    }
    else
      bufRead[0] = '\0';
    //ser1.write("a", 1);

  }
  
}
