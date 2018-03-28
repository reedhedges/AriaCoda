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
#include "ArMTXIO.h"

int testDigitalOut(ArMTXIO *mtxIO);
int testAnalogIn(ArMTXIO *mtxIO);
int testFPGARead(ArMTXIO *mtxio);

int main(int argc, char **argv)
{
  Aria::init();
  ArArgumentParser parser(&argc, argv);
  parser.loadDefaultArguments();
  ArRobot robot;

  ArRobotConnector robotConnector(&parser, &robot);
  robot.setConnectWithNoParams(true);
  if(!robotConnector.connectRobot())
  {
    ArLog::log(ArLog::Terse, "mtxIOTest: Could not connect to the robot.");
    if(parser.checkHelpAndWarnUnparsed())
    {
        // -help not given
        Aria::logOptions();
        Aria::exit(1);
    }
  }

  if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
  {
    Aria::logOptions();
    Aria::exit(1);
  }
  ArLog::log(ArLog::Normal, "mtxIOTest: Connected.");
  robot.comInt(ArCommands::JOYINFO, 0);

  ArMTXIO mtxIO;

  if(!mtxIO.isEnabled())
  {
    ArLog::log(ArLog::Terse, "mtxIOTest: Error opening MTX IO device interface!");
    Aria::exit(4);
  }

  robot.runAsync(true);

  ArLog::log(ArLog::Normal, "mtxIOTest: testFPGARead");
  testFPGARead(&mtxIO);
  /*
  ArLog::log(ArLog::Normal, "mtxIOTest: testDigitalOut");
  for(int n = 0; n < 100; n++)
  {
	  testDigitalOut(&mtxIO);
  }
*/

  ArLog::log(ArLog::Normal, "mtxIOTest: testAnalogIn");
  testAnalogIn(&mtxIO);




  Aria::exit(0);

}

int testFPGARead(ArMTXIO *mtxio)
{
    unsigned char val;
    unsigned short reg;

	for(reg = 0; reg < 0x501; reg++){
        mtxio->lock();
        if(mtxio->getRegValue(reg, &val)){
            ArLog::log(ArLog::Normal, "0x%04x\t0x%02x", reg, val);
        } else {
            ArLog::log(ArLog::Normal, "0x%04x\t error", reg);
            //return false;
        }
        mtxio->unlock();
	}
	return true;
}

int testDigitalOut(ArMTXIO *mtxIO)
{
	unsigned char cur;
	unsigned char out = 1;

    mtxIO->lock();
    // get current state
    if(!mtxIO->getDigitalOutputControl1(&cur))
    {
      ArLog::log(ArLog::Terse, "mtxIOTest: Error getting current state of output control 1");
      mtxIO->unlock();
      Aria::exit(2);
    }


    // set new state
    cur = out;
    if(!mtxIO->setDigitalOutputControl1(&cur))
    {
      ArLog::log(ArLog::Terse, "mtxIOTest: Error setting state of output control 1");
      mtxIO->unlock();
      Aria::exit(3);
    }


    // shift
    out = out << 1;
    if(out == 0) out = 1;

    mtxIO->unlock();

    return 1;
}

int testAnalogIn(ArMTXIO *mtxIO)
{
	unsigned short val;

    mtxIO->lock();
    for(int i = 0; i < 16; i++){
		mtxIO->getAnalogIOBlock1(i, &val);
		ArLog::log(ArLog::Terse, "mtxIOTest:testAnalogIn: Block1: port %i :  %i.", i, val);
    }
    for(int i = 0; i < 4; i++){
        mtxIO->getAnalogIOBlock2(i, &val);
        ArLog::log(ArLog::Terse, "mtxIOTest:testAnalogIn: Block2: port %i :  %i.", i, val);
    }
    mtxIO->unlock();

    return 1;
}
