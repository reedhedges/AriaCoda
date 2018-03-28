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
#ifndef ARPTZ_H
#define ARPTZ_H

#include "ariaTypedefs.h"
#include "ArFunctor.h"
#include "ArCommands.h"
#include "ArPTZConnector.h"

class ArRobot;
class ArBasePacket;
class ArRobotPacket;
class ArDeviceConnection;


/// Base class which handles the PTZ cameras
/** 
    This class is mainly concerned with making all the cameras look
    the same for outgoing data, it is also set up to facilitate the
    acquisition of incoming data but that is described in the
    following paragraphs.  There are two ways this can be used.  The
    first is the simplest and default behavior and should be used by
    those whose cameras are attached to their robot's microcontroller,
    a ArRobot pointer is passed in to the contructor, this is where
    the commands will be sent to the robot via the robot's connection
    which will then send it along over the second serial port.  The
    second way is to pass an ArDeviceConnection to
    setDeviceConnection, if this is done commands will be sent along
    the given serial port, this should ONLY be done if the camera is
    attached straight to a serial port on the computer this program is
    running on.

    The next two paragraphs describe how to get data back from the
    cameras, but this base class is set up so that by default it won't
    try to get data back and assumes you're not trying to do that.  If
    you are trying to get data back the important functions are
    packetHandler, robotPacketHandler and readPacket and you should
    read the docs on those.

    If the camera is attached to the robot (and you are thus using the
    first method described in the first paragraph) then the only way
    to get data back is to send an ArCommands::GETAUX, then set up a
    robotPacketHandler for the AUX id and have it call the
    packetHandler you set up in in the class.

    If the camera is attached to the serial port on the computer (and
    thus the second method described in the first paragraph was used)
    then its more complicated... the default way is to just pass in an
    ArDeviceConnection to setDeviceConnection and implement the
    readPacket method (which MUST not block), and every time through
    the robot loop readPacket (with the sensorInterpHandler) will be
    called and any packets will be given to the packetHandler (which
    you need to implement in your class) to be processed.  The other
    way to do this method is to pass both an ArDefaultConnection and
    false to setDeviceConnection, this means the camera will not be
    read at all by default, and you're on your own for reading the
    data in (ie like your own thread).

  @ingroup OptionalClasses
   @ingroup DeviceClasses

  @todo add functions (optional to implement): power on/off, isReady/isInitialized, slew
**/
class ArPTZ
{
public:
  AREXPORT ArPTZ(ArRobot *robot);
  /// Destructor
  AREXPORT virtual ~ArPTZ();

  /// Initializes the camera
  AREXPORT virtual bool init(void) = 0;

  /// Return name of this PTZ type 
  AREXPORT virtual const char *getTypeName() = 0;

  /// Resets the camera
  /**
     This function will reset the camera to 0 0 pan tilt, and 0 zoom,
     on some cameras that can get out of sync it may need to do more,
     such as call init on it again.
   **/
  AREXPORT virtual void reset(void) 
    { panTilt(0, 0); if (canZoom()) zoom(getMinZoom()); }

  /// Pans to the given degrees. 0 is straight ahead, - is to the left, + to the right
  virtual bool pan(double degrees) 
  {
    if(myInverted) {
      return pan_i(-degrees);
    } else {
      return pan_i(degrees); 
    }
  }
  /// Pans relative to current position by given degrees
  virtual bool panRel(double degrees) { if(myInverted) return panRel_i(-degrees); else return panRel_i(degrees); }

  /// Tilts to the given degrees. 0 is middle, - is downward, + is upwards.
  virtual bool tilt(double degrees) { if(myInverted) return tilt_i(-degrees); else return tilt_i(degrees); }
  /// Tilts relative to the current position by given degrees
  virtual bool tiltRel(double degrees)  { if(myInverted)  return tiltRel_i(-degrees); else return tiltRel_i(degrees); }

