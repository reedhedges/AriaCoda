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
#include "ArDPPTU.h"
#include "ArCommands.h"
#include "ArBasePacket.h"
#include "ariaInternal.h"
#include "ArDeviceConnection.h"

#define DO_DEBUG(code) {code;}

//#define DEBUG_CMD DO_DEBUG
//#define DEBUG_TIME DO_DEBUG
//#define DEBUG_POS DO_DEBUG

#ifndef DEBUG_CMD
#define DEBUG_CMD(code) {}
#endif

#ifndef DEBUG_TIME
#define DEBUG_TIME(code) {}
#endif

#ifndef DEBUG_POS
#define DEBUG_POS(code) {}
#endif

AREXPORT ArDPPTUPacket::ArDPPTUPacket(ArTypes::UByte2 bufferSize) :
  ArBasePacket(bufferSize, 0)
{
}

AREXPORT ArDPPTUPacket::~ArDPPTUPacket()
{

}

AREXPORT void ArDPPTUPacket::byte2ToBuf(int val)
{
  //ArLog::log(ArLog::Normal, "Putting %d in an DPPTU packet...", val);
  int i;
  char buf[8];
  if (myLength + 4 > myMaxLength)
  {
    ArLog::log(ArLog::Terse, "ArDPPTUPacket::byte2ToBuf: Trying to add beyond length of buffer.");
    return;
  }

  if(val > 9999999 || val < -999999) 
	  ArLog::log(ArLog::Terse, "ArDPPTUPacket::byte2ToBuf: Warning: truncating value %d to 7 digits!", val);

  snprintf(buf, 8, "%d", val);

  for (i=0;i<(int)strlen(buf);i++)
  {
      myBuf[myLength] = buf[i];
      ++myLength;
  }
}

AREXPORT void ArDPPTUPacket::finalizePacket(void)
{
    ArDPPTUPacket::uByteToBuf(ArDPPTUCommands::DELIM);
}

/** @a deviceType If a type other than PANTILT_DEFAULT is given, then manually
 * selects different parameters for different DPPTU models. It is recommended to
 * use PANTILT_DEFAULT (unless your PTU is not responding to resolution
 * queries for some reason)
        
 * @a deviceIndex if you have more than one PTU on a Seekur robot, this
 * specifies one device to power on at startup and power off on exit. If -1
 * (default value), then all possible PTU power ports are powered on.
 */
AREXPORT ArDPPTU::ArDPPTU(ArRobot *robot, DeviceType deviceType, int deviceIndex) :
  ArPTZ(robot),
  myCanGetRealPanTilt(true),
  myInit(false),
  myQueryCB(this, &ArDPPTU::query),
  myGotPanRes(false),
  myGotTiltRes(false)
{
  myRobot = robot;
  myDeviceType = deviceType;

  switch(myDeviceType) {
    case PANTILT_PTUD47:
	  myPanConvert = 0.0514;
      myTiltConvert = 0.0129;
	  ArPTZ::setLimits(158, -158, 30, -46);
	  /*
      myMaxPan = 158;
      myMinPan = -158;
      myMaxTilt = 30;
      myMinTilt = -46;
	  */
      myMaxPanSlew = 149;
      myMinPanSlew = 2;
      myMaxTiltSlew = 149;
      myMinTiltSlew = 2;
      myMaxPanAccel = 102;
      myMinPanAccel = 2;
      myMaxTiltAccel = 102;
      myMinTiltAccel = 2;
      myPanSlew = 40;
      myTiltSlew = 40;
      break;
    case PANTILT_PTUD46:
    case PANTILT_DEFAULT:  
	  // if DEFAULT, then in init() we will query the PTU to get the real conversion factors and limits (but start out assuming same as D46)
	  myPanConvert = 0.0514;
      myTiltConvert = 0.0514;
	  ArPTZ::setLimits(158, -158, 30, -46);
	  /*
      myMaxPan = 158;
      myMinPan = -158;
      myMaxTilt = 30;
      myMinTilt = -46;
	  */
      myMaxPanSlew = 149;
      myMinPanSlew = 2;
      myMaxTiltSlew = 149;
      myMinTiltSlew = 2;
      myMaxPanAccel = 102;
      myMinPanAccel = 2;
      myMaxTiltAccel = 102;
      myMinTiltAccel = 2;
      myPanSlew = 40; //Default to 1000 positions/sec
      myTiltSlew = 40; //Defaults to 1000 positions/sec
    default:
      break;
  }

  if(deviceIndex == -1)
  {
    myPowerPorts.push_back(9);
    myPowerPorts.push_back(22);
    myPowerPorts.push_back(23);
  }
  else if(deviceIndex == 0)
    myPowerPorts.push_back(9);
  else if(deviceIndex == 1)
    myPowerPorts.push_back(22);
  else if(deviceIndex == 2)
    myPowerPorts.push_back(23);
  else
    ArLog::log(ArLog::Terse, "ArDPPTU: Warning: No Seekur power port assignment known for PTU device #%d, won't turn any on.");

  if(myRobot)
  {
    for(std::vector<char>::const_iterator i = myPowerPorts.begin(); i != myPowerPorts.end(); ++i)
    {
      myRobot->com2Bytes(116, *i, 1);
    }
    //myRobot->addDisconnectNormallyCallback(&myShutdownCB);
    //myRobot->addDisconnectOnErrorCallback(&myShutdownCB);
    //myRobot->addUserTask("ArDPPTU", 65, &myQueryCB);
  }

 // Aria::addExitCallback(&myShutdownCB);
    
}

