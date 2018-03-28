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

#include "ArGPS.h"
#include "assert.h"

class TestArGPS : public ArGPS {
public:
  TestArGPS() : ArGPS() { }
  int tests();
};

int main(int argc, char **argv) {
  return TestArGPS().tests();
}

bool floatsCloseEnough(double x, double y, double eps = 0.000001)
{
  return( fabs(x - y) < eps );
}

int TestArGPS::tests() {

  puts("");
  double degmin = 4248.32544;
  double deg = gpsDegminToDegrees(degmin);
  printf("Degrees/minutes combination %f converts to decimal degrees %f.\nChecking against correct value 42.805424...\n", degmin, deg);
  assert(floatsCloseEnough(deg, 42.805424));
  puts("OK");

  puts("");
  degmin = -4248.32544;
  deg = gpsDegminToDegrees(degmin);
  printf("Degrees/minutes combination %f converts to decimal degrees %f.\nChecking against correct value -42.805424... \n", degmin, deg);
  assert(floatsCloseEnough(deg, -42.805424));
  puts("OK");

  puts("");
  ArNMEAParser::MessageVector msgParts;
  msgParts.push_back("GPRMC");
  msgParts.push_back("1.1");     // timestamp
  msgParts.push_back("A");       // warning flag
  msgParts.push_back("4248.32544");  // latitude
  msgParts.push_back("N");           // lat n/s
  msgParts.push_back("7234.293016");  // longitude
  msgParts.push_back("E");           // lon e/w
  msgParts.push_back("0");           // speed
  ArNMEAParser::Message msg;
  msg.message = &msgParts;
  handleGPRMC(msg);
  printf("Extracted and converted data from preconstructed GPRMC message with north latitude, east latitude: lat=%f, lon=%f, time.sec=%lu, time.msec=%lu, speed=%f.\nChecking against correct values lat=42.805424 lon=72.571550, time.sec=1, time.mseg=100, speed=0.0...\n", myData.latitude, myData.longitude, myData.GPSPositionTimestamp.getSec(), myData.GPSPositionTimestamp.getMSec(), myData.speed);
  assert(myData.havePosition);
  assert(floatsCloseEnough(myData.latitude, 42.805424));
  assert(floatsCloseEnough(myData.longitude, 72.571550));
  assert(myData.GPSPositionTimestamp.getSec() == 1);
  assert(myData.GPSPositionTimestamp.getMSec() == 100);
  assert(floatsCloseEnough(myData.speed, 0.0));
  puts("OK");

  puts("");
  msgParts[4] = "S";
  msgParts[6] = "W";
  handleGPRMC(msg);
  printf("Extracted and converted data from preconstructed GPRMC message with south latitude, west longitude: lat=%f, lon=%f, time.sec=%lu, time.msec=%ld, speed=%f.\nChecking against correct values lat=-42.805424 lon=-72.57155...\n", myData.latitude, myData.longitude, myData.GPSPositionTimestamp.getSec(), myData.GPSPositionTimestamp.getMSec(), myData.speed);
  assert(myData.havePosition);
  assert(floatsCloseEnough(myData.latitude, -42.805424));
  assert(floatsCloseEnough(myData.longitude, -72.57155));
  puts("OK");

  puts("");
  msgParts[6] = "xxx";
  msgParts[1] = "2";
  handleGPRMC(msg);
  printf("Extracted and converted data from preconstructed GPRMC message with south latitude, invalid (\"xxx\") longitude, message should have been ignored (timestamp should still show 1/100, not 2.): lat=%f, lon=%f, time.sec=%lu, time.msec=%ld, speed=%f.\nChecking against correct values lat=-42.805424 lon=-72.57155, time.sec!=2...\n", myData.latitude, myData.longitude, myData.GPSPositionTimestamp.getSec(), myData.GPSPositionTimestamp.getMSec(), myData.speed);
  assert(floatsCloseEnough(myData.latitude, -42.805424));
  assert(floatsCloseEnough(myData.longitude, -72.57155));
  assert(myData.GPSPositionTimestamp.getSec() != 2);
  puts("OK");


  puts("");
  puts("Now testing messages with invalid NMEA format (checksums should be seen as wrong, or messages ignored).");
  char *malformedMessage1 = "$GPRMC,11\r\n99,22$33,44*5*6\n" ;   // broken halfway through
  char *malformedMessage2 = "xxx\r\nxxxx\r\n";  // not an nmea message
  char *malformedMessage3 = "$*01\r\n"; // missing key contents. maybe acepted by ignored?
  char *malformedMessage4 = "$GPRMC,11,22,33\r\n"; // missing checksum
  char *malformedMessage5 = "$GPRMC,11,22$GPRMC,99,$GPR33MC88,77*01\r**0\n203\r\n";  // overlapping messages
  puts("\nMessage 1 of 5...");
  myNMEAParser.parse(malformedMessage1, strlen(malformedMessage1));
  myNMEAParser.parse("\r\n\r\n", 4);
  puts("\nMessage 2 of 5...");
  myNMEAParser.parse(malformedMessage2, strlen(malformedMessage2));
  myNMEAParser.parse("\r\n\r\n", 4);
  puts("\nMessage 3 of 5...");
  myNMEAParser.parse(malformedMessage3, strlen(malformedMessage3));
  myNMEAParser.parse("\r\n\r\n", 4);
  puts("\nMessage 4 of 5...");
  myNMEAParser.parse(malformedMessage4, strlen(malformedMessage4));
  myNMEAParser.parse("\r\n\r\n", 4);
  puts("\nMessage 5 of 5...");
  myNMEAParser.parse(malformedMessage5, strlen(malformedMessage5));
  myNMEAParser.parse("\r\n\r\n", 4);

  puts("");
  char *text = "HCHDM,123.4,M";
  char chk = 0;
  for(unsigned int i = 0; i < strlen(text); ++i)
  {
    chk ^= text[i];
  }
  printf("Checking checksum algorithm: Checksum for example message contents \"%s\" is 0x%X (%d). (ought to be 0x2D). Checking...\n", text, chk&0xFF, chk&0xFF);
  assert(chk == 0x2D);
  puts("OK");
  

  puts("");
  puts("Testing a message with a correct checksum. Should be no checksum warnings.");
  char *messageWithGoodChecksum = "$HCHDM,123.4,M*2D\r\n";
  myNMEAParser.parse(messageWithGoodChecksum, strlen(messageWithGoodChecksum));

  puts("");
  puts("Testing messages with incorrect checksums. Should see checksum warnings.");
  char *messageWithBadChecksum = "$HCHDM,123.4,M*23\r\n";
  myNMEAParser.parse(messageWithBadChecksum, strlen(messageWithBadChecksum));
  myNMEAParser.parse("\r\n\r\n", 4);
  messageWithBadChecksum = "$HCHDM,123.4,z*2D\r\n";
  myNMEAParser.parse(messageWithBadChecksum, strlen(messageWithBadChecksum));
  myNMEAParser.parse("\r\n\r\n", 4);

  puts("");
  puts("Done.");
  return 0;
}
