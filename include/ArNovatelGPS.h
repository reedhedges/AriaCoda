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

#ifndef ARNOVATELGPS_H
#define ARNOVATELGPS_H


#include "ariaTypedefs.h"
#include "ArFunctor.h"
#include "ariaUtil.h"
#include "ArGPS.h"

/** @brief GPS subclass to support the NovAtel ProPak G2 and other NovAtel
 * "OEM4" devices.
 *  
 *  This subclass overrides initDevice() to send initialization commands specific
 *  to NovAtel GPS devices.
 *  On connect, it initiazes SBAS (satellite-based augmentation) to
 *  "auto", which uses WAAS, EGNOS, or MSAS if available,
 *  and then requests NMEA messages to be interpreted by ArGPS.
 *
 *  It does not initialize corrections sent by your own DGPS fixed
 *  base station over radio, or the wide-area L-Band services such as OmniStar or 
 *  the Canada-wide DGPS (supported by the NovaTel ProPak LBplus). 
 *  To configure DGPS, you must connect to the GPS with a terminal
 *  program (e.g. "minicom" in Linux), configure the device for
 *  your specific service, region and radio settings, and save 
 *  with the "saveconfig" command. See the GPS setup notes 
 *  at http://robots.mobilerobots.com/tech_notes/GPS_Setup_Notes.txt
 *  and the NovAtel GPS Reference Manual Volume 1, Sections 4.3 and 4.5 for
 *  ("Transmitting and Receiving Corrections" and "Enabling L-Band Positioning")
 *  for more information on doing this.
 *
 */
class ArNovatelGPS : public virtual ArGPS {
protected:
  void handleNovatelGPGGA(ArNMEAParser::Message msg);
  ArFunctor1C<ArNovatelGPS, ArNMEAParser::Message> myNovatelGPGGAHandler;
public:
  AREXPORT ArNovatelGPS();
  AREXPORT virtual ~ArNovatelGPS();
protected:
  AREXPORT virtual bool initDevice();
};

/** @brief GPS subclass to support the NovAtel SPAN GPS with integrated IMU. 

    This subclass requests the INGLL NMEA message for combined IMU and GPS
    position, and updates the GPS position in ArGPS using this data.
    It replaces the normal GPRMC handler from ArGPS, and instead saves the uncorrected
    GPS data separately. (use dynamic_cast to cast an ArGPS object to an
    ArNovatelSPAN object to access this data).

    @since ARIA 2.7.2
*/
class ArNovatelSPAN : public virtual ArNovatelGPS {
protected:
  /** overrides ArNovatelGPS::handleGPRMC(), and keeps results of parsing the
  * message in this class rather than ArGPS. */
  void handleGPRMC(ArNMEAParser::Message msg);
  void handleINGLL(ArNMEAParser::Message msg);
  AREXPORT virtual bool initDevice();
  ArFunctor1C<ArNovatelSPAN, ArNMEAParser::Message> myGPRMCHandler;
  ArFunctor1C<ArNovatelSPAN, ArNMEAParser::Message> myINGLLHandler;
public:
  double GPSLatitude, GPSLongitude;
  bool haveGPSPosition, GPSValidFlag;
  ArTime timeGotGPSPosition, GPSTimestamp;
  AREXPORT ArNovatelSPAN();
  AREXPORT virtual ~ArNovatelSPAN();
};

#endif 


