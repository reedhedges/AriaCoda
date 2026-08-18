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

#include "Aria/ArNMEAParser.h"
#include "Aria/ArFunctor.h"
#include "assert.h"
#include <string>
#include <vector>

static int hdmHandlerCalls = 0;
static int ggaHandlerCalls = 0;
static std::string lastId;
static std::string lastPrefix;
static std::vector<std::string> lastFields;

static void onHDM(ArNMEAParser::Message msg)
{
  ++hdmHandlerCalls;
  lastId = msg.id;
  lastPrefix = msg.prefix;
  lastFields = *msg.message;
}

static void onGGA(ArNMEAParser::Message msg)
{
  ++ggaHandlerCalls;
  lastId = msg.id;
  lastPrefix = msg.prefix;
  lastFields = *msg.message;
}

int main(int argc, char **argv) {

  ArNMEAParser nmeaParser;
  int result = 0;
  ArGlobalFunctor1<ArNMEAParser::Message> hdmHandler(&onHDM);
  nmeaParser.addHandler("HDM", &hdmHandler);

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
  const char *messageWithGoodChecksum = "$HCHDM,123.4,M*2D\r\n";
  result = nmeaParser.parse(messageWithGoodChecksum, strlen(messageWithGoodChecksum));
  assert(!(result & ArNMEAParser::ParseError));

  puts("");
  puts("Testing messages with incorrect checksums. Should see checksum warnings.");
  const char *messageWithBadChecksum = "$HCHDM,123.4,M*23\r\n";
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
  for(int i = 0; i <= 255; ++i)
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
  puts("Testing ParseFinished on empty input.");
  result = nmeaParser.parse("", 0);
  assert(result == ArNMEAParser::ParseFinished);
  assert(!(result & ArNMEAParser::ParseError));

  puts("");
  puts("Testing message split across two parse() calls.");
  {
    ArNMEAParser splitParser;
    ArGlobalFunctor1<ArNMEAParser::Message> splitHandler(&onHDM);
    splitParser.addHandler("HDM", &splitHandler);
    hdmHandlerCalls = 0;

    const char *part1 = "$HCHDM,12";
    const char *part2 = "3.4,M*2D\r\n";
    result = splitParser.parse(part1, strlen(part1));
    assert(!(result & ArNMEAParser::ParseError));
    assert(!(result & ArNMEAParser::ParseUpdated));
    assert(hdmHandlerCalls == 0);

    result = splitParser.parse(part2, strlen(part2));
    assert(!(result & ArNMEAParser::ParseError));
    assert(result & ArNMEAParser::ParseUpdated);
    assert(hdmHandlerCalls == 1);
  }

  puts("");
  puts("Testing multiple complete messages in one buffer.");
  {
    ArNMEAParser multiParser;
    ArGlobalFunctor1<ArNMEAParser::Message> multiHandler(&onHDM);
    multiParser.addHandler("HDM", &multiHandler);
    hdmHandlerCalls = 0;

    const char *twoMessages = "$HCHDM,123.4,M*2D\r\n$HCHDM,123.4,M*2D\r\n";
    result = multiParser.parse(twoMessages, strlen(twoMessages));
    assert(!(result & ArNMEAParser::ParseError));
    assert(result & ArNMEAParser::ParseUpdated);
    assert(hdmHandlerCalls == 2);
  }

  puts("");
  puts("Testing field and message size limits (MaxFieldSize=128, MaxNumFields=50).");
  {
    // One field longer than MaxFieldSize: must abort without crashing or updating.
    ArNMEAParser fieldParser;
    ArGlobalFunctor1<ArNMEAParser::Message> fieldHandler(&onHDM);
    fieldParser.addHandler("HDM", &fieldHandler);
    hdmHandlerCalls = 0;

    std::string longFieldMsg = "$";
    longFieldMsg.append(129, 'x'); // > MaxFieldSize (128)
    longFieldMsg += "*00\r\n";
    result = fieldParser.parse(longFieldMsg.c_str(), longFieldMsg.size());
    assert(!(result & ArNMEAParser::ParseError));
    assert(!(result & ArNMEAParser::ParseUpdated));
    assert(hdmHandlerCalls == 0);

    // More fields than MaxNumFields: must abort without crashing or updating.
    ArNMEAParser fieldsParser;
    ArGlobalFunctor1<ArNMEAParser::Message> fieldsHandler(&onHDM);
    fieldsParser.addHandler("HDM", &fieldsHandler);
    hdmHandlerCalls = 0;

    std::string manyFields = "$ID";
    for (int i = 0; i < 51; ++i) // 51 commas => > MaxNumFields (50) after pushes
      manyFields += ',';
    manyFields += "x*00\r\n";
    result = fieldsParser.parse(manyFields.c_str(), manyFields.size());
    assert(!(result & ArNMEAParser::ParseError));
    assert(!(result & ArNMEAParser::ParseUpdated));
    assert(hdmHandlerCalls == 0);
  }

  puts("");
  puts("Testing handler callback fields, id, and prefix (HCHDM).");
  {
    ArNMEAParser hdmParser;
    ArGlobalFunctor1<ArNMEAParser::Message> hdmCb(&onHDM);
    hdmParser.addHandler("HDM", &hdmCb);
    hdmHandlerCalls = 0;
    lastId.clear();
    lastPrefix.clear();
    lastFields.clear();

    const char *msg = "$HCHDM,123.4,M*2D\r\n";
    result = hdmParser.parse(msg, strlen(msg));
    assert(!(result & ArNMEAParser::ParseError));
    assert(result & ArNMEAParser::ParseUpdated);
    assert(hdmHandlerCalls == 1);
    assert(lastId == "HDM");
    assert(lastPrefix == "HC");
    assert(lastFields.size() == 3);
    assert(lastFields[0] == "HCHDM");
    assert(lastFields[1] == "123.4");
    assert(lastFields[2] == "M");
  }

  puts("");
  puts("Testing talker prefix stripping (GPGGA vs GNGGA -> same GGA handler).");
  {
    ArNMEAParser ggaParser;
    ArGlobalFunctor1<ArNMEAParser::Message> ggaCb(&onGGA);
    ggaParser.addHandler("GGA", &ggaCb);

    ggaHandlerCalls = 0;
    lastId.clear();
    lastPrefix.clear();
    lastFields.clear();
    const char *gpgga =
      "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";
    result = ggaParser.parse(gpgga, strlen(gpgga));
    assert(!(result & ArNMEAParser::ParseError));
    assert(result & ArNMEAParser::ParseUpdated);
    assert(ggaHandlerCalls == 1);
    assert(lastId == "GGA");
    assert(lastPrefix == "GP");
    assert(!lastFields.empty());
    assert(lastFields[0] == "GPGGA");

    lastId.clear();
    lastPrefix.clear();
    lastFields.clear();
    const char *gngga =
      "$GNGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*59\r\n";
    result = ggaParser.parse(gngga, strlen(gngga));
    assert(!(result & ArNMEAParser::ParseError));
    assert(result & ArNMEAParser::ParseUpdated);
    assert(ggaHandlerCalls == 2);
    assert(lastId == "GGA");
    assert(lastPrefix == "GN");
    assert(!lastFields.empty());
    assert(lastFields[0] == "GNGGA");
  }

  puts("");
  puts("Done.");
  return 0;
}
