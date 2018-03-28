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
#include "ArTrimbleGPS.h"
#include "ArDeviceConnection.h"

//#define DEBUG_ARTRIMBLEGPS 1

#ifdef DEBUG_ARTRIMBLEGPS
void ArTrimbleGPS_printTransUnprintable( const char *data, int size){  for(int i = 0; i < size; ++i)  {    if(data[i] < ' ' || data[i] > '~')    {      printf("[0x%X]", data[i] & 0xff);    }    else    {      putchar(data[i]);    }  }}
#endif

AREXPORT ArTrimbleGPS::ArTrimbleGPS() :
  myAuxDataHandler(this, &ArTrimbleGPS::handlePTNLAG001)
{
  myMutex.setLogName("ArTrimbleGPS::myMutex");
  addNMEAHandler("NLAG001", &myAuxDataHandler);
}

AREXPORT ArTrimbleGPS::~ArTrimbleGPS() {
}


AREXPORT bool ArTrimbleGPS::initDevice()
{
  if (!ArGPS::initDevice()) return false;

  const int maxpktlen = 10;
  char cmd[maxpktlen];


    /* These commands may not do anything, if the firmware configuration (set
     * using AgRemote) takes precedence:
     */

  // Enable automatic switching between DGPS and non-DGPS:
  memset(cmd, 0, maxpktlen);
  const int dgpspktlen = 5;
  cmd[0] = 0x10;
  cmd[1] = 0x62;
  cmd[2] = 3;   // mode 3 (automatic)
  //cmd[2] = 1;   // mode 1 (dgps only)
  cmd[3] = 0x10;  // packet terminators
  cmd[4] = 0x03;
  if(myDevice->write(cmd, dgpspktlen) < dgpspktlen)
    return false;

  // Enable WAAS? 


  // Enable NMEA output for some messages that we know the ArGPS class
  // uses. See TSIP manual (Rev C), table 2-106 on pg 2-62. The special message PTNLAG001
  // is always sent if data from a second device is recieved and the port
  // protocols/formats are set up correctly. These commands may not actually
  // have any effect if the messages are selected in the configuration using
  // AgRemote anyway.
  const ArTypes::Byte4 maskGGA = 0x00000001;
  const ArTypes::Byte4 maskGSV = 0x00000008;
  const ArTypes::Byte4 maskGSA = 0x00000010;
  const ArTypes::Byte4 maskRMC = 0x00000080;
  const ArTypes::Byte4 maskGST = 0x00000400;
  const ArTypes::Byte4 maskMSS = 0x00001000;
  const ArTypes::Byte4 mask = maskGGA|maskGSA|maskRMC|maskGSV|maskMSS|maskGST;
  const int nmeapktlen = 10;
  cmd[0] = (char)0x10;  // command header
  cmd[1] = (char)0x7A;  // general NMEA command id
  cmd[2] = (char)0x80;  // subcommand to set NMEA outputs
  //cmd[3] = 1;     // 1-second interval
  cmd[3] = 0;     // output at position fixrate (i.e. as fast as GPS can calculate them)
  cmd[4] = (char) (mask << 24);  // MSB, if on little-endian architecture (BUG if ported!)
  cmd[5] = (char) (mask << 16);
  cmd[6] = (char) (mask << 8);
  cmd[7] = (char) mask;
  cmd[8] = (char)0x10;  // packet terminators
  cmd[9] = (char)0x03;
  // (note, byte 0x10 is special, and if any of the bytes in mask was 0x10,
  // it would have to be preceded by an extra 0x10 byte. see TSIP reference
  // manual section 1.4.1). with the currently set value, none of the bytes of mask
  // will have value 0x10.

#ifdef DEBUG_ARTRIMBLEGPS
  printf("XXX ArTrimbleGPS sending command: hdr=0x%x, id=0x%x, sid=0x%x, int=%d, mask1=0x%x, mask2=0x%x, mask3=0x%x, mask=0x%x [<-those four bytes should match mask 0x%x, but with MSB first], tail1=0x%x, tail2=0x%x.\n", 
      (int)cmd[0] & 0xFF, (int)cmd[1] & 0xFF, (int)cmd[2] & 0xFF, 
      (int)cmd[3] & 0xFF, (int)cmd[4] & 0xFF, (int)cmd[5] & 0xFF, 
      (int)cmd[6] & 0xFF, (int)cmd[7] & 0xFF, mask,
      (int)cmd[8] & 0xFF, (int)cmd[9] & 0xFF);
#endif

  if(myDevice->write(cmd, nmeapktlen) < nmeapktlen)
  {
      ArLog::log(ArLog::Normal, "ArTrimbleGPS: Warning: error sending some configuration commands to GPS receiver (serial port write error?)");
      return false;
  }

  return true;
}



/* Instead of just merging input from a secondary device, which is
 * part of the design of NMEA and which would make life easy, Trimble 
 * wraps messages in a proprietary message  PTNLAG001. This handler
 * just re-serializes the contents of that message and bounces
 * it back to ArGPS for further parsing. However we don't distinguish 
 * the same messages from different NMEA "talkers" yet. 
 */
void ArTrimbleGPS::handlePTNLAG001(ArNMEAParser::Message m)
{
  ArNMEAParser::MessageVector *message = m.message;
  if(message->size() < 2) return;
  std::string text;
  size_t len = 0;
  // Undo split by commas, skip msg[0] which is "PTNLAG001"
  for(ArNMEAParser::MessageVector::const_iterator i = message->begin() + 1; i != message->end(); ++i)
  {
    if(i != message->begin()) text += ",";
    text += *i;
    len += (*i).length() + 1;
  }
#ifdef DEBUG_ARTRIMBLEGPS
  printf("XXXXXXXXX Got PTNLAG001 contents from Trimble, %d bytes: %s\n", len, text.c_str());
#endif

  // Reparse contents. Note, this will clobber NMEA parser state, including
  // *message, so after  calling this we can't do anything else with it. (So
  // exit the function)
  text += "\r\n";
  len += 2;
  myNMEAParser.parse(text.c_str(), len);
}


AREXPORT bool ArTrimbleGPS::sendTSIPCommand(char cmd, const char *data, size_t size)
{
#ifdef DEBUG_ARTRIMBLEGPS
  ArLog::log(ArLog::Normal, "ArTrimbleGPS sending command 0x%X to GPS, with data:", cmd & 0xFF);
  printf("\t");
  ArTrimbleGPS_printTransUnprintable(data, size);
  puts("");
#endif

    // TODO check these writes return values
    //
  char hdr[2];
  hdr[0] = 0x10;
  hdr[1] = cmd;
  getDeviceConnection()->write(hdr, 2);
  // Since the 0x10 byte is TSIP signals the end of the command, need to
  // "escape" it by doubling it.  So write data in segments, with a double 0x10
  // between each.
  const char *nextSegment = strchr(data, '\x10');
  const char *dataStart = data;
  while(nextSegment)
  {
    getDeviceConnection()->write(data, nextSegment - data);
    getDeviceConnection()->write("\x10\x10", 2);
    data = nextSegment + 1;
    nextSegment = strchr(data, '\x10');
  }
  // Write the last segment, if it exists
  getDeviceConnection()->write(data, dataStart + size - data);
  getDeviceConnection()->write("\x10\x03", 2); // command suffix

  return true;
}

