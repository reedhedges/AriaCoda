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

#include "Aria/ArGPS.h"
#include "assert.h"
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

class TestArGPS : public ArGPS {
public:
  TestArGPS() : ArGPS() { }
  int tests();
};

int main(int /*argc*/, char ** /*argv*/) {
  return TestArGPS().tests();
}

bool floatsCloseEnough(double x, double y, double eps = 0.000001)
{
  return( fabs(x - y) < eps );
}

static ArNMEAParser::Message makeMsg(ArNMEAParser::MessageVector *parts,
                                     const char *id = "")
{
  ArNMEAParser::Message msg;
  msg.message = parts;
  msg.id = id;
  if (id[0] == '\0' && parts && !parts->empty() && (*parts)[0].size() >= 2)
    msg.id = (*parts)[0].substr(2);
  if (parts && !parts->empty() && (*parts)[0].size() >= 2)
    msg.prefix = (*parts)[0].substr(0, 2);
  return msg;
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
  puts("gpsDegminToDegrees edge cases...");
  assert(floatsCloseEnough(gpsDegminToDegrees(0.0), 0.0));
  assert(floatsCloseEnough(gpsDegminToDegrees(18060.0), 181.0)); // 180 deg + 60 min
  assert(floatsCloseEnough(gpsDegminToDegrees(0.0001), 0.0001 / 60.0));
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
  ArNMEAParser::Message msg = makeMsg(&msgParts, "RMC");
  handleGPRMC(msg);
  printf("Extracted and converted data from preconstructed GPRMC message with north latitude, east latitude: lat=%f, lon=%f, time.sec=%lu, time.msec=%lu, speed=%f.\nChecking against correct values lat=42.805424 lon=72.571550, time.sec=1, time.mseg=100, speed=0.0...\n", myData.latitude, myData.longitude, myData.GPSPositionTimestamp.getSec(), myData.GPSPositionTimestamp.getMSec(), myData.speed);
  assert(myData.havePosition);
  assert(myData.qualityFlag);
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
  puts("Invalid latitude hemisphere should leave prior position unchanged...");
  msgParts[1] = "1.1";
  msgParts[2] = "A";
  msgParts[4] = "S";
  msgParts[6] = "W";
  handleGPRMC(msg); // restore known-good S/W position
  assert(floatsCloseEnough(myData.latitude, -42.805424));
  msgParts[4] = "X";
  msgParts[1] = "9";
  handleGPRMC(msg);
  assert(floatsCloseEnough(myData.latitude, -42.805424));
  assert(floatsCloseEnough(myData.longitude, -72.57155));
  assert(myData.GPSPositionTimestamp.getSec() != 9);
  puts("OK");

  puts("");
  puts("GPRMC void flag (V): qualityFlag false; position still updates...");
  msgParts[1] = "3.3";
  msgParts[2] = "V";
  msgParts[3] = "4000.0";
  msgParts[4] = "N";
  msgParts[5] = "7000.0";
  msgParts[6] = "E";
  msgParts[7] = "0";
  handleGPRMC(msg);
  assert(myData.havePosition);
  assert(!myData.qualityFlag);
  assert(floatsCloseEnough(myData.latitude, 40.0));
  assert(floatsCloseEnough(myData.longitude, 70.0));
  puts("OK");

  puts("");
  puts("GPRMC non-zero speed (knots -> m/s)...");
  msgParts[1] = "4.4";
  msgParts[2] = "A";
  msgParts[3] = "4248.32544";
  msgParts[4] = "N";
  msgParts[5] = "7234.293016";
  msgParts[6] = "E";
  msgParts[7] = "10.0";
  handleGPRMC(msg);
  assert(myData.haveSpeed);
  assert(floatsCloseEnough(myData.speed, knotsToMPS(10.0)));
  puts("OK");

  puts("");
  puts("GPRMC fractional timestamps...");
  // readTimeFromString: whole part -> sec; fractional digits * 100 -> msec
  msgParts[1] = "123456.789";
  msgParts[2] = "A";
  handleGPRMC(msg);
  assert(myData.GPSPositionTimestamp.getSec() == 123456);
  assert(myData.GPSPositionTimestamp.getMSec() == 78900); // atoi("789") * 100

  msgParts[1] = "000001.0";
  handleGPRMC(msg);
  assert(myData.GPSPositionTimestamp.getSec() == 1); // leading zeros ignored by atoi
  assert(myData.GPSPositionTimestamp.getMSec() == 0);

  msgParts[1] = "12.34";
  handleGPRMC(msg);
  assert(myData.GPSPositionTimestamp.getSec() == 12);
  assert(myData.GPSPositionTimestamp.getMSec() == 3400); // atoi("34") * 100
  puts("OK");

  puts("");
  puts("handleGPGGA: fix type, satellites, HDOP, altitude...");
  {
    ArNMEAParser::MessageVector gga;
    gga.push_back("GPGGA");
    gga.push_back("123519");
    gga.push_back("4807.038");
    gga.push_back("N");
    gga.push_back("01131.000");
    gga.push_back("E");
    gga.push_back("1");       // GPS fix
    gga.push_back("08");      // satellites
    gga.push_back("0.9");     // HDOP
    gga.push_back("545.4");   // altitude
    gga.push_back("M");
    gga.push_back("46.9");
    gga.push_back("M");
    gga.push_back("");
    gga.push_back("123");     // DGPS station
    handleGPGGA(makeMsg(&gga, "GGA"));
    assert(myData.fixType == GPSFix);
    assert(myData.numSatellitesTracked == 8);
    assert(myData.haveHDOP);
    assert(floatsCloseEnough(myData.HDOP, 0.9));
    assert(myData.haveAltitude);
    assert(floatsCloseEnough(myData.altitude, 545.4));
    assert(myData.haveDGPSStation);
    assert(myData.DGPSStationID == 123);
  }
  puts("OK");

  puts("");
  puts("handleGPGSA: PDOP/HDOP/VDOP...");
  {
    ArNMEAParser::MessageVector gsa;
    gsa.push_back("GPGSA");
    gsa.push_back("A");
    gsa.push_back("3");
    for (int i = 0; i < 12; ++i)
      gsa.push_back(""); // satellite ID slots (indices 3-14)
    gsa.push_back("1.5");  // PDOP index 15
    gsa.push_back("1.2");  // HDOP index 16
    gsa.push_back("2.0");  // VDOP index 17
    handleGPGSA(makeMsg(&gsa, "GSA"));
    assert(myData.havePDOP);
    assert(myData.haveHDOP);
    assert(myData.haveVDOP);
    assert(floatsCloseEnough(myData.PDOP, 1.5));
    assert(floatsCloseEnough(myData.HDOP, 1.2));
    assert(floatsCloseEnough(myData.VDOP, 2.0));
  }
  puts("OK");

  puts("");
  puts("handleGPGSV: mean SNR from one completed set...");
  {
    ArNMEAParser::MessageVector gsv;
    gsv.push_back("GPGSV");
    gsv.push_back("1");   // numMsgs
    gsv.push_back("1");   // thisMsg
    gsv.push_back("01");  // sats in view
    gsv.push_back("01");  // PRN
    gsv.push_back("40");  // elev
    gsv.push_back("083"); // az
    gsv.push_back("46");  // SNR
    handleGPGSV(makeMsg(&gsv, "GSV"));
    assert(myData.haveSNR);
    assert(floatsCloseEnough(myData.meanSNR, 46.0));
  }
  puts("OK");

  puts("");
  puts("handleHCHDx: magnetic and true headings...");
  {
    ArNMEAParser::MessageVector hdm;
    hdm.push_back("HCHDM");
    hdm.push_back("123.4");
    hdm.push_back("M");
    handleHCHDx(makeMsg(&hdm, "HDM"));
    assert(myData.haveCompassHeadingMag);
    assert(floatsCloseEnough(myData.compassHeadingMag, 123.4));

    ArNMEAParser::MessageVector hdt;
    hdt.push_back("HCHDT");
    hdt.push_back("200.5");
    hdt.push_back("T");
    handleHCHDx(makeMsg(&hdt, "HDT"));
    assert(myData.haveCompassHeadingTrue);
    assert(floatsCloseEnough(myData.compassHeadingTrue, 200.5));
  }
  puts("OK");

  puts("");
  puts("handleGPMSS / handleGPGST / handlePGRMZ smoke...");
  {
    ArNMEAParser::MessageVector mss;
    mss.push_back("GPMSS");
    mss.push_back("25.5");  // signal
    mss.push_back("12.0");  // snr
    mss.push_back("300.0"); // freq
    mss.push_back("100");   // bps
    mss.push_back("7");     // channel
    handleGPMSS(makeMsg(&mss, "MSS"));
    assert(myData.haveBeaconInfo);
    assert(floatsCloseEnough(myData.beaconSignalStrength, 25.5));
    assert(floatsCloseEnough(myData.beaconSNR, 12.0));
    assert(floatsCloseEnough(myData.beaconFreq, 300.0));
    assert(myData.beaconBPS == 100);
    assert(myData.beaconChannel == 7);

    ArNMEAParser::MessageVector gst;
    gst.push_back("GPGST");
    gst.push_back("123456.0");
    gst.push_back("0.5");   // inputs RMS
    gst.push_back("1.1");   // ellipse major
    gst.push_back("0.8");   // ellipse minor
    gst.push_back("45.0");  // orient
    gst.push_back("0.3");   // lat err
    gst.push_back("0.4");   // lon err
    gst.push_back("0.6");   // alt err
    handleGPGST(makeMsg(&gst, "GST"));
    assert(myData.haveInputsRMS);
    assert(floatsCloseEnough(myData.inputsRMS, 0.5));
    assert(myData.haveErrorEllipse);
    assert(floatsCloseEnough(myData.errorEllipse.getX(), 0.8)); // minor
    assert(floatsCloseEnough(myData.errorEllipse.getY(), 1.1)); // major
    assert(floatsCloseEnough(myData.errorEllipse.getTh(), 45.0));
    assert(myData.haveLatLonError);

    ArNMEAParser::MessageVector rmz;
    rmz.push_back("PGRMZ");
    rmz.push_back("100.0"); // feet
    rmz.push_back("f");
    handlePGRMZ(makeMsg(&rmz, "RMZ"));
    assert(myData.haveAltimeter);
    assert(floatsCloseEnough(myData.altimeter, feetToMeters(100.0)));
  }
  puts("OK");

  puts("");
  puts("End-to-end: parse full $GPRMC sentence through myNMEAParser...");
  {
    const char *rmc =
      "$GPRMC,1.1,A,4248.32544,N,7234.293016,E,10.0*2F\r\n";
    myNMEAParser.parse(rmc, strlen(rmc));
    assert(myData.havePosition);
    assert(myData.qualityFlag);
    assert(floatsCloseEnough(myData.latitude, 42.805424));
    assert(floatsCloseEnough(myData.longitude, 72.571550));
    assert(myData.haveSpeed);
    assert(floatsCloseEnough(myData.speed, knotsToMPS(10.0)));
    assert(myData.GPSPositionTimestamp.getSec() == 1);
    assert(myData.GPSPositionTimestamp.getMSec() == 100);
  }
  puts("OK");


  puts("");
  puts("Now testing messages with invalid NMEA format (checksums should be seen as wrong, or messages ignored).");
  const char *malformedMessage1 = "$GPRMC,11\r\n99,22$33,44*5*6\n" ;   // broken halfway through
  const char *malformedMessage2 = "xxx\r\nxxxx\r\n";  // not an nmea message
  const char *malformedMessage3 = "$*01\r\n"; // missing key contents. maybe acepted by ignored?
  const char *malformedMessage4 = "$GPRMC,11,22,33\r\n"; // missing checksum
  const char *malformedMessage5 = "$GPRMC,11,22$GPRMC,99,$GPR33MC88,77*01\r**0\n203\r\n";  // overlapping messages
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
  const char *text = "HCHDM,123.4,M";
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
  const char *messageWithGoodChecksum = "$HCHDM,123.4,M*2D\r\n";
  myNMEAParser.parse(messageWithGoodChecksum, strlen(messageWithGoodChecksum));

  puts("");
  puts("Testing messages with incorrect checksums. Should see checksum warnings.");
  const char *messageWithBadChecksum = "$HCHDM,123.4,M*23\r\n";
  myNMEAParser.parse(messageWithBadChecksum, strlen(messageWithBadChecksum));
  myNMEAParser.parse("\r\n\r\n", 4);
  messageWithBadChecksum = "$HCHDM,123.4,z*2D\r\n";
  myNMEAParser.parse(messageWithBadChecksum, strlen(messageWithBadChecksum));
  myNMEAParser.parse("\r\n\r\n", 4);

  puts("");
  puts("Done.");
  return 0;
}