AREXPORT ArDPPTU::~ArDPPTU()
{
  shutdown();
  Aria::remExitCallback(&myShutdownCB);
}

void ArDPPTU::shutdown()
{
  if(!myInit) return;
  haltAll();
  offStatPower();
  lowMotPower();
  if(myRobot)
  {
    for(std::vector<char>::const_iterator i = myPowerPorts.begin(); i != myPowerPorts.end(); ++i)
    {
      myRobot->com2Bytes(116, *i, 0);
    }
    myRobot->remSensorInterpTask(&myQueryCB);
  }
  myInit = false;
}

void ArDPPTU::preparePacket(void)
{
  myPacket.empty();
  myPacket.byteToBuf(ArDPPTUCommands::DELIM);
}

AREXPORT bool ArDPPTU::init(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::INIT);
  if (!sendPacket(&myPacket))
  {
    ArLog::log(ArLog::Terse, "ArDPPTU: Error sending INIT to PTU! (Write error?)");
    return false;
  }

  myPan = -1;  //myPan and myTilt set to -1 for initial positioning
  myTilt = -1;

  setMovePower(PAN, LOW);
  setMovePower(TILT, LOW);
  setHoldPower(PAN, OFF);
  setHoldPower(PAN, OFF);


  switch(myDeviceType) {
    case PANTILT_PTUD47:
      //Assuming default accel and slew rates
      myPanSlew = 40;
      myBasePanSlew = 40;
      myTiltSlew = 40;
      myBaseTiltSlew = 40;
      myPanAccel = 80;
      myTiltAccel = 80;
      break;
    case PANTILT_PTUD46:
    case PANTILT_DEFAULT:
    default:
      //Assuming default accel and slew rates
      myPanSlew = 40; // 1000 positions/sec
      myBasePanSlew = 40; // 1000 positions/sec
      myTiltSlew = 40; // 1000 positions/sec
      myBaseTiltSlew = 40; // 1000 positions/sec
      myPanAccel = 80; // 2000 positions/sec^2
      myTiltAccel = 80; // 2000 positions/sec^2
      break;
  }

  if(myDeviceType == PANTILT_DEFAULT)
  {
    // query resolution, conversion factors will be 
    // set again based on responses (replacing default value set 
    // in constructor)
    preparePacket();
    myPacket.byteToBuf('P');
    myPacket.byteToBuf('R');
    if(!sendPacket(&myPacket))
      ArLog::log(ArLog::Terse, "ArDPPTU: Warning: write error sending pan resolution query");
    // We can't distinguish PR and TR responses based on their content alone, so
    // we have to query pan resolution (PR), then after receiving resolution
    // response, query TR. (see readPacket() for TR).
	///@todo query the device for pan and tilt limits, and when response is received, call ArPTZ::setLimits() to change.
  }

  query();   // do first position query

  if (!panTilt(0,0))
    return false;

  myInit = true;

  return true;
}

