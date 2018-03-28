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

#ifndef ARTRIMBLEGPS_H
#define ARTRIMBLEGPS_H


#include "ariaTypedefs.h"
#include "ArFunctor.h"
#include "ariaUtil.h"
#include "ArGPS.h"
#include "ArDeviceConnection.h"
#include "ArMutex.h"

#include <deque>

class ArTrimbleAuxDeviceConnection;

/** @brief GPS subclass to support the Trimble AgGPS and other Trimble GPS devices.
 *  
 *  This subclass extends ArGPS to send initialization commands specific
 *  to Trimble GPS devices, and to handle the PTNLAG001 message which
 *  is specific to the Trimble GPS (this message contains data received 
 *  from an auxilliary device connected to the GPS; ArTrimbleGPS
 *  simply parses its contents as a new NMEA message; i.e. data received
 *  by the Trimble is assumed to be NMEA messages that it forwards
 *  via the PTNLAG001 message.)
 *
 *  @note You must also configure the ports using
 *  the Trimble AgRemote program
 *  (http://www.trimble.com/support_trl.asp?pt=AgRemote&Nav=Collection-1545).  
 *  Enable the following messages on whichever
 *  GPS port the computer is connected to: GPRMC, GPGGA, GPGSA, GPGSV, GPGST,
 *  GPMSS, and set input (I) protocol to TSIP 38k baud, and output
 *  protocol (O) to NMEA 38k baud. 
 *  This configuration is done by MobileRobots when shipping a Trimble AgGPS
 *  but you may need to do this if the GPS loses its configuration or after
 *  changing any other settings (Note that AgRemote resets the port settings each time
 *  it connects, so you must reset them each time before exiting AgRemote!)
 *
 *  @since 2.6.0
 */
class ArTrimbleGPS : public virtual ArGPS {
private:
  ArFunctor1C<ArTrimbleGPS, ArNMEAParser::Message> myAuxDataHandler;
  void handlePTNLAG001(ArNMEAParser::Message message);
public:
  AREXPORT ArTrimbleGPS();
  AREXPORT virtual ~ArTrimbleGPS();

  /** Send a TSIP command to the Trimble GPS.
   *  See the TSIP Reference guide for details.
   *  Note, the data must be 66 characters or less.
   */
  AREXPORT bool sendTSIPCommand(char command, const char *data, size_t dataLen);

protected:
  AREXPORT virtual bool initDevice();

};


#endif 