  /// Pans and tilts to the given degrees
  virtual bool panTilt(double degreesPan, double degreesTilt) { if(myInverted) return panTilt_i(-degreesPan, -degreesTilt); else return panTilt_i(degreesPan,  degreesTilt); }
  /// Pans and tilts relatives to the current position by the given degrees
  virtual bool panTiltRel(double degreesPan, double degreesTilt) { if(myInverted) return panTiltRel_i(-degreesPan, -degreesTilt); else return panTiltRel_i(degreesPan,  degreesTilt); }

  /// Returns true if camera can zoom and this class can control the zoom amount
  AREXPORT virtual bool canZoom(void) const = 0;

  /// Zooms to the given value
  AREXPORT virtual bool zoom(int zoomValue) { return false; }
  /// Zooms relative to the current value, by the given value
  AREXPORT virtual bool zoomRel(int zoomValue) { return false; }

  /** The angle the camera is panned to (or last commanded value sent, if unable to obtain real pan position)
      @sa canGetRealPanTilt()
  */
  virtual double getPan(void) const  { if(myInverted) return -getPan_i(); else return getPan_i(); }

  /** The angle the camera is tilted to (or last commanded value sent, if unable to obtain real pan position)
      @sa canGetRealPanTilt()
  */
  virtual double getTilt(void) const  { if(myInverted) return -getTilt_i(); else return getTilt_i(); }

  /** The amount the camera is zoomed to (or last commanded value sent, 
      if unable to obtain real pan position)
    @sa canZoom();
    @sa canGetZoom()
  */
  AREXPORT virtual int getZoom(void) const { return 0; }

  /// Whether getPan() hand getTilt() return the device's real position, or last commanded position.
  AREXPORT virtual bool canGetRealPanTilt(void) const { return false; }

  /// Whether getZoom() returns the device's real zoom amount, or last commanded zoom position.
  AREXPORT virtual bool canGetRealZoom(void) const { return false; }


  /// Gets the highest positive degree the camera can pan to (inverted if camera is inverted)
  virtual double getMaxPosPan(void) const { if (myInverted) return -myMaxPosPan; else return myMaxPosPan; }


  /// @copydoc getMaxPosPan()
  double getMaxPan() const { return getMaxPosPan(); }

  /// Gets the lowest negative degree the camera can pan to (inverted if camera is inverted)
  virtual double getMaxNegPan(void) const { if (myInverted) return -myMaxNegPan; else return myMaxNegPan; }

  /// @copydoc getMaxNegPan()
  double getMinPan() const { return getMaxNegPan(); }


  /// Gets the highest positive degree the camera can tilt to (inverted if camera is inverted)
  virtual double getMaxPosTilt(void) const { if (myInverted) return -myMaxPosTilt; else return myMaxPosTilt; }

  /// @copydoc getMaxPosTilt()
  double getMaxTilt() const { return getMaxPosTilt(); }

  /// Gets the lowest negative degree the camera can tilt to (inverted if camera is inverted)
  virtual double getMaxNegTilt(void) const { if (myInverted) return -myMaxNegTilt; else return myMaxNegTilt; }

  ///@copydoc getMaxNegTilt() 
  double getMinTilt() const { return getMaxNegTilt(); }

  /// Halt any pan/tilt movement, if device supports it
  virtual bool haltPanTilt() { return false; };
  
  /// Halt any zoom movement, if device supports that
  virtual bool haltZoom() { return false; }

  /// Can pan and tilt speed (slew rates) be set to move device?
  virtual bool canPanTiltSlew() { return false; }
  /// @copydoc canPanTiltSlew()
  bool canSetSpeed() { return canPanTiltSlew(); }
  
  /// Set pan slew rate (speed) (degrees/sec) if device supports it (see canPanTiltSlew())
  virtual bool panSlew(double s) { return false; }
  /// @copydoc panSlew()
  bool setPanSpeed(double s) { return panSlew(s); }
  
  /// Set tilt slew rate (speed) (degrees/sec) if device supports it (see canPanTiltSlew())
  virtual bool tiltSlew(double s) { return false; }
  /// @copydoc tiltSlew()
  bool setTiltSpeed(double s) { return tiltSlew(s); }

  /// Maximum pan speed (slew rate) (degrees/sec) if device supports, or 0 if not.
  virtual double getMaxPanSpeed() { return 0.0; }

