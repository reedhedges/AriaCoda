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

#include "ArNMEAParser.h"
#include "assert.h"

int main(int argc, char **argv) {

  ArNMEAParser nmeaParser;
  int result = 0;

  puts("");
  puts("Now testing messages with invalid NMEA format (checksums should be seen as wrong, or messages ignored).");
  const char *malformedMessage1 = "$GPRMC,11\r\n99,22$33,44*5*6\n" ;   // broken halfway through
  const char *malformedMessage2 = "xxx\r\nxxxx\r\n";  // not an nmea message, should just be ignored
  const char *malformedMessage3 = "$*01\r\n"; // missing key contents, except bogus checksum. error.
  const char *malformedMessage4 = "$GPRMC,11,22,33\r\n"; // missing checksum
  const char *malformedMessage5 = "$GPRMC,11,22$GPRMC,99,$GPR33MC88,77*01\r**0\n203\r\n";  // overlapping messages
  puts("\nMessage 1 of 5 (abrupt end of message)...");
  result = nmeaParser.parse(malformedMessage1, strlen(malformedMessage1));
  assert(result & ArNMEAParser::ParseError);
  nmeaParser.parse("\r\n\r\n", 4);
  puts("\nMessage 2 of 5 (no NMEA syntactical characters present)...");
  result = nmeaParser.parse(malformedMessage2, strlen(malformedMessage2));
  assert(!(result & ArNMEAParser::ParseError));
  nmeaParser.parse("\r\n\r\n", 4);
  puts("\nMessage 3 of 5 (no contents, just start and (bogus) checksum)...");
  result = nmeaParser.parse(malformedMessage3, strlen(malformedMessage3));
  assert(result & ArNMEAParser::ParseError);
  nmeaParser.parse("\r\n\r\n", 4);
  puts("\nMessage 4 of 5 (missing checksum)...");
  result = nmeaParser.parse(malformedMessage4, strlen(malformedMessage4));
  assert(result & ArNMEAParser::ParseError);
  nmeaParser.parse("\r\n\r\n", 4);
  puts("\nMessage 5 of 5 (overlapping/corrupted messages)...");
  result = nmeaParser.parse(malformedMessage5, strlen(malformedMessage5));
  assert(result & ArNMEAParser::ParseError);
  nmeaParser.parse("\r\n\r\n", 4);


  puts("");
  puts("Testing a message with a correct checksum. Should be no checksum warnings.");
  char *messageWithGoodChecksum = "$HCHDM,123.4,M*2D\r\n";
  result = nmeaParser.parse(messageWithGoodChecksum, strlen(messageWithGoodChecksum));
  assert(!(result & ArNMEAParser::ParseError));

  puts("");
  puts("Testing messages with incorrect checksums. Should see checksum warnings.");
  char *messageWithBadChecksum = "$HCHDM,123.4,M*23\r\n";
  result = nmeaParser.parse(messageWithBadChecksum, strlen(messageWithBadChecksum));
  assert(result & ArNMEAParser::ParseError);
  nmeaParser.parse("\r\n\r\n", 4);
  messageWithBadChecksum = "$HCHDM,123.4,z*2D\r\n";
  result = nmeaParser.parse(messageWithBadChecksum, strlen(messageWithBadChecksum));
  assert(result & ArNMEAParser::ParseError);
  nmeaParser.parse("\r\n\r\n", 4);

  puts("");
  puts("Testing very long message.");
  char longMessage[256];
  char c = 'a';
  for(int i = 0; i <= 256; ++i)
  {
    longMessage[i] = c;
    if(c == 'z') c = 'A';
    else if(c == 'Z') c = 'a';
    else ++c;
  }
  result = nmeaParser.parse(longMessage, 256);
  assert(!(result & ArNMEAParser::ParseError));
  assert(!(result & ArNMEAParser::ParseUpdated));
    

  puts("");
  puts("Done.");
  return 0;
}