/** A blank packet can be sent to exit monitor mode **/
AREXPORT bool ArDPPTU::blank(void)
{
  myPacket.empty();
  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::pan_i(double pdeg)
{
  //ArLog::log(ArLog::Normal, "ArDPPTU::panTilt(%f, %f)", pdeg, tdeg);
  if (pdeg > getMaxPosPan_i())
    pdeg = getMaxPosPan_i();
  if (pdeg < getMaxNegPan_i())
    pdeg = getMaxNegPan_i();

  if (pdeg != myPanSent)
  {
	  DEBUG_CMD(ArLog::log(ArLog::Normal, 
      "ArDPPTU::pan: sending command to pan to %f deg (maxPosPan=%f, minNegPan=%f, myPanSent=%f)", 
      pdeg, getMaxPosPan_i(), getMaxNegPan_i(), myPanSent); 
    )
    preparePacket();
    myPacket.byteToBuf('P');
    myPacket.byteToBuf('P');
    myPacket.byte2ToBuf(ArMath::roundInt(pdeg/myPanConvert));

    myPanSent = pdeg;
    if(!myCanGetRealPanTilt) myPan = myPanSent;
    if (!sendPacket(&myPacket)) return false;
  }
return true;
}

AREXPORT bool ArDPPTU::tilt_i(double tdeg)
{
  if (tdeg > getMaxPosTilt_i())
    tdeg = getMaxPosTilt_i();
  if (tdeg < getMaxNegTilt_i())
    tdeg = getMaxNegTilt_i();

  if (tdeg != myTiltSent)
  {
	  DEBUG_CMD(ArLog::log(ArLog::Normal, 
      "ArDPPTU::tilt: sending command to tilt to %f deg (maxPosTilt=%f, minNegTilt=%f, myTiltSent=%f)", 
      tdeg, getMaxPosTilt_i(), getMaxNegTilt_i(), myTiltSent)
    );
    preparePacket();
    myPacket.byteToBuf('T');
    myPacket.byteToBuf('P');
    myPacket.byte2ToBuf(ArMath::roundInt(tdeg/myTiltConvert));

    myTiltSent = tdeg;
    if(!myCanGetRealPanTilt) myTilt = myTiltSent;
    if (!sendPacket(&myPacket)) return false;
  }

  return true;
}

AREXPORT bool ArDPPTU::panSlew(double deg)
{
  if (deg > getMaxPanSlew())
    deg = getMaxPanSlew();
  if (deg < getMinPanSlew())
    deg = getMinPanSlew();
  
  myPanSlew = deg;
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::PAN);
  myPacket.byteToBuf(ArDPPTUCommands::SPEED);

  myPacket.byte2ToBuf(ArMath::roundInt(deg/myPanConvert));

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::tiltSlew(double deg)
{
  if (deg > getMaxTiltSlew())
    deg = getMaxTiltSlew();
  if (deg < getMinTiltSlew())
    deg = getMinTiltSlew();
  
  myTiltSlew = deg;
  preparePacket();
  myPacket.byteToBuf('T');// ArDPPTUCommands::TILT);
  myPacket.byteToBuf('S');// ArDPPTUCommands::SPEED);

  myPacket.byte2ToBuf(ArMath::roundInt(deg/myTiltConvert));

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::resetCalib(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::RESET);
  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::disableReset(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::RESET);
  myPacket.byteToBuf(ArDPPTUCommands::DISABLE);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::resetTilt(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::RESET);
  myPacket.byteToBuf(ArDPPTUCommands::TILT);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::resetPan(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::RESET);
  myPacket.byteToBuf(ArDPPTUCommands::PAN);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::resetAll(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::RESET);
  myPacket.byteToBuf(ArDPPTUCommands::ENABLE);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::saveSet(void)
{
  preparePacket();
  myPacket.byteToBuf('D'); //ArDPPTUCommands::DISABLE);
  myPacket.byteToBuf('S'); //ArDPPTUCommands::SPEED);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::restoreSet(void)
{
  preparePacket();
  myPacket.byteToBuf('D'); //ArDPPTUCommands::DISABLE);
  myPacket.byteToBuf('R'); //ArDPPTUCommands::RESET);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::factorySet(void)
{
  preparePacket();
  myPacket.byteToBuf('D'); //ArDPPTUCommands::DISABLE);
  myPacket.byteToBuf('F'); //ArDPPTUCommands::FACTORY);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::limitEnforce(bool val)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::LIMIT);

  if (val)
    myPacket.byteToBuf(ArDPPTUCommands::ENABLE);
  else
    myPacket.byteToBuf(ArDPPTUCommands::DISABLE);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::immedExec(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::IMMED);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::slaveExec(void)
{
  preparePacket();
  myPacket.byteToBuf('S'); //ArDPPTUCommands::SPEED);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::awaitExec(void)
{
  preparePacket();
  myPacket.byteToBuf('A'); //ArDPPTUCommands::ACCEL);

  return sendPacket(&myPacket);
}


AREXPORT bool ArDPPTU::haltAll(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::HALT);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::haltPan(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::HALT);
  myPacket.byteToBuf(ArDPPTUCommands::PAN);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::haltTilt(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::HALT);
  myPacket.byteToBuf(ArDPPTUCommands::TILT);

  return sendPacket(&myPacket);
}


AREXPORT bool ArDPPTU::panAccel(double deg)
{
  if (deg > getMaxPanAccel())
    deg = getMaxPanAccel();
  if (deg < getMinPanAccel())
    deg = getMinPanAccel();

  if (myPanAccel != deg) {
    preparePacket();
    myPacket.byteToBuf(ArDPPTUCommands::PAN);
    myPacket.byteToBuf(ArDPPTUCommands::ACCEL);
    myPacket.byte2ToBuf(ArMath::roundInt(deg/myPanConvert));

    return sendPacket(&myPacket);
  }

  return true;
}

AREXPORT bool ArDPPTU::tiltAccel(double deg)
{
  if (deg > getMaxPanAccel())
    deg = getMaxPanAccel();
  if (deg < getMinPanAccel())
    deg = getMinPanAccel();

  if (myTiltAccel != deg) {
    preparePacket();
    myPacket.byteToBuf(ArDPPTUCommands::TILT);
    myPacket.byteToBuf(ArDPPTUCommands::ACCEL);
    myPacket.byte2ToBuf(ArMath::roundInt(deg/myTiltConvert));

    return sendPacket(&myPacket);
  }

  return true;
}

AREXPORT bool ArDPPTU::basePanSlew(double deg)
{
  myBasePanSlew = deg;

  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::PAN);
  myPacket.byteToBuf(ArDPPTUCommands::BASE);
  myPacket.byte2ToBuf(ArMath::roundInt(deg/myPanConvert));

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::baseTiltSlew(double deg)
{
  myBaseTiltSlew = deg;

  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::TILT);
  myPacket.byteToBuf(ArDPPTUCommands::BASE);
  myPacket.byte2ToBuf(ArMath::roundInt(deg/myTiltConvert));

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::upperPanSlew(double deg)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::PAN);
  myPacket.byteToBuf(ArDPPTUCommands::UPPER);
  myPacket.byte2ToBuf(ArMath::roundInt(deg/myPanConvert));

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::lowerPanSlew(double deg)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::PAN);
  myPacket.byteToBuf('L'); //ArDPPTUCommands::LIMIT);
  myPacket.byte2ToBuf(ArMath::roundInt(deg/myPanConvert));

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::upperTiltSlew(double deg)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::TILT);
  myPacket.byteToBuf(ArDPPTUCommands::UPPER);
  myPacket.byte2ToBuf(ArMath::roundInt(deg/myTiltConvert));

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::lowerTiltSlew(double deg)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::TILT);
  myPacket.byteToBuf('L'); //ArDPPTUCommands::LIMIT);
  myPacket.byte2ToBuf(ArMath::roundInt(deg/myTiltConvert));

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::indepMove(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::CONTROL);
  myPacket.byteToBuf('I'); //ArDPPTUCommands::IMMED);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::velMove(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::CONTROL);
  myPacket.byteToBuf(ArDPPTUCommands::VELOCITY);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::enMon(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::MONITOR);
  myPacket.byteToBuf(ArDPPTUCommands::ENABLE);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::disMon(void)
{
  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::MONITOR);
  myPacket.byteToBuf(ArDPPTUCommands::DISABLE);

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::initMon(double deg1, double deg2, 
			       double deg3, double deg4)
{

  preparePacket();
  myPacket.byteToBuf(ArDPPTUCommands::MONITOR);

  myPacket.byte2ToBuf(ArMath::roundInt(deg1/myPanConvert));
  myPacket.byteToBuf(',');
  myPacket.byte2ToBuf(ArMath::roundInt(deg2/myPanConvert));
  myPacket.byteToBuf(',');
  myPacket.byte2ToBuf(ArMath::roundInt(deg3/myTiltConvert));
  myPacket.byteToBuf(',');
  myPacket.byte2ToBuf(ArMath::roundInt(deg4/myTiltConvert));

  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::setHoldPower(Axis axis, PowerMode mode)
{
  preparePacket();
  myPacket.byteToBuf(axis);
  myPacket.byteToBuf('H');
  myPacket.byteToBuf(mode);
  return sendPacket(&myPacket);
}

AREXPORT bool ArDPPTU::setMovePower(Axis axis, PowerMode mode)
{
  preparePacket();
  myPacket.byteToBuf(axis);
  myPacket.byteToBuf('M');
  myPacket.byteToBuf(mode);
  return sendPacket(&myPacket);
}


AREXPORT ArBasePacket *ArDPPTU::readPacket()
{
  if(!myConn)
    return NULL;

  // ask for next set of data every robot cycle
  query();

  const int databufsize = 64;
  char databuf[databufsize];
  int databufp = 0;

  // state indicates what part of the message we expect
  // to see next as we read and scan it.
  enum State {
    start,
    space,
    data,
    end
  } state;
  state = start;
  bool errormsg = false;
  bool gotpan = false;
  bool gottilt = false;

  const int maxbytes = 512;

  // bug, this doesn't handle messages broken accross our 512 byte read size.

  int i;
  for(i = 0;  i < maxbytes; ++i)
  {
    char c;
    if(myConn->read(&c, 1, 1) <= 0)
      break; // no data, abort read until next robot cycle

    //printf("ArDPPTU::read[%d]= %c (0x%x)\n", i, c, c);

    if(c == '*')
      state = start; // jump into start of new message no matter what the previous state was. (start state will see * again and proceed to state after that)

    switch (state)
    {
      case start:
        //puts("(RE)START PARSE");
        databuf[0] = 0;
        databufp = 0;
        if(c == '*')
        {
          state = space;
          errormsg = false;
        }
        else if(c == '!')
        {
          state = space;
          errormsg = true;
        }
        else // error, restart
        {
          state = start;
        }
        break;

      case space:
        if(c == ' ')
        {
          state = data;
        }
        else // error, restart
        {
          state = start;
        }
        break;

      case data:
        if(c == '\r')
        {
          //puts("IN DATA, REACHED END");
          databuf[databufp++] = '\0';
          state = end;
        }
        else if(!isprint(c))
        {
          // error, restart
          state = start;
        }
        else
        {
          databuf[databufp++] = c;
          if(databufp > databufsize)
          {
            ArLog::log(ArLog::Terse, "ArDPPTU: Internal parse error, data in incoming message is too large for buffer, aborting.");
            state = start;
          }
        }
        break;

      case end:
      default:
        // fall out of switch, end is a special flag state checked outside this switch
        // where action is taken based on what data was received from the dpptu,
        // then state is reset to start.
        break;
    }

    if(state == end)
    {

      // responses we handle are:
      const char *panPosResponse = "Current Pan position is ";
      const size_t panPosResponseLen = strlen(panPosResponse);
      const char *tiltPosResponse = "Current Tilt position is ";
      const size_t tiltPosResponseLen = strlen(tiltPosResponse);
      const char *resResponse = " seconds arc per position";

      //printf("IN PARSE END, DATABUF=%s   \n", databuf);
      if(strncmp(databuf, panPosResponse, panPosResponseLen) == 0)
      {
        char *s = databuf + panPosResponseLen;
        DEBUG_POS(printf("\npan position: %s\n", s));
        myPan = myPanRecd = atof(s) * myPanConvert;
        myCanGetRealPanTilt = true;
        gotpan = true;
      }
      else if(strncmp(databuf, tiltPosResponse, tiltPosResponseLen) == 0)
      {
        char *s = databuf + tiltPosResponseLen;
        DEBUG_POS(printf("\ntilt position: %s\n", s));
        myTilt = myTiltRecd = atof(s) * myTiltConvert;
        myCanGetRealPanTilt = true;
        gottilt = true;
      }
// XXX TODO get pan/tilt and slew limits as well
// conversion factors to ( response / (60.0*60.0) ) (ticks per arcsecond...   converting to degrees)
      else if(char *s = strstr(databuf, resResponse))
      {
         //printf("it's a resolution response. (already got pan? %d; already got tilt?  %d)", myGotPanRes, myGotTiltRes);
        *s = '\0';
        const float res = atof(databuf);
        if(!myGotPanRes)
        {
          myPanConvert = res / (60.0*60.0);  // convert from arcsecond to degr
          ArLog::log(ArLog::Normal, "ArDPPTU: Received pan resolution response from PTU: %f deg / %f arcsec. Now requesting tilt resolution...", myPanConvert, res);

          // Now ask for tilt resolution (there is no way to distinguish the
          // responses from the PTU so we have to ask for them in sequence.)
          myGotPanRes = true;
          preparePacket();
          myPacket.byteToBuf('T');
          myPacket.byteToBuf('R');
          sendPacket(&myPacket);
        }
        else if(!myGotTiltRes)
        {
          myTiltConvert = res / (60.0*60.0);
          ArLog::log(ArLog::Normal, "ArDPPTU: Received tilt resolution response from PTU: %f deg / %f arcsec", myTiltConvert, res);
          myGotTiltRes = true;
        }
        else
        {
          ArLog::log(ArLog::Normal, "ArDPPTU: Warning: got unexpected resolution response (already received both pan and tilt resolution responses). Ignoring.");
        }
      }
      else
      {
        // unrecognized message.
        ArLog::log(ArLog::Normal, "ArDPPTU: Warning: received unrecognized message from PTU: %s", databuf);
      }

      state = start; // start looking for next message


    }


  }

  if(gotpan && gottilt)
  {
      // a message was recognized and handled above.
      DEBUG_TIME(ArLog::log(ArLog::Normal, "ArDPPTU recieve interval=%ld", myLastPositionMessageHandled.mSecSince()));
      myLastPositionMessageHandled.setToNow();
      myWarnedOldPositionData = false;

      // ask for next position . this times our queries to the actual     //
      // recieve rate.
      // moved to above to always query every loop query();
  }

  if(myLastPositionMessageHandled.mSecSince() > 2000 && !myWarnedOldPositionData)
  {
    ArLog::log(ArLog::Terse, "ArDPPTU: Warning: Have not received pan and tilt position for more than 2 seconds! Data will be incorrect.");
    myWarnedOldPositionData = true;
  }

  if(i >= maxbytes)
  {
    ArLog::log(ArLog::Normal, "ArDPPTU: Warning: parse or communications error: no valid message found in %d bytes read from PTU.", maxbytes);
  }

  // next time.
  return NULL;
}


void ArDPPTU::query()
{
  DEBUG_TIME(ArLog::log(ArLog::Normal, "ArDPPTU query interval=%ld", myLastQuerySent.mSecSince()));
  // ask for pan and tilt positions.
  if(myConn)
  {
    if(myConn->write("PP\rTP\r", 2*2+2) <= 0) return;
    myLastQuerySent.setToNow();
  }
}


ArPTZConnector::GlobalPTZCreateFunc ArDPPTU::ourCreateFunc(&ArDPPTU::create);

ArPTZ* ArDPPTU::create(size_t index, ArPTZParams params, ArArgumentParser *parser, ArRobot *robot)
{
  return new ArDPPTU(robot);
}

void ArDPPTU::registerPTZType()
{
  ArPTZConnector::registerPTZType("dpptu", &ourCreateFunc);
}
