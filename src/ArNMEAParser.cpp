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
#include "ArNMEAParser.h"

#include <iostream>


//#define DEBUG_ARNMEAPARSER 1

#ifdef DEBUG_ARNMEAPARSER
void ArNMEAParser_printBuf(FILE *fp, const char *data, int size) {  
  for(int i = 0; i < size; ++i) {
    if(data[i] == '\n')
      fprintf(fp, "[LF]");
    else if(data[i] == '\r')
      fprintf(fp, "[CR]");
    else if(data[i] < ' ' || data[i] > '~')  {
      fprintf(fp, "[0x%X]", data[i] & 0xff);    
    }
    else
      fputc(data[i], fp);    
  }
}
#endif

AREXPORT ArNMEAParser::ArNMEAParser(const char *name) :
  myName(name),
  MaxNumFields(50),
  MaxFieldSize(128),
  ignoreChecksum(false),
  checksumBufOffset(0),
  inChecksum(false),
  inMessage(false),
  currentChecksum(0),
  gotCR(false)
{
  memset(checksumBuf, 0, 3);
}

AREXPORT void ArNMEAParser::addHandler(const char *message, ArNMEAParser::Handler *handler)
{
  myHandlers[message] = handler;
}

AREXPORT void ArNMEAParser::removeHandler(const char *message)
{
  HandlerMap::iterator i = myHandlers.find(message);
  if(i != myHandlers.end()) myHandlers.erase(i);
}


void ArNMEAParser::nextField()
{
  currentMessage.push_back(currentField);
  currentField = "";
  if (currentMessage.size() > MaxNumFields)
    endMessage();
}

void ArNMEAParser::endMessage()
{
  inMessage = false;
  inChecksum = false;
  currentField = "";
  gotCR = false;
  currentMessage.clear();
}

void ArNMEAParser::beginChecksum()
{
  checksumBufOffset = 0;
  inChecksum = true;
}

void ArNMEAParser::beginMessage()
{
  currentMessageStarted.setToNow();
  currentMessage.clear();
  inChecksum = false;
  inMessage = true;
  currentField = "";
  gotCR = false;
  currentChecksum = 0;
  memset(checksumBuf, 0, sizeof(checksumBuf));
}


AREXPORT int ArNMEAParser::parse(ArDeviceConnection *dev) 
{
  int n = dev->read(myReadBuffer, sizeof(myReadBuffer));
#ifdef DEBUG_ARNMEAPARSER
  std::cerr << "\t[ArNMEAParser: read " << n << " bytes of data from device connection:\n\t";
  ArNMEAParser_printBuf(stderr, myReadBuffer, n);
#endif
  if(n < 0) return ParseError;
  if(n == 0) return ParseFinished;
  return parse(myReadBuffer, n);
}

