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
#include "ArNovatelGPS.h"
#include "ArDeviceConnection.h"


AREXPORT ArNovatelGPS::ArNovatelGPS() :
  ArGPS(),
  myNovatelGPGGAHandler(this, &ArNovatelGPS::handleNovatelGPGGA)
{
  // override normal GPGGA handler:
  addNMEAHandler("GGA", &myNovatelGPGGAHandler);
}

AREXPORT bool ArNovatelGPS::initDevice()
{
  if (!ArGPS::initDevice()) return false;

  char cmd[32];
  memset(cmd, 0, 32);

  myDevice->write("\r\n", 2); // prod the connection and end any previous commands it was waiting for, errors OK 

  // Enable WAAS/EGNOS/MSAS/etc satellite-based correction:
  const char* const sbasCmd = "sbascontrol enable auto 0 zerototwo\r\n";
  if (myDevice->write(sbasCmd, strlen(sbasCmd)) < (int) strlen(sbasCmd))
    return false;

  // Send a command to start sending data for each message type in the ArGPS
  // handlers map:
  const ArNMEAParser::HandlerMap& handlers = myNMEAParser.getHandlersRef();
  for(ArNMEAParser::HandlerMap::const_iterator i = handlers.begin(); i != handlers.end(); ++i)
  {
    float interval = 1;
    if( (*i).first == "RMC") interval = 0.25;  //special case, make this come faster
    ArLog::log(ArLog::Verbose, "ArNovatelGPS: Requesting GP%s at %g sec interval", (*i).first.c_str(), interval);
    snprintf(cmd, 32, "log thisport GP%s ontime %g\r\n", (*i).first.c_str(), interval);
        // XXX TODO don't just insert "GP" prefix here, store the expected
        // prefix, or, the expected prefixes for different GPS receiver types,
        // in the message/handler list?
    //ArLog::log(ArLog::Verbose, "ArNovatelGPS: sending command: %s", cmd);
    if (myDevice->write(cmd, strlen(cmd)) != (int) strlen(cmd)) return false;
  }

  return true;
}


AREXPORT ArNovatelGPS::~ArNovatelGPS() {
  if(!myDevice) return;
  myDevice->write("unlogall\r\n", strlen("unlogall\r\n")); // don't worry about errors
}


void ArNovatelGPS::handleNovatelGPGGA(ArNMEAParser::Message msg)
{
  // call base handler
  ArGPS::handleGPGGA(msg);

  // Some of Novatel's values are different from the standard:
  // (see
  // http://na1.salesforce.com/_ui/selfservice/pkb/PublicKnowledgeSolution/d?orgId=00D300000000T86&id=501300000008RAN&retURL=%2Fsol%2Fpublic%2Fsolutionbrowser.jsp%3Fsearch%3DGPGGA%26cid%3D000000000000000%26orgId%3D00D300000000T86%26t%3D4&ps=1 or search Novatel's Knowlege Base for "GPGGA")
 
  ArNMEAParser::MessageVector *message = msg.message;
  if(message->size() < 7) return;
  switch((*message)[6].c_str()[0])
  {
    case '2':
      myData.fixType = OmnistarConverging;
      break;
    case '5':
      myData.fixType = OmnistarConverged;
      break;
    case '9':
      myData.fixType = DGPSFix;
      break;
  }
}

AREXPORT ArNovatelSPAN::ArNovatelSPAN() : 
  ArNovatelGPS(), 
  myGPRMCHandler(this, &ArNovatelSPAN::handleGPRMC),
  myINGLLHandler(this, &ArNovatelSPAN::handleINGLL),
  GPSLatitude(0), GPSLongitude(0), haveGPSPosition(false), GPSValidFlag(false) 
{
  replaceNMEAHandler("GPRMC", &myGPRMCHandler);

  // NOTE if the SPAN provides an "INRMC" that has the same format as GPRMC,
  // then this class could be simplified by supplying ArGPS::myGPRMCHandler as
  // the handler for INRMC, instead of implementing a new INGLL handler here.
  addNMEAHandler("INGLL", &myINGLLHandler);
  addNMEAHandler("GLL", &myINGLLHandler);
}

AREXPORT ArNovatelSPAN::~ArNovatelSPAN()
{
}

void ArNovatelSPAN::handleGPRMC(ArNMEAParser::Message msg)
{
  parseGPRMC(msg, GPSLatitude, GPSLongitude, GPSValidFlag, haveGPSPosition, timeGotGPSPosition, GPSTimestamp, myData.haveSpeed, myData.speed);
}

void ArNovatelSPAN::handleINGLL(ArNMEAParser::Message msg)
{
  const ArNMEAParser::MessageVector *mv = msg.message;

  if(mv->size() < 5) return;
  double lat, lon;
  if(!readFloatFromStringVec(mv, 1, &lat, &gpsDegminToDegrees)) return;
  if((*mv)[2] == "S") lat *= -1;
  else if((*mv)[2] != "N") return;
  if(!readFloatFromStringVec(mv, 3, &lon, &gpsDegminToDegrees)) return;
  if((*mv)[4] == "W") lon *= -1;
  else if((*mv)[4] != "E") return;
  myData.latitude = lat;
  myData.longitude = lon;
  myData.havePosition = true;

  if(mv->size() < 6) return;
  if(!readTimeFromString((*mv)[5], &(myData.GPSPositionTimestamp))) return;

  if(mv->size() < 7) return;
  myData.qualityFlag = ((*mv)[6] != "V" && (*mv)[6] != "N");
}

AREXPORT bool ArNovatelSPAN::initDevice()
{
  if(!ArNovatelGPS::initDevice()) return false;
  ArLog::log(ArLog::Normal, "ArNovatelSPAN: will request INS-corrected latitude and longitude to use as GPS position.");

  // Actually request a faster rate for INGLL than ArNovatelGPS::initDevice() did:
  const char *cmd = "log thisport INGLL ontime 0.25\r\n";
  myDevice->write(cmd, strlen(cmd));

  return true;
}