  /// Maximum tilt speed (slew rate) (degrees/sec) if device supports it, or 0 if not.
  virtual double getMaxTiltSpeed() { return 0.0; }

protected:
  /// Versions of the pan and tilt limit accessors where inversion is not applied, for use by subclasses to check when given pan/tilt commands.
  /// @todo limits checking should be done in ArPTZ pan(), tilt() and panTilt() public interface methods instead of in each implementation
  //@{
  virtual double getMaxPosPan_i(void) const { return myMaxPosPan; }
  double getMaxPan_i() const { return getMaxPosPan_i(); }
  virtual double getMaxPosTilt_i(void) const { return myMaxPosTilt; }
  double getMinPan_i() const { return getMaxNegPan_i(); }
  virtual double getMaxNegPan_i(void) const { return myMaxNegPan; }
  double getMaxTilt_i() const { return getMaxPosTilt_i(); }
  virtual double getMaxNegTilt_i(void) const { return myMaxNegTilt; }
  double getMinTilt_i() const { return getMaxNegTilt_i(); }
  //@}

public:

  /// Gets the maximum value for the zoom on this camera
  virtual int getMaxZoom(void) const { if (myInverted) return -myMaxZoom; else return myMaxZoom; }
  /// Gets the lowest value for the zoom on this camera
  virtual int getMinZoom(void) const { if (myInverted) return -myMinZoom; else return myMinZoom; }
  /// Whether we can get the FOV (field of view) or not
  virtual bool canGetFOV(void) { return false; }
  /// Gets the field of view at maximum zoom
  AREXPORT virtual double getFOVAtMaxZoom(void) { return 0; }
  /// Gets the field of view at minimum zoom
  AREXPORT virtual double getFOVAtMinZoom(void) { return 0; }

  /// Set gain on camera, range of 1-100.  Returns false if out of range
  /// or if you can't set the gain on the camera
  AREXPORT virtual bool setGain(double gain) const { return false; }
  /// Get the gain the camera is set to.  0 if not supported
  AREXPORT virtual double getGain(double gain) const { return 0; }
  /// If the driver can set gain on the camera, or not
  AREXPORT virtual bool canSetGain(void) const { return false; }

  /// Set focus on camera, range of 1-100.  Returns false if out of range
  /// or if you can't set the focus on the camera
  AREXPORT virtual bool setFocus(double focus) const { return false; }
  /// Get the focus the camera is set to.  0 if not supported
  AREXPORT virtual double getFocus(double focus) const { return 0; }
  /// If the driver can set the focus on the camera, or not
  AREXPORT virtual bool canSetFocus(void) const { return false; }

  /// Disable/enable autofocus mode if possible. Return false if can't change autofocus mode.
  AREXPORT virtual bool setAutoFocus(bool af = true) { return false; }

  /// Set whether the camera is inverted (upside down). If true, pan and tilt axes will be reversed.
  void setInverted(bool inv) { myInverted = inv; }

  /// Get whether the camera is inverted (upside down). If true, pan and tilt axes will be reversed.
  bool getInverted() { return myInverted; }

  /// Sets the device connection to be used by this PTZ camera, if set
  /// this camera will send commands via this connection, otherwise
  /// its via robot aux. serial port (see setAuxPortt())
  AREXPORT virtual bool setDeviceConnection(ArDeviceConnection *connection,
					    bool driveFromRobotLoop = true);
  /// Gets the device connection used by this PTZ camera
  AREXPORT virtual ArDeviceConnection *getDeviceConnection(void);
  /// Sets the aux port on the robot to be used to communicate with this device
  AREXPORT virtual bool setAuxPort(int auxPort);
  /// Gets the port the device is set to communicate on
  AREXPORT virtual int getAuxPort(void) { return myAuxPort; }
  /// Reads a packet from the device connection, MUST NOT BLOCK
  /** 
      This should read in a packet from the myConn connection and
      return a pointer to a packet if there was on to read in, or NULL
      if there wasn't one... this MUST not block if it is used with
      the default mode of being driven from the sensorInterpHandler,
      since that is on the robot loop.      
      @return packet read in, or NULL if there was no packet read
   **/
  AREXPORT virtual ArBasePacket *readPacket(void) { return NULL; }
  
