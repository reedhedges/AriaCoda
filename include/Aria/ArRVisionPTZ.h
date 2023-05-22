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


*/
#ifndef ARRVISIONPTZ_H
#define ARRVISIONPTZ_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ArBasePacket.h"
#include "Aria/ArPTZ.h"

/// A class for for making commands to send to the RVision camera
/** There are only two functioning ways to put things into this packet,
 * uByteToBuf() and byte2ToBuf;  You
 *  MUST use thse, if you use anything else your commands won't work.  
 *  @since 2.7.0
*/
class ArRVisionPacket final : public virtual ArBasePacket
{
public:
  /// Constructor
  AREXPORT ArRVisionPacket(uint16_t bufferSize = 15);
  
  AREXPORT virtual void uByteToBuf(uint8_t val) override;
  AREXPORT virtual void byte2ToBuf(int16_t val) override;
  // todo disable others
  
  /// This is a new function, read the details before you try to use it
  AREXPORT void byte2ToBufAtPos(int16_t val, uint16_t pos);
};

//class ArRobot;

/** Control the RVision camera pan tilt zoom unit.
   In addition to creating an ArRvisionPTZ instance, you will also need
   to create an ArSerialConnection object and open the serial port connection
   (the RVision is normally on COM3 on Seekur and Seekur Jr. robots) and
   use the setDeviceConnection() method to associate the serial connection
   with the ArRVisionPTZ object.
	@since 2.7.0
*/

class ArRVisionPTZ final : public virtual ArPTZ
{
public:
  AREXPORT explicit ArRVisionPTZ(ArRobot *robot);
  //AREXPORT virtual ~ArRVisionPTZ();
  
  AREXPORT virtual bool init() override;
  AREXPORT virtual const char *getTypeName() override { return "rvision"; }
  /// Set serial port
  /// @since 2.7.6
  void setPort(const char *port)
  {
	  mySerialPort = port;
  }
protected:
  AREXPORT virtual bool pan_i(double degrees) override;
  AREXPORT virtual bool panRel_i(double degrees) override;
  AREXPORT virtual bool tilt_i(double degrees) override;
  AREXPORT virtual bool tiltRel_i(double degrees) override;
  AREXPORT virtual bool panTilt_i(double degreesPan, double degreesTilt) override;
  AREXPORT virtual bool panTiltRel_i(double degreesPan, double degreesTilt) override;
public:
  AREXPORT virtual bool canZoom() const override { return true; }
  AREXPORT virtual bool zoom(int zoomValue) override;
  AREXPORT virtual bool zoomRel(int zoomValue) override;
protected:
  AREXPORT virtual double getPan_i() const override { return myPan; }
  AREXPORT virtual double getTilt_i() const override { return myTilt; }
public:
  AREXPORT virtual int getZoom() const override { return myZoom; }
  //AREXPORT void getRealPanTilt();
  //AREXPORT void getRealZoomPos();
  /*
  AREXPORT virtual double getMaxPosPan() const override { return MAX_PAN; }
  AREXPORT virtual double getMaxNegPan() const override { return MIN_PAN; }
  AREXPORT virtual double getMaxPosTilt() const override { return MAX_TILT; }
  AREXPORT virtual double getMaxNegTilt() const override { return MIN_TILT; }
  AREXPORT virtual int getMaxZoom() const override { return MAX_ZOOM; }
  AREXPORT virtual int getMinZoom() const override { return MIN_ZOOM; }
  */

  AREXPORT virtual bool canGetRealPanTilt() const override { return false; }
  AREXPORT virtual bool canGetRealZoom() const override { return false; }
  AREXPORT virtual bool canGetFOV() override { return true; }
  /// Gets the field of view at maximum zoom
  AREXPORT virtual double getFOVAtMaxZoom()  override { return 4.4; }
  /// Gets the field of view at minimum zoom
  AREXPORT virtual double getFOVAtMinZoom() override { return 48.8; }

  //virtual ArBasePacket* readPacket();
  constexpr static double MAX_PAN = 180; ///< maximum degrees the unit can pan (clockwise from top)
  constexpr static double MIN_PAN = -180; ///< minimum degrees the unit can pan (counterclockwise from top)
  constexpr static double   MIN_TILT = -30; ///< minimum degrees the unit can tilt
  constexpr static double MAX_TILT = 60; ///< maximum degrees the unit can tilt
  constexpr static int MIN_ZOOM = 0; ///< minimum value for zoom
  constexpr static int MAX_ZOOM = 32767; ///< maximum value for zoom
  constexpr static double TILT_OFFSET_IN_DEGREES = 38; ///< offset value to convert internal camera coords to world
  constexpr static double PAN_OFFSET_IN_DEGREES = 190; ///< offset value to convert internal camera coords to world
  
  /// called automatically by Aria::init()
  ///@since 2.7.6
  ///@internal
#ifndef ARIA_WRAPPER
  static void registerPTZType();
#endif
protected:
  void initializePackets();
  double myPan;
  double myTilt;
  int myZoom;
  double myDegToTilt;
  double myDegToPan;
  double myPanOffsetInDegrees;
  double myTiltOffsetInDegrees;
  ArRVisionPacket myPacket;
  ArRVisionPacket myZoomPacket; 
  ArRVisionPacket myPanTiltPacket;
  ArRVisionPacket myInquiryPacket;
  const char *mySerialPort;

  ///@since 2.7.6
  static ArPTZ* create(size_t index, ArPTZParams params, ArArgumentParser *parser, ArRobot *robot);
  ///@since 2.7.6
  static ArPTZConnector::GlobalPTZCreateFunc ourCreateFunc;

};

#endif // ARRVISIONPTZ_H
