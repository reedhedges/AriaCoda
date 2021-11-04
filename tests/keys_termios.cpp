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


*/

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>

int main()
{
  printf("fileno(stdin) is %d\n", fileno(stdin));

  //const int flags = fcntl(fileno(stdin), F_GETFL);
  //fcntl(fileno(stdin), flags & O_NONBLOCK);


  struct termios t;
  tcgetattr(fileno(stdin), &t);

  //cfmakeraw(&t);

  t.c_cc[VTIME] = 5;
  t.c_cc[VMIN] = 0;
  t.c_lflag &= ~ECHO;
  t.c_lflag &= ~ICANON;
  //t.c_cflag &= ~CSIZE;
  //t.c_cflag |= CS8;

  tcsetattr(fileno(stdin), TCSANOW, &t);

  
  printf("type away... (ESC to quit)\n");
  while (1)
  {
    //const int key = fgetc(stdin);
    //if(key == -1) continue;

    char c;
    const ssize_t nr = read(fileno(stdin), &c, 1);
    if(nr < 1) continue;
    const int key = c;

    printf("key is %d. ", key);
    if(isgraph(key))
      printf("(%c)\n", key);
    else
      puts("");
  }

  return 0;
}
