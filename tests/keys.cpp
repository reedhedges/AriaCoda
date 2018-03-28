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
  int key;
  ArKeyHandler keyHandler;
  Aria::init();

  printf("type away... (ESC to quit)\n");
  while (1)
  {
    //keyHandler.checkKeys();
    key = keyHandler.getKey();
    if(key == -1)
    {
      ArUtil::sleep(100);
      continue;
    }
    printf("keyHandler.getKey() returned %d.\n", key);
    switch (key) {
    case ArKeyHandler::UP:
      printf("Up\n");
      break;
    case ArKeyHandler::DOWN:
      printf("Down\n");
      break;
    case ArKeyHandler::LEFT:
      printf("Left\n");
      break;
    case ArKeyHandler::RIGHT:
      printf("Right\n");
      break;
    case ArKeyHandler::ESCAPE:
      printf("Escape\n");
      printf("Exiting\n");
      keyHandler.restore();
      exit(0);
    case ArKeyHandler::F1:
      printf("F1\n");
      break;
    case ArKeyHandler::F2:
      printf("F2\n");
      break;
    case ArKeyHandler::F3:
      printf("F3\n");
      break;
    case ArKeyHandler::F4:
      printf("F4\n");
      break;
    case ArKeyHandler::F5:
      printf("F5\n");
      break;
    case ArKeyHandler::F6:
      printf("F6\n");
      break;
    case ArKeyHandler::F7:
      printf("F7\n");
      break;
    case ArKeyHandler::F8:
      printf("F8\n");
      break;
    case ArKeyHandler::F9:
      printf("F9\n");
      break;
    case ArKeyHandler::F10:
      printf("F10\n");
      break;
    case ArKeyHandler::F11:
      printf("F11\n");
      break;
    case ArKeyHandler::F12:
      printf("F12\n");
      break;
    case ArKeyHandler::HOME:
      printf("HOME\n");
      break;
    case ArKeyHandler::END:
      printf("END\n");
      break;
    case ArKeyHandler::INSERT:
      printf("INSERT\n");
      break;
    case ArKeyHandler::DEL:
      printf("DELETE\n");
      break;
    case ArKeyHandler::PAGEUP:
      printf("PAGEUP\n");
      break;
    case ArKeyHandler::PAGEDOWN:
      printf("PAGEDOWN\n");
      break;
    case ArKeyHandler::SPACE:
      printf("Space\n");
      break;
    case ArKeyHandler::TAB:
      printf("Tab\n");
      break;
    case ArKeyHandler::ENTER:
      printf("Enter\n");
      break;
    case ArKeyHandler::BACKSPACE:
      printf("Backspace\n");
      break;
    case -1:
      ArUtil::sleep(1);
      break;
    default:
      printf("'%c' %d\n", key, key);
      break;
    }
  }
}