  /// Sends a given packet to the camera (via robot or serial port, depending)
  AREXPORT virtual bool sendPacket(ArBasePacket *packet);
  /// Handles a packet that was read from the device
  /**
     This should work for the robot packet handler or for packets read
     in from readPacket (the joys of OO), but it can't deal with the
     need to check the id on robot packets, so you should check the id
     from robotPacketHandler and then call this one so that your stuff
     can be used by both robot and serial port connections.
     @param packet the packet to handle 
     @return true if this packet was handled (ie this knows what it
     is), false otherwise
  **/
  AREXPORT virtual bool packetHandler(ArBasePacket *packet) { return false; }

  /// Handles a packet that was read by the robot
  /**
     This handles packets read in from the robot, this function should
     just check the ID of the robot packet and then return what
     packetHandler thinks of the packet.
     @param packet the packet to handle
     @return true if the packet was handled (ie this konws what it is),
     false otherwise
  **/
  AREXPORT virtual bool robotPacketHandler(ArRobotPacket *packet);

  /// Internal, attached to robot, inits the camera when robot connects
  AREXPORT virtual void connectHandler(void);
  /// Internal, for attaching to the robots sensor interp to read serial port
  AREXPORT virtual void sensorInterpHandler(void);

  /// Return ArRobot object this PTZ is associated with. May be NULL
  ArRobot *getRobot() { return myRobot; }

  /// Set ArRobot object this PTZ is associated with. May be NULL
  void setRobot(ArRobot* r) { myRobot = r; }

protected:
  ArRobot *myRobot;
  ArDeviceConnection *myConn;
  ArFunctorC<ArPTZ> myConnectCB;
  ArFunctorC<ArPTZ> mySensorInterpCB;
  int myAuxPort;
  ArCommands::Commands myAuxTxCmd;
  ArCommands::Commands myAuxRxCmd;
  ArRetFunctor1C<bool, ArPTZ, ArRobotPacket *> myRobotPacketHandlerCB;
  bool myInverted;
  double myMaxPosPan;
  double myMaxNegPan;
  double myMaxPosTilt;
  double myMaxNegTilt;
  int myMaxZoom;
  int myMinZoom;

  /// Subclasses call this to set extents (limits) returned by getMaxPosPan(), getMaxNegPan(), getMaxPosTilt(), getMaxNegTilt(), getMaxZoom(), and getMinZoom().
  /// @since 2.7.6
  void setLimits(double maxPosPan, double maxNegPan,      double maxPosTilt, double maxNegTilt, int maxZoom = 0, int minZoom = 0)
  {
    myMaxPosPan = maxPosPan;
    myMaxNegPan = maxNegPan;
    myMaxPosTilt = maxPosTilt;
    myMaxNegTilt = maxNegTilt;
    myMaxZoom = maxZoom;
    myMinZoom = minZoom;
  }

  /// Internal implementations by subclasses. Inverted is not applied in these functions, it is done in the public interface above.
  /// Note, once execution enters one of these _i methods, then inversion has been
  /// applied and no call should be made to any pan/tilt or max/min limit accessor
  /// method that does not end in _i, or inversion will be applied again,
  /// reversing it.
  /// @since 2.7.6
  //@{
  AREXPORT virtual bool pan_i (double degrees) = 0;
  AREXPORT virtual bool panRel_i(double degrees) = 0;
  AREXPORT virtual bool tilt_i(double degrees) = 0;
  AREXPORT virtual bool tiltRel_i (double degrees) = 0; 
  AREXPORT virtual bool panTilt_i(double degreesPan, double degreesTilt) = 0;
  AREXPORT virtual bool panTiltRel_i(double degreesPan, double degreesTilt) = 0;
  AREXPORT virtual double getPan_i(void) const = 0;
  AREXPORT virtual double getTilt_i(void) const = 0;
  //@}

};

#endif // ARPTZ_H
