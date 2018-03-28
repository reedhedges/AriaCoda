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


/* Supply a raw log of GPS NMEA data with -infile argument or via stdin to test
 * ArGPS behavior with recorded data.
 */

#include "Aria.h"
#include "ArGPS.h"
#include "ArFileDeviceConnection.h"
#include <iostream>

int main(int argc, char** argv)
{
  Aria::init();

  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();

  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    ArLog::log(ArLog::Terse, "gpsLogTest options:"
                             "\n    -printTable      Print data to standard output in regular columns rather than a refreshing terminal display, and print more digits of precision"
                             "\n    -infile <file>   Specify input file. Use stdin if not given."
                             "\n    -outfile <file>  Specify output file. Use stdout if not given"
    );
    Aria::exit(1);
  }


  // check command line arguments for -printTable
  bool printTable = parser.checkArgument("printTable");

  // Try connecting to a GPS. We pass the robot pointetr to the connector so it
  // can check the robot parameters for this robot type for default values for
  // GPS device connection information (receiver type, serial port, etc.)
  ArLog::log(ArLog::Normal, "gpsLogTest: Connecting to GPS, it may take a few seconds...");
  ArGPS gps;
  ArFileDeviceConnection con;
  const char *infile = NULL;
  const char *outfile = NULL;
  if(!parser.checkParameterArgumentString("infile", &infile)) infile = NULL;
  if(!parser.checkParameterArgumentString("outfile", &outfile)) outfile = NULL;
  if(con.open(infile, outfile) != 0)
  {
    ArLog::log(ArLog::Normal, "gpsLogTest: Error opening log file(s)");
    Aria::exit(-1);
    return -1;
  }
  con.setForceReadBufferSize(85);
  con.setReadByteDelay(2);
  gps.setDeviceConnection(&con);

  if(!gps.connect())
  {
    ArLog::log(ArLog::Terse, "gpsLogTest: Error connecting to GPS device.");
    Aria::exit(-2);
    return -2;
  }

  ArLog::log(ArLog::Normal, "gpsLogTest: Reading data...");
  ArTime lastReadTime;
  if(printTable)
    gps.printDataLabelsHeader();

  while(true)
  {
    int r = gps.read();
printf("%d\n" , r);
    if(r & ArGPS::ReadError)
    {
      ArLog::log(ArLog::Terse, "gpsLogTest: Warning: error reading GPS data.");
      Aria::exit(-5);
      return -5;
    }


    if(r & ArGPS::ReadUpdated)
    {
      if(printTable)
      {
        gps.printData(false);
        printf("\n");
      }
      else
      {
        gps.printData();
        printf("\r");
      }
      fflush(stdout);
      ArUtil::sleep(500);
      lastReadTime.setToNow();
      continue;
    } else {
      if(lastReadTime.secSince() >= 10) {
        Aria::exit(0);
      }
      else if(lastReadTime.secSince() >= 5) {
        ArLog::log(ArLog::Terse, "gpsLogTest: Warning: haven't recieved any data from GPS for more than 5 seconds!");
      }
      ArUtil::sleep(1000);
      continue;
    }

    if(r & ArGPS::ReadFinished)
      Aria::exit(0);

  }
  Aria::exit(0);
  return 0;
}
