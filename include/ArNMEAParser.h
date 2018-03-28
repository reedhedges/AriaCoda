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

#ifndef ARNMEAPARSER_H
#define ARNMEAPARSER_H

#include "ariaTypedefs.h"
#include "ArFunctor.h"
#include "ariaUtil.h"
#include "ArDeviceConnection.h"
#include <string>
#include <vector>


/** @brief NMEA Parser
 *
 *  Parses NMEA input data and calls callbacks for certain messages with message
 *  parts.   NMEA is a standard output data protocol used by GPS devices and
 *  others (e.g. compass, altimiter, etc.)   This class is used internally by ArNMEAParser and
 *  subclasses, and by ArTCMCompassDirect.
 */
class ArNMEAParser {

public:
    /** @param name Used in log messages */
    AREXPORT ArNMEAParser(const char *name = "NMEA Parser");

    /** @brief Flags to indicates what the parse() method did. 
     *  i.e. If nothing was done, then the
     *  result will be 0. To check a parse() return result @a result to see if data was updated, use
     *  (result & ParseUpdated). To check if there was an error, use (result &
     *  ParseError). 
     */
    enum {
      ParseFinished = 1,  ///< There was no data to parse
      ParseError = 2,     ///< There was an error
      ParseData = 4,      ///< Input was recieved and stored, but no complete messages were parsed
      ParseUpdated = 8    ///< At least one complete message was parsed
    } ParseFlags;

    /** @brief Set whether checksum is ignored (default behavior is not to ignore it, and
     * skip messages with incorrect checksums, and log a warning mesage) */
    AREXPORT void setIgnoreChecksum(bool ignore) { ignoreChecksum = ignore; }

    /** NMEA message, divided into parts.  */
    typedef std::vector<std::string> MessageVector;

    /** Message data passed to handlers */
    typedef struct {
      /** The parts of the message, including initial message ID (but excluding
       * checksum) */
      ArNMEAParser::MessageVector* message;
      /** Timestamp when the beginning of this message was recieved and parsing
       * began. */
      ArTime timeParseStarted;
      /// Message ID (first word minus talker prefix)
      std::string id;
      /// Talker-ID prefix, may indicate type of receiver or what kind of GPS system is in use
      std::string prefix;
    } Message;
      

    /** NMEA message handler type.  */
    typedef ArFunctor1<ArNMEAParser::Message> Handler;


    /** Set a handler for an NMEA message. Mostly for internal use or to be used
     * by related classes, but you could use for ususual or custom messages
     * emitted by a device that you wish to be handled outside of the ArNMEAParser
     * class. 
     * @param messageID ID of NMEA sentence/message, without two-letter "talker" prefix.
     * @param handler Handler object
     */
    AREXPORT void addHandler(const char *messageID, ArNMEAParser::Handler *handler);
    AREXPORT void removeHandler(const char *messageID);

    /* Read a chunk of input text from the given device connection and 
     * parse with parse(char*, int).  The maximum amount of text read from the device
     * connection is determined by the internal buffer size in this class
     * (probably a few hundred bytes limit).
     * @return a result code from ParseFlags
     * @note You should only use one stream of data with ArNMEAParser, and in a
     * continuous fashion, since it will store partially recieved messages for
     * the next call to one of the parse() methods.
     */
    AREXPORT int parse(ArDeviceConnection *dev);

    /* Parse a chunk of input text. Call message handlers as complete NMEA
     * messages are parsed.  Parsing state is stored in this ArNMEAParser object.
     * @return a result code from ParseFlags
     */
    AREXPORT int parse(const char *buf, int n);

    

     
    
public:
    /** Map of message identifiers (without "GP" prefix) to handler functors 
        @sa getHandlersRef()
     */
    typedef std::map<std::string, ArNMEAParser::Handler*> HandlerMap;

private:
    /* NMEA message handlers used by ArNMEAParser */
    HandlerMap myHandlers;

    std::map<std::string, std::string> myLastPrefix;

public:
    /** Return a const reference to the set of message handlers.
        See HandlerMap type definition.
    */
    const ArNMEAParser::HandlerMap& getHandlersRef() const { return myHandlers; }

private:

    const char *myName;

    /*  NMEA scanner state.
     *  There are possabilities for opmitization here, such 
     *  as just storing the read data in a buffer and handling
     *  each field as it is found in the buffer, or building
     *  a list of char* for each field pointing into the buffer
     *  instead of copying each field into a std::string in the
     *  currentMessage vector, etc. etc.
     */
    const unsigned short MaxNumFields;
    const unsigned short MaxFieldSize; // bytes
    bool ignoreChecksum;

    MessageVector currentMessage;
    ArTime currentMessageStarted;
    std::string currentField;
    char checksumBuf[3];
    short checksumBufOffset;
    bool inChecksum;
    bool inMessage;
    char currentChecksum;
    bool gotCR;

    // Tools to update state
    void beginMessage();
    void endMessage();
    void nextField();
    void beginChecksum();

    /* Data buffer used by handleInput(ArDeviceConnection*).
     * This should be enough to hold several NMEA messages. 
     * Most NMEA messages should be less than 50 bytes or so; 
     * 256 then allows a minumum of 5 messages parsed per 
     * call to parse(arDeviceConnection*).)
     */
    char myReadBuffer[256]; //[512];

};

#endif // ifdef ARNMEAPARSER_H

