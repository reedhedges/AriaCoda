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

#include <inttypes.h>
#include "Aria.h"


bool tryport(const char *port, bool is422, int baud, char **argv, int argc, int maxread, int timeout);

int main(int argc, char **argv)
{


  {
    char myBuf[5];
    myBuf[0] = 0xfa;
    myBuf[1] = 0xcd;
    myBuf[2] = 4;
    myBuf[3] = 2;
    myBuf[4] = 2;

    int i;
    unsigned char n;
    int c = 0;

    i = 3;
    n = myBuf[2] - 2;
    while (n > 1) {
      c += ((unsigned char)myBuf[i] << 8) | (unsigned char)myBuf[i + 1];
      c = c & 0xffff;
      n -= 2;
      i += 2;
    }
    if (n > 0)
      c = c ^ (int)((unsigned char)myBuf[i]);
    printf("0x%x 0x%x\n", (unsigned char)(c>>8), (unsigned char)c);
    exit(0);
  }


  const int num_params_reqd = 5;
  if (argc < num_params_reqd+1)  // plus one for command name
  {
    puts("usage: serialProbe <port|\"all\"> <baud> <\"rs232\"|\"rs422\"> <maxread> <timeout> [sendbyte1 [sendbyte2]...]");
    return 1;
  }

  Aria::init();

  const char *port = argv[1];
  int baud = atoi(argv[2]);

  const char *proto = argv[3];
  bool is422;
  if (strcmp(proto, "rs232") == 0)
    is422 = false;
  else if (strcmp(proto, "rs422") == 0)
    is422 = true;
  else
  {
    puts("Error: must specify rs232 or rs422 for protocol type.");
    Aria::exit(1);
  }
  int maxread = atoi(argv[4]);
  int timeout = atoi(argv[5]);

  argv += num_params_reqd+1; // start of optional list of bytes to send
  argc -= num_params_reqd+1;

  if (strcmp(port, "all") == 0)
  {
    char portname[12];
    for (int i = 1; i <= 15; ++i)
    {
      if (i >= 10)
        snprintf(portname, 11, "\\\\.\\COM%d", i);
      else
        snprintf(portname, 11, "COM%d", i);
      printf("\n------------------ %s ------------------\n", portname);
      tryport(portname, is422, baud, argv, argc, maxread, timeout);
    }
  }
  else
  {
    tryport(port, is422, baud, argv, argc, maxread, timeout);
  }

  Aria::exit(0);
  return 0;

}

bool tryport(const char *port, bool is422, int baud, char **argv, int argc, int maxread, int timeout)
{
  ArSerialConnection ser1(is422);
  ser1.setPort(port);
  ser1.setBaud(baud);
  printf("Trying %s port %s at baud rate %d:\n", is422?"RS-422":"RS-232", port, baud);

  if (!ser1.openSimple())
  {
    ArLog::logErrorFromOS(ArLog::Terse, "Error opening %s.", port);
    return false;
  }

  if (argc > 0)
  {
    printf("-> %3d bytes: ", argc);
    fflush(stdout);
  }

  for (int i = 0; i < argc; ++i)
  {
    int d = strtol(argv[i], NULL, 0);
    if (d == 0 && errno != 0)
    {
      ArLog::logErrorFromOS(ArLog::Terse, "error parsing command argument %d (\"%s\"). Must be decimal, hexidecimal, or octal number.", i, argv[i]);
      Aria::exit(3);
    }
    unsigned char c = (unsigned char)d;
    printf("0x%.2X %c ", (unsigned char) c, (c >= ' ' && c <= '~') ? c : ' ');
    fflush(stdout);
    ser1.write((char*)&c, 1);
  }

  if (argc > 0)
    puts("");

  char buf[256];
  int n = 0;
  ArTime t;
  while (1)
  {
    ArUtil::sleep(1);
    int r = 0;
    if ((r = ser1.read(buf, sizeof(buf))) < 0)
    {
      puts("Error reading data from serial port.");
      return false;
    }
    else if (r > 0)
    {
      printf("<- %3d bytes: ", r);
      for (int i = 0; i < r; ++i)
      {
        //printf("[%d] ", i); fflush(stdout);
        char c = buf[i];
        printf("0x%.2X %c ", (unsigned char)c, (c >= ' ' && c <= '~') ? c : ' ');
        if ((i+1) % 8 == 0)
          printf("\n              ");
      }
      printf("\n");
    }
    if ((n += r) >= maxread)
    {
      puts("max");
      return true;
    }
    if (t.secSince() > timeout)
    {
      puts("timeout");
      return false;
    }
  }
  return true;
}


