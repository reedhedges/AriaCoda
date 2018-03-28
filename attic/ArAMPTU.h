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
#ifndef ARAMPTU_H
#define ARAMPTU_H

#include "ariaTypedefs.h"
#include "ArBasePacket.h"
#include "ArPTZ.h"

/// A class with the commands for the AMPTU
class ArAMPTUCommands
{
public:
  enum {
    ABSTILT = 0x35, ///< Absolute tilt
    RELTILTU = 0x36, ///< Relative tilt, up
    RELTILTD = 0x37, ///< Relative tilt, down
    ABSPAN = 0x31, ///< Absolute pan
    RELPANCW = 0x32, ///< Relative pan, clockwise
    RELPANCCW = 0x33, ///< Relative pan, counter clockwise
    PANTILT = 0x28, ///< Pan and tilt absolute
    PANTILTUCW = 0x29, ///< Relative tilt up, pan clockwise
    PANTILTDCW = 0x2A, ///< Relative tilt down, pan clockwise
    PANTILTUCCW = 0x2B, ///< Relative tilt up, pan counter-clockwise
    PANTILTDCCW = 0x2C, ///< Relative tilt down, pan counter-clockwise
    ZOOM = 0x3F, ///< Zoom
    PAUSE = 0x39, ///< Pause the current movement
    CONT = 0x3A, ///< Continue paused movement
    PURGE = 0x3B, ///< Stops movement and purges commands
    STATUS = 0x3C, ///< Requests a status packet
    INIT = 0x3D, ///< Initializes the camera
    RESP = 0x3E, ///< Response
    PANSLEW = 0x34, ///< Sets the pan slew rate
    TILTSLEW = 0x38 ///< Sets the tilt slew rate
  };

};

/// A class for for making commands to send to the AMPTU
/** 
    There are only a few functioning ways to put things into this packet, you
    MUST use thse, if you use anything else your commands won't work.  You 
    must use byteToBuf and byte2ToBuf.  
    @see getUnitNumber
    @see setUnitNumber
*/
class ArAMPTUPacket: public ArBasePacket
{
public:
  /// Constructor
  AREXPORT ArAMPTUPacket(ArTypes::UByte2 bufferSize = 30);
  /// Destructor
  AREXPORT virtual ~ArAMPTUPacket();

  /// Gets the unit number this packet is for
  AREXPORT unsigned char getUnitNumber(void);
  /// Sets the unit number htis packet is for
  AREXPORT bool setUnitNumber(unsigned char unitNumber);
  
  AREXPORT virtual void byteToBuf(ArTypes::Byte val);
  AREXPORT virtual void byte2ToBuf(ArTypes::Byte2 val);
  
  AREXPORT virtual void finalizePacket(void);

protected:
  unsigned char myUnitNumber;
};

/// Driver for the AMPTU
/// @ingroup ObscureClasses
class ArAMPTU : public ArPTZ
{
public:
  /// Constructor
  AREXPORT ArAMPTU(ArRobot *robot, int unitNumber = 0);
  /// Destructor
  AREXPORT virtual ~ArAMPTU();
  AREXPORT virtual bool init(void);
  AREXPORT virtual const char *getTypeName() { return "amptu"; }
protected:
  AREXPORT virtual bool pan_i(double deg);
  AREXPORT virtual bool panRel_i(double deg);
  AREXPORT virtual bool tilt_i(double deg);
  AREXPORT virtual bool tiltRel_i(double deg);
  AREXPORT virtual bool panTilt_i(double panDeg, double tiltDeg);
  AREXPORT virtual bool panTiltRel_i(double panDeg, double tiltDeg);
public:
  /// Sets the rate that the camera pans at
  AREXPORT bool panSlew(double deg);
  /// Sets the rate the camera tilts at 
  AREXPORT bool tiltSlew(double deg);
  virtual bool canZoom(void) const { return false; }
  virtual bool canPanTiltSlew() { return true; }
  


  /// Stops current pan/tilt, can be resumed later
  AREXPORT bool pause(void);
  /// Resumes a previously paused pan/tilt
  AREXPORT bool resume(void);
  /// Stops motion and purges last command
  AREXPORT bool purge(void);
  /// Retrieves the camera status
  AREXPORT bool requestStatus(void);
  /// Gets the angle the camera is panned to
  AREXPORT virtual double getPan_i(void) const { return myPan; }
  /// Gets the angle the camera is tilted to
  AREXPORT virtual double getTilt_i(void) const { return myTilt; }
  enum {
    MIN_SLEW = 15,
    MAX_TILT_SLEW = 200,
    MAX_PAN_SLEW = 120
  };
protected:
  ArRobot *myRobot;
  ArAMPTUPacket myPacket;
  int myUnitNumber;
  double myPan;
  double myTilt;
  double myPanSlew;
  double myTiltSlew;
  /*double myCurPan;
  double myCurTilt;
  double myCurPanSlew;
  double myCurTiltSlew;*/
};

#endif // ARAMPTU_H

