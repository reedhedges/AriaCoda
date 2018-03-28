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


// Required setup:
//  Use regedit to create the key:
//    HKEY_CURRENT_USER\Software\MobileRobots\Aria
//  with value named test with any string value.
//  If testing Wow64 redirection (64-bit system with 32-bit build), also create:
//    HKEY_CURRENT_USER\Software\Wow6432Node\MobileRobots\Aria
//  with value named test with any string value

int main(int argc, char **argv) 
{
  puts("Aria init...");
  fflush(stdout);
  Aria::init(Aria::SIGHANDLE_THREAD, false);

  char buf[512];
  memset(buf, 0, 512);
  const char* keypath = "SOFTWARE\\MobileRobots\\Aria";
  const char* valname = "test";
 
  bool r = ArUtil::getStringFromRegistry(ArUtil::REGKEY_CURRENT_USER, keypath, valname, buf, 512);
  printf("\ngetStringFromRegistry(REGKEY_CURRENT_USER, \"%s\", \"%s\"...) returned %s. buf=\"%s\"\n", keypath, valname, r?"true":"false", buf);
  
  r = ArUtil::getStringFromRegistry(ArUtil::REGKEY_LOCAL_MACHINE, keypath, valname, buf, 512);
  printf("\ngetStringFromRegistry(REGKEY_LOCAL_MACHINE, \"%s\", \"%s\"...) returned %s. buf=\"%s\"\n", keypath, valname, r ? "true" : "false", buf);

  r = ArUtil::findFirstStringInRegistry(keypath, valname, buf, 512);
  printf("\nfindFirstStringInRegistry(\"%s\", \"%s\"...) returned %s. buf=\"%s\"\n", keypath, valname, r ? "true" : "false", buf);

  keypath = "SOFTWARE\\MobileRobots\\Arnl";
  valname = "Install Directory";

  r = ArUtil::getStringFromRegistry(ArUtil::REGKEY_CURRENT_USER, keypath, valname, buf, 512);
  printf("\ngetStringFromRegistry(REGKEY_CURRENT_USER, \"%s\", \"%s\"...) returned %s. buf=\"%s\"\n", keypath, valname, r ? "true" : "false", buf);

  r = ArUtil::getStringFromRegistry(ArUtil::REGKEY_LOCAL_MACHINE, keypath, valname, buf, 512);
  printf("\ngetStringFromRegistry(REGKEY_LOCAL_MACHINE, \"%s\", \"%s\"...) returned %s. buf=\"%s\"\n", keypath, valname, r ? "true" : "false", buf);

  r = ArUtil::findFirstStringInRegistry(keypath, valname, buf, 512);
  printf("\nfindFirstStringInRegistry(\"%s\", \"%s\"...) returned %s. buf=\"%s\"\n", keypath, valname, r ? "true" : "false", buf);


  puts("exit");
  Aria::exit(0);

  return(0);
}

