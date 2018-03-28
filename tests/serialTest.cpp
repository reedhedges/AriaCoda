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

int  test(const char *port1, const char *port2);

int main(int argc, char **argv)
{
  Aria::init();

  srand(time(NULL));



 
  if (argc < 3 || (argc >= 2 && strcmp(argv[1], "-h") == 0))
  {
    puts("usage: serialTest <port1> <port2>\nConnect port1 and port2 with crossover (null modem) serial cable. Data is transferred between in varying size chunks and verified.");
    return 1;
  }

 /// const char *port1 = argv[1];
  //const char *port2 = argv[2];
  //test(port1, port1);

  std::list< std::pair<int, int> > matches;

  char port1[12];
  char port2[12];
  for (int i = 3; i <= 14; ++i)
  {
    if (i >= 10)
      snprintf(port1, 11, "\\\\.\\COM%d", i);
    else
      snprintf(port1, 11, "COM%d", i);
    for (int j = i + 1; j <= 14; ++j)
    {
      if (j >= 10)
        snprintf(port2, 11, "\\\\.\\COM%d", j);
      else
        snprintf(port2, 11, "COM%d", j);
      printf("\n------------------ %s and %s  ------------------\n", port1, port2);
      int r = test(port1, port2);
      if (r == 0)
      {
        printf("\n%s and %s communication passed.\n", port1, port2);
        matches.push_back(std::pair<int, int>(i, j));
      }
    }
  }

  for (auto i = matches.begin(); i != matches.end(); ++i)
    printf("matched: COM%d, COM%d\n", i->first, i->second);
  Aria::exit(0);
}

 int test(const char *port1, const char *port2)
 {
   int ret;
   char bufWrite[1024];
   char bufRead[1024];
   int i, n;
   for (i = 0; i < 1024; i++)
   {
     bufWrite[i] = 0x66;
     bufRead[i] = 0;
   }

   int bytes1 = 0;
   int bytes2 = 0;
   //int numToWrite = 1;

   ArTime lastPrint;

  printf("Testing %s and %s...\n", port1, port2);
  ArSerialConnection ser1;
  ArSerialConnection ser2;
  ser1.setPort(port1);
  ser1.setBaud(9600);
  ser2.setPort(port2);
  ser2.setBaud(9600);
  if (!ser1.openSimple())
  {
    printf("Error, could not open %s\n", port1);
    return 1;
  }
  if (!ser2.openSimple())
  {
    printf("Error, could not open %s\n", port2);
    return 2;
  }
  printf("Ports %s and %s opened\n", port1, port2);
  lastPrint.setToNow();
  for (int loop = 0; loop < 10; ++loop)
  {
    // write a random amount of bytes to port 1
    n = rand() % 1024;
    printf("-> write %d bytes -> %s...\n", n, port1);
    if (ser1.write(bufWrite, n) < 0)
    {
      printf("Failed write to port 1\n");
      return 3;
    }
    // read a random amount of bytes from port 2
    //n = rand() % n;
    printf("<- read %d bytes <- %s...", n, port2);
    if ((ret = ser2.read(bufRead, n)) < 0)
    {
      printf("Failed read from port 2\n");
      return 4;
    }
    printf(" (%d bytes actually read)\n", ret);
    if (ret > 0)
    {
      // all bytes read into bufRead should be 0x66 since bufWrite was initialized as such above.
      for (i = 0; i < ret; i++)
      {
        if (bufRead[i] != 0x66)
        {
          printf("\n!! Failed (data does not match) !!\n\n");
          return 5;
        }
      }
      printf("\n ** Passed (data matches) ** \n\n");
      bytes2 += ret;
    }

    for (i = 0; i < 1024; i++)
      bufRead[i] = 0;

    n = rand() % 1024;
    printf("-> write %d bytes -> %s...\n", n, port1);
    if (ser2.write(bufWrite, n) < 0)
    {
      printf("Failed2 write\n");
      return 6;
    }
    //n = rand() % 1024;
    printf("<- read %d bytes <- %s...", n, port2);
    if ((ret = ser1.read(bufRead, n)) < 0)
    {
      printf("Failed2 read\n");
      return 7;
    }
    printf(" (%d bytes actually read)\n", ret);
    if (ret > 0)
    {
      for (i = 0; i < ret; i++)
      {
        if (bufRead[i] != 0x66)
        {
          printf("\n!! Failed2 0x%x %d/%d (data does not match) !!\n\n", bufRead[i], i, n);
          return 8;
        }
      }
      printf("\n ** Passed (data matches) ** \n\n");
      bytes1 += ret;
    }

    if (lastPrint.mSecSince() > 1000)
    {
      printf("\ntotal %d bytes %s->%s; %d bytes %s->%s\n\n", bytes1, port1, port2, bytes2, port2, port1);
      lastPrint.setToNow();
    }
  }
 if(bytes1 > 0 && bytes2 > 0) return 0;
 else return 9;
}

