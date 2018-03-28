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

#include "ArExport.h"
#include "ariaOSDef.h"
#include "ariaInternal.h"
#include "ariaUtil.h"
#include "ArTCM2.h"
#include "ArTCMCompassDirect.h"
#include "ArTCMCompassRobot.h"

AREXPORT ArTCM2::ArTCM2() :
  myHeading(0.0),
  myPitch(0.0),
  myRoll(0.0),
  myXMag(0.0),
  myYMag(0.0),
  myZMag(0.0),
  myTemperature(0.0),
  myError(0),
  myCalibrationH(0.0),
  myCalibrationV(0.0),
  myCalibrationM(0.0),
  myHaveHeading(false), 
  myHavePitch(false),
  myHaveRoll(false),
  myHaveXMag(false),
  myHaveYMag(false),
  myHaveZMag(false),
  myHaveTemperature(false),
  myHaveCalibrationH(false),
  myHaveCalibrationV(false),
  myHaveCalibrationM(false),
  myTimeLastPacket(0),
  myPacCurrentCount(0),
  myPacCount(0)
{
}

AREXPORT bool ArTCM2::connect() { return true; }
AREXPORT bool ArTCM2::blockingConnect(unsigned long) { return true; }

AREXPORT ArCompassConnector::ArCompassConnector(ArArgumentParser *argParser) :
  myArgParser(argParser),
  myParseArgsCallback(this, &ArCompassConnector::parseArgs),
  myLogArgsCallback(this, &ArCompassConnector::logOptions),
  myDeviceType(None),
  mySerialPort(ARTCM2_DEFAULT_SERIAL_PORT),
  mySerialTCMReadFunctor(NULL),
  myRobot(NULL)
{
  myParseArgsCallback.setName("ArCompassConnector");
  myLogArgsCallback.setName("ArCompassConnector");
  Aria::addParseArgsCB(&myParseArgsCallback);
  Aria::addLogOptionsCB(&myLogArgsCallback);
}

AREXPORT ArCompassConnector::~ArCompassConnector() {
  if(mySerialTCMReadFunctor && myRobot) 
  {
    myRobot->lock();
    myRobot->remSensorInterpTask(mySerialTCMReadFunctor);
    delete mySerialTCMReadFunctor;
    myRobot->unlock();
  }
}

bool ArCompassConnector::parseArgs()
{
  if(!myArgParser) return false;
  if(!myArgParser->checkParameterArgumentString("-compassPort", &mySerialPort)) return false;
  char *deviceType = myArgParser->checkParameterArgument("-compassType");
  if(deviceType)
  {
    if(strcasecmp(deviceType, "robot") == 0)
      myDeviceType = Robot;
    else if(strcasecmp(deviceType, "serialtcm") == 0)
      myDeviceType = SerialTCM;
    else if(strcasecmp(deviceType, "tcm") == 0)
      myDeviceType = SerialTCM;
    else
    {
      ArLog::log(ArLog::Terse, "ArCompassConnector: Error: unrecognized -compassType option: %s. Valid values are robot and serialTCM.", deviceType);
      return false;
    }
  }
  return true;
}

void ArCompassConnector::logOptions()
{
  ArLog::log(ArLog::Terse, "Compass options:");
  ArLog::log(ArLog::Terse, "-compassType <robot|serialTCM>\tSelect compass device type (default: robot)");
  ArLog::log(ArLog::Terse, "-compassPort <port>\tSerial port for \"serialTCM\" type compass. (default: %s)", ARTCM2_DEFAULT_SERIAL_PORT);
}

AREXPORT ArTCM2 *ArCompassConnector::create(ArRobot *robot)
{
  if(myDeviceType == None)
  {
    if(robot && robot->getRobotParams())
    {
      const char *type = robot->getRobotParams()->getCompassType();
      if(type == NULL || strlen(type) == 0 || strcmp(type, "robot") == 0)
      {
        myDeviceType = Robot;
      }
      else if(strcmp(type, "serialTCM") == 0)
      {
        myDeviceType = SerialTCM;
        const char *port = robot->getRobotParams()->getCompassPort();
        if(port == NULL || strlen(port) == 0)
          mySerialPort = ARTCM2_DEFAULT_SERIAL_PORT;
        else
          mySerialPort = port;
      }
      else
      {
        ArLog::log(ArLog::Terse, "ArCompassConnector: Error: invalid compass type \"%s\" in robot parameters.", type);
        return NULL;
      }
    }
    else
    {
      myDeviceType = Robot;
    }
  }

  switch(myDeviceType)
  {
    case Robot:
      ArLog::log(ArLog::Verbose, "ArCompassConnector: Using robot compass");
      return new ArTCMCompassRobot(robot);
    case SerialTCM:
      {
        ArLog::log(ArLog::Verbose, "ArCompassConnector: Using TCM2 compass on serial port %s", mySerialPort);
        ArTCMCompassDirect *newDirectTCM = new ArTCMCompassDirect(mySerialPort);
        mySerialTCMReadFunctor = new ArRetFunctor1C<int, ArTCMCompassDirect, unsigned int>(newDirectTCM, &ArTCMCompassDirect::read, 1);
        robot->lock();
        robot->addSensorInterpTask("ArTCMCompassDirect read", 200, mySerialTCMReadFunctor);
        myRobot = robot;
        robot->unlock();
        return newDirectTCM;
      }
    case None:
    default:
      // break out of switch and print warning there
      break;
  }
  ArLog::log(ArLog::Terse, "ArCompassConnector: Error: No compass type selected.");
  return NULL;
}

AREXPORT bool ArCompassConnector::connect(ArTCM2 *compass) const
{
  return compass->blockingConnect();
}

