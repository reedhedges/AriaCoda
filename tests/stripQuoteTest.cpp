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

#include <cassert>
#include <string>
#include "Aria/Aria.h"

void test(const char *str, const char *expected)
{
  printf("Stripping quotes from input string: %s\n", str);
  char buffer[512];
  bool s = ArUtil::stripQuotes(buffer, str, 512);
  assert(s);
  printf("\t...after stripping quotes: %s\n", buffer);
  assert(strcmp(buffer, expected) == 0);
}

int main()
{
  test("\"\"", "");
  test("", "");
  test("\"", "\"");
  test("hi", "hi");
  test("\"hi\"", "hi");
  test("boy oh boy is this fun!", "boy oh boy is this fun!");
  test("\"boy oh boy is this fun!\"", "boy oh boy is this fun!");
  test("boy \"oh boy\" is this fun!\"", "boy \"oh boy\" is this fun!\"");

  // should fail because buffer is too small:
  puts("Trying to use a buffer that is too small, should refuse...");
  char smallbuffer[5];
  bool s = ArUtil::stripQuotes(smallbuffer, "hello world", 5);
  assert(s == false);
  puts("\t...ok.");

  std::string str("hello world no quotes");
  printf("Testing stripping quotes in std::strings in place....\nBefore: %s\n", str.c_str());
  s = ArUtil::stripQuotes(&str);
  assert(s);
  printf("\t...after: %s\n", str.c_str());
  assert(str == "hello world no quotes");

  
  str = "\"quoted string\"";
  printf("Before: %s\n", str.c_str());
  s = ArUtil::stripQuotes(&str);
  printf("\t...after: %s\n", str.c_str());
  assert(s);
  assert(str == "quoted string");

  puts("stripQuoteTest done.");
  return 0;
}