AREXPORT int ArNMEAParser::parse(const char *buf, int n)
{
  int result = 0;
  if (n < 0) 
  {
    return result|ParseError;
  }

  if (n == 0) 
  {
    return result|ParseFinished;
  }

#ifdef DEBUG_ARNMEAPARSER
  std::cerr << "\t[ArNMEAParser: given " << n << " bytes of data.]\n";
  std::cerr << "\t[ArNMEAParser: parsing chunk \"";
  ArNMEAParser_printBuf(stderr, buf, n);
  std::cerr << "\"]\n";
#endif


  for (int i = 0; i < n; i++)
  {
    // Check for message start
    if (buf[i] == '$')
    {
      beginMessage();
      continue;
    }

    // Otherwise, we must be in a sentece to do anything
    if (!inMessage)
      continue;

    // Reached the CR at the end?
    if (buf[i] == '\r') 
    {
      gotCR = true;
      continue;
    }
  
    // Reached the Newline at the end?
    if (buf[i] == '\n') 
    {
      if (gotCR) 
      {
        // Got both CR and LF?-- then end of message

        if(!inChecksum)
        {
          // checksum should have preceded.
          ArLog::log(ArLog::Terse, "ArNMEAParser: Missing checksum.");
          result |= ParseError;
          endMessage();
          continue;
        }

        // got CRLF but there was no data. Ignore.
        if(currentMessage.size() == 0)
        {
          endMessage();
          continue;
        }
        

        // ok:
        Message msg;
        msg.message = &currentMessage;
        msg.timeParseStarted = currentMessageStarted;
        msg.prefix = currentMessage[0].substr(0, 2);
        // TODO should we check for an accepted set of prefixes? (e.g. GP, GN,
        // GL, GB, BD, HC, PG, etc.)
        msg.id = currentMessage[0].substr(2);
#ifdef DEBUG_ARNMEAPARSER
        fprintf(stderr, "\t[ArNMEAPArser: Input message has system prefix %s with message ID %s]\n", msg.prefix.c_str(), msg.id.c_str());
#endif
        std::string lastprefix = myLastPrefix[msg.id];
        if(lastprefix != "" && lastprefix != msg.prefix)
        {
          const char *id = msg.id.c_str();
          const char *p = msg.prefix.c_str();
          const char *lp = lastprefix.c_str();
          ArLog::log(ArLog::Normal, "ArNMEAParser: Warning: Got duplicate %s message with prefix %s (previous prefix was %s).  Data from %s%s will replace %s%s.", id, p, lp, p, id, lp, id);
        }
        HandlerMap::iterator h = myHandlers.find(msg.id);
        if (h != myHandlers.end()) 
        {
#ifdef DEBUG_ARNMEAPARSER
          fprintf(stderr, "\t[ArNMEAParser: Got complete message, calling handler for %s...]\n", msg.id.c_str());
#endif
          if(h->second)
          {
            h->second->invoke(msg);
            result |= ParseUpdated;
          }
          else
          {
            ArLog::log(ArLog::Terse, "ArNMEAParser Internal Error: NULL handler functor for message %s!\n", msg.id.c_str());
          }
        }
#ifdef DEBUG_ARNMEAPARSER
        else
        {
          fprintf(stderr, "\t[ArNMEAParser: Have no message handler for %s (%s).]\n", msg.id.c_str(), currentMessage[0].c_str());
        }
#endif
      }
      else
      {
        ArLog::log(ArLog::Normal, "ArNMEAParser: syntax error, \\n without \\r.");
        result |= ParseError;
      }

      endMessage();
      continue;
    }

    // Are we in the final checksum field?
    if (inChecksum)
    {
      checksumBuf[checksumBufOffset++] = buf[i];
      if (checksumBufOffset > 1)   // two bytes of checksum
      {
        int checksumRec = (int) strtol(checksumBuf, NULL, 16);
        if (checksumRec != currentChecksum) 
        {
          ArLog::log(ArLog::Normal, "%s: Warning: Skipping message with incorrect checksum.", myName);

          // reconstruct message to log:
          std::string nmeaText = "";
          for(MessageVector::const_iterator i = currentMessage.begin(); i != currentMessage.end(); ++i)
          {
            if(i != currentMessage.begin()) nmeaText += ",";
            nmeaText += *i;
          }
          ArLog::log(ArLog::Normal, "%s: Message provided checksum \"%s\" = 0x%x (%d). Calculated checksum is 0x%x (%d).  NMEA message contents were: \"%s\"", myName, checksumBuf, checksumRec, checksumRec, currentChecksum, currentChecksum, nmeaText.c_str());

          // abort the message and start looking for the next one.
          result |= ParseError;
          endMessage();
        }
      }
      continue;
    }


    // Got to the checksum?
    if (buf[i] == '*')
    {
      nextField();
      if (!ignoreChecksum)
        beginChecksum();
      continue;
    }

    // Every byte in a message between $ and * XORs to form the
    // checksum:
    currentChecksum ^= buf[i];

    // Time to start a new field?
    if (buf[i] == ',')
    {
      nextField();
      continue;
    }


    // Else, we must be in the middle of a field
    // TODO we could use strchr to look ahead in the buf 
    // for the end of the field (',' or '*') or end of the buf, and copy more
    // than one byte at a time.
    currentField += buf[i];
    if (currentField.size() > MaxFieldSize)
    {
      endMessage();
      continue;
    }
  }

  return result;
}



